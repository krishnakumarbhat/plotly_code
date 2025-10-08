from typing import Tuple

import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluationConfig import Nexus50kEvaluationConfig
from aspe.extractors.Nexus.NexusExtractedData import NexusExtractedData
from aspe.utilities.nexus50k_events_finder.event_finders.base_event_finder import BaseNexusEventFinder
from aspe.utilities.SupportingFunctions import contiguous_regions


class StopAndGoEventFinder(BaseNexusEventFinder):
    def __init__(self, config: Nexus50kEvaluationConfig):
        super().__init__(config, 'stop_and_go')

    def find_event(self, extracted_data: NexusExtractedData) -> Tuple[float, float]:
        host = extracted_data.host.signals
        event_start_time, event_end_time = np.inf, -np.inf

        host_speed_below_stationary_thr = host.raw_speed.to_numpy() < 1.0
        if np.any(host_speed_below_stationary_thr) & np.any(~host_speed_below_stationary_thr):
            for start_idx, end_idx in contiguous_regions(host_speed_below_stationary_thr):
                start_time, end_time = host.timestamp.iloc[start_idx], host.timestamp.iloc[end_idx - 1]

                if (end_time - start_time) > 5.0:  # at least 5 seconds of standing
                    event_start_time = min(event_start_time, start_time - 5.0)
                    event_end_time = max(event_end_time, end_time + 5.0)

            log_min_time, log_max_time = host.timestamp.min(), host.timestamp.max()
            event_start_time = max(event_start_time, log_min_time)
            event_end_time = min(event_end_time, log_max_time)

        return event_start_time, event_end_time
