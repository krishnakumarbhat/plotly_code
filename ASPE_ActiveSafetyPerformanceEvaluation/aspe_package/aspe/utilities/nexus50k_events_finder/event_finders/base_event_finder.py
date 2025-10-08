from abc import abstractmethod
from datetime import datetime, timedelta
from pathlib import Path
from typing import Tuple

import numpy as np
import pandas as pd
from psac.interface import PSACInterface

from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluation import Nexus50kEvaluation, NexusEvent
from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluationConfig import Nexus50kEvaluationConfig
from aspe.extractors.API.nexus import extract_data_from_nexus_log
from aspe.extractors.Nexus import NexusExtractedData
from aspe.utilities.aspera_connect import AsperaConnect
from aspe.utilities.nexus50k_events_finder.utils import save_event_list_to_csv


class BaseNexusEventFinder:
    """
    Class responsible for finding events in given extracted data from 50k data stored on Nexus.
    """
    def __init__(self, config: Nexus50kEvaluationConfig, event_name: str):
        self.event_name = event_name
        self.config = config
        self.evaluation = Nexus50kEvaluation(config=config, pe_pipeline=None)
        self.pi = PSACInterface(url="https://nexus.aptiv.com/api", user=config.NEXUS_USER_NAME,
                                pass_word=config.NEXUS_PASSWORD)
        self.found_events = []

    @abstractmethod
    def find_event(self, extracted_data: NexusExtractedData) -> Tuple[float, float]:
        """
        Find event inside given extracted_data from Nexus. Logic should be specific for event type which needs to be
        found.
        @param extracted_data: data in ASPE format downloaded from Nexus. Should contains auto-gt data, host and radar
        detections.
        @return: nexus event info
        """
        pass

    def run(self):
        self.found_events = []
        for nexus_id, nexus_data in self.get_nexus_50k_logs_iterator():
            event_start_time, event_end_time = self.find_event(nexus_data)
            if np.isfinite(event_start_time) and np.isfinite(event_end_time):
                print('Event found')
                event = NexusEvent(self.event_name, nexus_id, datetime.fromtimestamp(event_start_time),
                                   datetime.fromtimestamp(event_end_time))
                self.found_events.append(event)
        return self.found_events

    def save_events_to_csv(self, csv_dir=None):
        if csv_dir is None:
            csv_dir = self.config.DATA_50K_CACHE_DIR
        csv_p = Path(csv_dir) / f'{self.event_name}_events.csv'
        self.update_events_with_aux_info()
        save_event_list_to_csv(self.found_events, csv_p)

    def get_nexus_50k_logs_iterator(self):
        nexus_to_aspera_table = self.evaluation.nexus_to_hpc_table
        f360_50k_logs = set(nexus_to_aspera_table.nexus_id.to_list())
        tag_50k_id = self.pi.getTagList(name='50k')[0].id
        logs_50k = self.pi.getLogList(
            query={"tags": tag_50k_id})
        logs_50k = [l for l in logs_50k if l.id in f360_50k_logs]

        logs_count = len(logs_50k)
        for log_idx, log in enumerate(logs_50k, start=1):
            log_start = datetime.fromisoformat(log.startTime.replace('Z', ''))
            log_end = datetime.fromisoformat(log.endTime.replace('Z', ''))
            log_len = (log_end - log_start).total_seconds()

            if log_len == 0.0:
                continue

            elif log_len < 60:
                print(f'Processing log {log.id} - {log_idx} / {logs_count}')
                try:
                    nexus_data = extract_data_from_nexus_log(log_id=log.id,
                                                             nexus_user=self.config.NEXUS_USER_NAME,
                                                             nexus_password=self.config.NEXUS_PASSWORD,
                                                             detections=True,
                                                             tracker_output=True,
                                                             cache_dir=self.config.NEXUS_CACHE_DIR)
                    yield log.id, nexus_data
                except Exception as e:
                    print('Error occurred, skipping log')
            else:
                dt = timedelta(seconds=60)
                start = log_start
                end = log_start + dt
                while end < log_end:
                    print(f'Processing log {log.id} - {log_idx} / {logs_count} - time range {end} / {log_end}')
                    try:
                        nexus_data = extract_data_from_nexus_log(log_id=log.id,
                                                                 nexus_user=self.config.NEXUS_USER_NAME,
                                                                 nexus_password=self.config.NEXUS_PASSWORD,
                                                                 cache_dir=self.config.NEXUS_CACHE_DIR,
                                                                 tracker_output=True,
                                                                 detections=True,
                                                                 min_time=start,
                                                                 max_time=end)
                        yield log.id, nexus_data
                    except Exception as e:
                        print('Error occurred, skipping log')
                    start = end
                    end = start + dt

    def update_events_with_aux_info(self):
        for ev in self.found_events:
            ev_files = self.evaluation.find_event_in_nexus_to_hpc_table(ev)
            if len(ev_files):
                ev.first_srr_file = Path(ev_files.hpcc_path.iloc[0]).stem
                ev.last_srr_file = Path(ev_files.hpcc_path.iloc[-1]).stem
                cached_nexus_file = self.evaluation.get_nexus_cached_data_path(ev, ev.tmin.timestamp(),
                                                                               ev.tmax.timestamp())
                if cached_nexus_file is not None:
                    ev.cached_nexus_data_file = Path(cached_nexus_file).stem
                else:
                    ev.cached_nexus_data_file = None
                    self.evaluation.get_nexus_cached_data_path(ev, ev.tmin.timestamp(), ev.tmax.timestamp())