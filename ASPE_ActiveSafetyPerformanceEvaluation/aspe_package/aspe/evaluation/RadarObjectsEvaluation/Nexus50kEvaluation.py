import os
import re
import subprocess
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union

import numpy as np
import pandas as pd
from psac.interface import PSACInterface
from psac.items import Event
from tqdm import tqdm

from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PEObjectsEvaluationOutput import (
    PEObjectsEvaluationOutputMultiLog,
    PEObjectsEvaluationOutputSingleLog,
)
from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluationConfig import Nexus50kEvaluationConfig
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures import IntersectionOverUnionRatio
from aspe.evaluation.RadarObjectsEvaluation.PairsPreBuilders import PEPairedObjectsPreBuilderGating
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.extractors.API.mudp import extract_f360_from_mudp
from aspe.extractors.API.nexus import extract_data_from_nexus_log
from aspe.extractors.F360.DataSets.F360MudpExtractedData import F360MudpExtractedData
from aspe.extractors.Interfaces.Enums.Object import ObjectClass
from aspe.extractors.Interfaces.IObjects import IObjects
from aspe.extractors.Nexus.NexusExtractedData import NexusExtractedData
from aspe.providers.Preprocessing.ScanIndexSynch.ScanIndexInterpolationSynch import ScanIndexInterpolationSynch
from aspe.utilities.aspera_connect import AsperaConnect
from aspe.utilities.nexus50k_events_finder.pe_output_aggregator import PEOutputAggregator
from aspe.utilities.SupportingFunctions import calc_dets_azimuth_vcs, load_from_pkl, replace_file_extension, save_to_pkl


class NexusEvent:
    def __init__(self, name: str, nexus_log_id: str, tmin: datetime, tmax: datetime, nexus_event_id=None, index=None):
        self.name = name
        self.nexus_log_id = nexus_log_id
        self.tmin = tmin
        self.tmax = tmax
        self.nexus_event_id = nexus_event_id
        self.index = index

    def to_dict(self):
        return self.__dict__

    @staticmethod
    def create_from_nexus_event_record(nexus_event_record: Event, pi: Optional[PSACInterface] = None, index=None):
        from aspe.utilities.nexus50k_events_finder.utils import get_psac_interface

        if pi is None:
            psac_int = get_psac_interface()
        else:
            psac_int = pi
        event_name = psac_int.getEventNameList(event_name_id=nexus_event_record.name)[0]
        return NexusEvent(name=event_name.name,
                          nexus_log_id=nexus_event_record.log,
                          tmin=datetime.fromisoformat(nexus_event_record.startTimestamp.replace('Z', '')),
                          tmax=datetime.fromisoformat(nexus_event_record.endTimestamp.replace('Z', '')),
                          nexus_event_id=nexus_event_record.id,
                          index=index)


class Nexus50kEvaluation:
    def __init__(self, config: Nexus50kEvaluationConfig, pe_pipeline: PEPipeline, auto_gt_time_shift=0.200,
                 gmt_time_diff=7200):
        """
        Class responsible for evaluation F360 tracker objects with lidar based auto-ground truth cuboids.
        Tracker objects are taken from resimed .mudp files and ground truth is downloaded from Nexus database.
        :param config: configuration of evaluation environment
        :param pe_pipeline: ASPE pipeline object
        :param auto_gt_time_shift: time which is added to ground-truth to synchronize both data sets
        :param gmt_time_diff: amount of time difference between F360 and ground truth which is caused by time-zones
        difference - Nexus timestamps are given in UTC (Greenwich time) and F360 data timestamps are in local time.
        """
        self.config = config
        self.pi = PSACInterface(url="https://nexus.aptiv.com/api", user=config.NEXUS_USER_NAME,
                                pass_word=config.NEXUS_PASSWORD)
        self.interpolation = ScanIndexInterpolationSynch(ts_master_name='timestamp', ts_slave_name='timestamp')
        self.pe_pipeline = pe_pipeline
        self.aspera_connect = AsperaConnect(netid=config.ASPERA_USER_NAME, password=config.ASPERA_PASSWORD)
        self.nexus_to_hpc_table = self.load_nexus_to_srr_debug_map_table()
        self.gmt_time_diff = gmt_time_diff  # 2 hours in seconds for time zones correction
        self.auto_gt_time_shift = auto_gt_time_shift

    def evaluate_f360_resim_with_nexus_events_set(self,
                                                  events_source: Union[NexusEvent, List[NexusEvent], str, List[str]],
                                                  resim_exe_path: str = None,
                                                  resim_suffix: str = None,
                                                  resim_mydir: str = None,
                                                  existing_resimulated_output_dir_name: str = None,
                                                  single_event_output_save_dir: Optional[str] = None,
                                                  aggregated_output_save_dir: Optional[str] = None):
        """
        Run evaluation with set of events.
        :param events_source: parameter defining group of events for evaluation. Can be passed in few forms:
                              1. NexusEvent - evaluate just single event
                              2. list of NexusEvent - evaluate in loop every event in loop
                              3. string - name of event group stored in Nexus (as EventName type) - evaluate all event
                                 from this group
                              4. string - path to .csv file which stores events definitions (output of events finder)
                              5. list of strings - each element contains Nexus id of event which needs to be evaluated
        :param resim_exe_path: path to F360 core resim .exe file whih will be used for resimulation of events logs
        :param resim_suffix: 'osuffix' resim parameter
        :param existing_resimulated_output_dir_name: name of directory with already resimulated files. Use this if logs
        which need to be evaluated are already resimed. If this argument is passed resim_exe_path, resim_suffix and
        resim_mydir are not used
        :param resim_mydir: 'mydir' resim parameter - defines directory where resimmed logs should be placed
        :param single_event_output_save_dir: directory where evaluation output for each event will be saved
        :param aggregated_output_save_dir: directory where aggregated results for all events should be saved
        :return: None
        """
        events_list = self.get_events_list_from_events_source(events_source)

        if len(events_list) > 0:
            aggregator = PEOutputAggregator()
            for event in tqdm(events_list):
                print('-----------------------------------------------------------------------')
                out = self.evaluate_f360_resim_with_nexus_event(event, resim_exe_path, resim_suffix, resim_mydir,
                                                                existing_resimulated_output_dir_name,
                                                                single_event_output_save_dir)
                if out is not None:
                    aggregator.add(out, event)

            if existing_resimulated_output_dir_name is not None:
                sw_signature = existing_resimulated_output_dir_name
            else:
                sw_signature = resim_suffix

            agg_out = aggregator.aggregate(sw_signature=sw_signature)
            self.save_aggregated_output(agg_out,
                                        aggregated_output_save_dir,
                                        sw_signature=sw_signature,
                                        events_source=events_source)
        else:
            print('List of events for evaluation is empty - verify events_source argument')

    def get_events_list_from_events_source(self, events_source: Union[NexusEvent, List[NexusEvent], str, List[str]]) \
            -> List[NexusEvent]:

        from aspe.utilities.nexus50k_events_finder.download_upload_events_nexus import download_events_from_nexus
        from aspe.utilities.nexus50k_events_finder.utils import get_events_list_from_csv

        events_list = []
        # events_source is single NexusEvent object
        if isinstance(events_source, NexusEvent):
            events_list.append(events_source)

        # events_source is list of NexusEvent objects
        elif isinstance(events_source, list) and isinstance(events_source[0], NexusEvent):
            events_list.extend(events_source)

        # events_source is list of nexus event hashes
        elif isinstance(events_source, list) and isinstance(events_source[0], str):
            event_records = [download_events_from_nexus(pi=self.pi, event_id=id) for id in events_source]
            flattened_records = [x for xs in event_records for x in xs]
            events_list.extend([NexusEvent.create_from_nexus_event_record(r, pi=self.pi, index=i)
                                for i, r in enumerate(flattened_records)])

        # events_souce is path to .csv file which is output of events finder
        elif Path(events_source).exists() and '.csv' in events_source:
            events_list.extend(get_events_list_from_csv(events_source, only_n_first_events=None))

        # events_source is event name list from nexus, like 'f360_close_trucks'
        elif isinstance(events_source, str):
            event_records = download_events_from_nexus(event_name_list=events_source, pi=self.pi)
            events_list.extend([NexusEvent.create_from_nexus_event_record(r, pi=self.pi, index=i)
                                for i, r in enumerate(event_records)])
        return events_list

    def evaluate_f360_resim_with_nexus_event(self,
                                             nexus_event: NexusEvent,
                                             resim_exe_path: str = None,
                                             resim_suffix: str = None,
                                             resim_mydir: str = None,
                                             resim_out_dir_name: str = None,
                                             output_save_dir: Optional[str] = None) \
            -> PEObjectsEvaluationOutputMultiLog:
        """
        Calculate KPIs for given event.
        1. Get ASPERA paths to SRR_DEBUG.mf4 files which are within given event range.
        2. Download logs
        3. Convert to .mudp files
        4. Resim logs with given resim .exe path
        5. Perform ASPE evaluation of ressimed logs.
        6. Return KPIs.
        All data is downloaded only in first run, later on class checks if given log was already downloaded and if so -
        it path is simply loaded from disk.
        :param nexus_event: nexus log id and time range of event which needs to be evaluated
        :param resim_exe_path: path to F360 core resim .exe file which will be used to resim event logs
        :param resim_suffix: resim suffix which will be added to resimmed files
        :param resim_mydir: resim mydir argument, if passed resim output files are saved in mydir directory
        :param output_save_dir: directory where evaluation output for evaluated event will be saved
        :param resim_out_dir_name: evaluate already ressimed files within given directory in srr_debug_cache directory;
        when passed resim_exe_path and resim_suffix are ignored
        :return: performance evaluation output
        """
        if resim_exe_path is None and resim_out_dir_name is None:
            raise AttributeError('one of arguments: resim_exe_path or resim_out_dir_name needs to be specified')

        nexus_to_aspera_event = self.find_event_in_nexus_to_hpc_table(nexus_event)
        mudp_paths = self.download_mf4_and_convert_to_mudp_event_logs(nexus_to_aspera_event)

        if len(mudp_paths) == 0:
            return None

        if resim_out_dir_name:
            resimed_mudp_paths = self.find_resimed_mudp_files(mudp_paths, resim_out_dir_name)
        else:
            resimed_mudp_paths, resim_out_dir_name = self.resim_mudp_files(resim_exe_path, mudp_paths,
                                                                           osuffix=resim_suffix, mydir=resim_mydir)

        pe_output_agg = PEObjectsEvaluationOutputMultiLog()
        print("Evaluation started:")
        for mudp_path in resimed_mudp_paths:
            try:
                pe_out = self.evaluate_f360_log(mudp_path, nexus_event)
                pe_output_agg.update(mudp_path, pe_out)
            except Exception as e:
                print('Error, skipping log')
                print(e)

        self.calculate_aggregated_results(pe_output_agg)
        pe_output_agg.print_kpis_and_log_features()
        if output_save_dir is not None:
            self.save_single_event_output(pe_output_agg, nexus_event, resim_out_dir_name, output_save_dir)
        return pe_output_agg

    def evaluate_f360_log(self, mudp_path: str, nexus_event: NexusEvent) -> PEObjectsEvaluationOutputSingleLog:
        """
        Evaluate single f360 log file using auto-gt data from specified event. It is assumed that given mudp file
        is within specified event time-range.
        :param mudp_path: path to .mudp file which will be evaluated
        :param nexus_event: Nexus event
        :return: evaluation output
        """
        extracted_f360 = extract_f360_from_mudp(mudp_path, internal_objects=True, objects=False, detections=True,
                                                save_to_file=True)
        self.transform_local_time_to_utc_by_timezone_diff(extracted_f360)
        self.switch_time_domain_to_utc(extracted_f360)
        self.trim_f360_log_to_event_time(extracted_f360, nexus_event.tmin, nexus_event.tmax)
        min_log_ts, max_log_ts = self.get_min_max_utc_timestamp_from_f360_extracted(extracted_f360)
        extracted_nexus = self.get_nexus_data(nexus_event, min_log_ts, max_log_ts)

        self.synchronize_data_sources(extracted_f360, extracted_nexus)
        self.transform_to_same_cs(extracted_f360, extracted_nexus)
        self.count_radar_detections_in_auto_gt_boxes(extracted_f360, extracted_nexus)
        self.merge_overlapping_auto_gt_trucks(extracted_nexus)

        sync_filepath = self.get_nexus_sync_save_path(nexus_event, min_log_ts, max_log_ts)
        save_to_pkl(extracted_nexus, sync_filepath)
        extracted_nexus.objects = extracted_nexus.auto_gt_lidar_cuboids  # hack for pe pipeline

        pe_out = self.pe_pipeline.evaluate(extracted_f360, extracted_nexus)
        return pe_out

    def find_event_in_nexus_to_hpc_table(self, event: NexusEvent) -> pd.DataFrame:
        """
        Find given event range within self.nexus_to_hpc_table which represents linkage between all 50k data on HPC and
        Nexus logs id.
        :param event: event which needs to be evaluated
        :return: table with SRR_DEBUG HPC paths to logs which are within specified event range
        """
        event_table = self.nexus_to_hpc_table.loc[self.nexus_to_hpc_table.nexus_id == event.nexus_log_id, :].dropna()
        event_table = event_table.sort_values(by='min_utc_timestamp')

        log_tmin = event_table.loc[:, 'min_utc_timestamp'] - self.gmt_time_diff
        log_tmax = event_table.loc[:, 'max_utc_timestamp'] - self.gmt_time_diff

        event_tmin, event_tmax = event.tmin.timestamp(), event.tmax.timestamp()
        overlaping_timeranges = ~((log_tmax < event_tmin) | (event_tmax < log_tmin))

        event_table = event_table.loc[overlaping_timeranges, :]
        return event_table

    def download_mf4_and_convert_to_mudp_event_logs(self, nexus_to_hpc_table_event: pd.DataFrame) -> List[str]:
        """
        Take .mf4 SRR_DEBUG files HPC paths, convert it to ASPERA paths and download it. Next convert it to .mudp files.
        If .mudp file already exists - skip download and conversion.
        :param nexus_to_hpc_table_event: table with linkage between HPC SRR_DEBUG.mf4 files and Nexus logs id which are
        within evaluated event range.
        :return: list of converted .mudp files paths
        """
        hpcc_paths = nexus_to_hpc_table_event.loc[:, 'hpcc_path']
        mudp_paths = []
        for hpcc_path in hpcc_paths:
            mudp_cached_path = self.get_mudp_cached_path(hpcc_path)
            if mudp_cached_path.exists():
                mudp_paths.append(str(mudp_cached_path))
            else:
                aspera_path = self.convert_hpcc_path_to_aspera_path(hpcc_path)
                downloaded_mf4_path = self.aspera_connect.download(aspera_path, self.config.SRR_DEBUG_CACHE_DIR)
                if downloaded_mf4_path is not None and Path(
                        downloaded_mf4_path).exists():  # additional check to verify if downloading was success
                    converted_mudp_path = self.convert_mf4_to_mudp(downloaded_mf4_path)
                    if Path(converted_mudp_path).exists():
                        mudp_paths.append(converted_mudp_path)
                        Path(downloaded_mf4_path).unlink()  # remove mf4 file
                    else:
                        # conversion failed - stop process
                        print(f'mudp file after conversion: {converted_mudp_path} not found. Skipping it.')
                        pass  # TODO handle that/ throw exc or something
                else:
                    # file was not downloaded - stop process
                    pass  # TODO handle that/ throw exc or something

        return mudp_paths

    def convert_mf4_to_mudp(self, mf4_file_path: str) -> str:
        """
        Use mdf2dvl_mudp converter to convert SRR_DEBUG files downloaded from ASPERA to .mudp files and return path to
        it. If file is already converted - return path.
        :param mf4_file_path: path to mf4 file
        :return: path to output converted mudp file
        """
        file_p = Path(mf4_file_path)
        mudp_path = None
        if file_p.exists():
            file_ext = Path(mf4_file_path).suffix
            if file_ext.lower() == '.mf4':
                mudp_path = mf4_file_path.replace(file_ext, '.mudp')
                if Path(mudp_path).exists():
                    print('MDF2MUDP conversion: File already converted')
                else:
                    command = f'{self.config.MDF2MUDP_CONV_PATH} {mf4_file_path}'
                    print('MDF2MUDP conversion: Conversion started')
                    subprocess.call(command, stdout=subprocess.DEVNULL)
                    print('MDF2MUDP conversion: Conversion finished!')
        return mudp_path

    def resim_mudp_files(self, resim_exe_path: str, mudp_paths: List[str], osuffix: Optional[str] = None,
                         mydir: Optional[str] = None) -> List[str]:  # todo this should be in utilities?
        """
        Resim given .mudp paths with resim_exe_path. If few file paths are given function creates temporary log list.txt
        file and run resim in continous mode.
        :param resim_exe_path: path to F360Core resim .exe path
        :param mudp_paths: paths to mudp files which are ressimed
        :param osuffix: resim osuffix argument
        :param mydir: resim mydir argument, if passed resim output files are saved in mydir directory
        :return: path to output ressimed files
        """
        resim_input_paths = sorted([replace_file_extension(mp, '.dvl') for mp in mudp_paths])
        temp_log_list_path = Path(self.config.DATA_50K_CACHE_DIR) / 'temp_resim_log_list.txt'

        if len(resim_input_paths) == 1:  # single file resim case
            info_msg = f'Resim: Started resim of single file:\n {resim_input_paths[0]}'
            command = f'{resim_exe_path} {resim_input_paths[0]} '

        elif len(resim_input_paths) > 1:  # continous resim case
            resim_paths_string = '\n'.join(resim_input_paths)
            info_msg = (f'RESIM: Started resimulation of {len(resim_input_paths)}'
                        f' files in continuous mode:\n{resim_paths_string}')
            with open(temp_log_list_path, 'w') as file:
                for pat in resim_input_paths:
                    file.write(f"{pat}\n")
            command = f'{resim_exe_path} {temp_log_list_path} -filelist '

        else:  # empty input list case
            return []

        if osuffix is not None and len(osuffix) > 0:
            command += f'-osuffix {osuffix} '

        if mydir is not None:
            command += f'-mydir {mydir} '

        command += '-stream BMW -f360trkopt -sync_input -init_from_log -endopt'
        print(info_msg)
        subprocess.call(command, stdout=subprocess.DEVNULL)
        print('RESIM: Finished resimulation')
        resimed_mudp_path = [self.get_latest_resimed_file(p, Path(mydir)).replace('.dvl', '.mudp') for p in
                             resim_input_paths]
        resim_parent_dir_name = Path(resimed_mudp_path[0]).parent.stem
        if temp_log_list_path.exists():
            temp_log_list_path.unlink()

        return resimed_mudp_path, resim_parent_dir_name

    def get_latest_resimed_file(self, log_path: str,
                                log_dir: Optional[str] = None) -> str:  # todo this should be in utilities?
        """
        Find latest resim output file for given original mudp file.
        :param log_path: path to resim input file
        :param log_dir: directory where ressimed file was saved
        :return: latest resim output file
        """
        log_p = Path(log_path)
        if log_dir is not None:
            log_name, log_ext = log_p.stem, log_p.suffix
        else:
            log_dir, log_name, log_ext = log_p.parent, log_p.stem, log_p.suffix
        resimed_paths = [str(p) for p in log_dir.rglob(f'{log_name}*{log_ext}')]
        if resimed_paths:
            modification_time = [os.path.getmtime(p) for p in resimed_paths]
            files_info = pd.DataFrame({'log_path': resimed_paths, 'modification_time': modification_time})
            files_info = files_info.sort_values(by='modification_time', ascending=False)
            latest_resimed_path = files_info.iloc[0, :].log_path
        else:
            # TODO handle that
            pass
        return latest_resimed_path

    def get_nexus_sync_save_path(self, nexus_event: NexusEvent, min_log_ts: float, max_log_ts: float) -> Path:
        """
        Create file path which will be used to save Nexus extracted data after synchronization.
        :param nexus_event: Nexus event
        :param min_log_ts: min utc timestamp of log for which nexus data was synced
        :param max_log_ts: max utc timestamp of log for which nexus data was synced
        :return:
        """
        tmin_string = datetime.fromtimestamp(min_log_ts).strftime("%Y%m%d%H%M%S%f")[:-6]
        tmax_string = datetime.fromtimestamp(max_log_ts).strftime("%Y%m%d%H%M%S%f")[:-6]
        sync_filename = \
            f'log_{nexus_event.nexus_log_id}_tmin_{tmin_string}_tmax_{tmax_string}_nexus_extracted_sync.pickle'
        return Path(self.config.NEXUS_CACHE_DIR) / sync_filename

    def load_nexus_to_srr_debug_map_table(self) -> pd.DataFrame:
        """
        Download table which links srr_debug mf4 files on hpcc to nexus log id. Table also contains information about
        each mf4 log file time range. If table is already downloaded - just load it from cache directory.
        :return: table with linkage between srr_debug mf4 files on hpcc to nexus log id
        """
        nexus_to_aspera_table_local_path = Path(self.config.DATA_50K_CACHE_DIR) / self.config.NEXUS_TO_ASPERA_TABLE_NAME
        if not nexus_to_aspera_table_local_path.exists():
            self.aspera_connect.download(self.config.NEXUS_TO_SRR_DEB_TABLE_ASPERA_DIR, self.config.DATA_50K_CACHE_DIR)
        nexus_to_aspera_table = pd.read_csv(nexus_to_aspera_table_local_path, index_col=0)
        return nexus_to_aspera_table

    def convert_hpcc_path_to_aspera_path(self, hpc_path: str):
        """
        Convert HPCC paths to ASPERA directory.
        :param hpc_path: HPCC path
        :return: ASPERA path
        """
        return hpc_path.replace(self.config.ASPERA_DIR_ON_HPC, '')

    def get_mudp_cached_path(self, hpcc_mf4_path: str) -> Path:
        """
        Get path to .mudp file which would exist after downloading and conversions.
        :param hpcc_mf4_path: path to srr debug mf4 file on hpcc
        :return:
        """
        mf4_filename = Path(hpcc_mf4_path).name
        mudp_filename = replace_file_extension(mf4_filename, '.mudp')
        mudp_path = Path(self.config.SRR_DEBUG_CACHE_DIR) / mudp_filename
        return mudp_path

    def get_min_max_utc_timestamp_from_f360_extracted(self, f360_extracted: F360MudpExtractedData) -> Tuple[
        float, float]:
        """
        Give min-max UTC timestamp range from f360 extracted data.
        :param f360_extracted: f360 extracted data
        :return:
        """
        trk_info_signals = f360_extracted.tracker_info.signals
        if 'utc_timestamp' in trk_info_signals:
            utc_timestamp = trk_info_signals.loc[:, 'utc_timestamp'].to_numpy()
            return utc_timestamp[0], utc_timestamp[-1]

    def transform_local_time_to_utc_by_timezone_diff(self, f360_extracted: F360MudpExtractedData):
        """
        Add self.gmt_time_diff to tracker info utc timestamp column within extracted data.
        :param f360_extracted: f360 extracted data
        :return:
        """
        f360_extracted.tracker_info.signals.utc_timestamp -= self.gmt_time_diff

    def trim_f360_log_to_event_time(self, f360_extracted: F360MudpExtractedData, tmin: datetime, tmax: datetime) \
            -> None:
        """
        Trim given extracted data (in place) to given time range.
        :param f360_extracted: f360 extracted data
        :param tmin: min time range
        :param tmax: max time range
        :return:
        """
        tracker_info = f360_extracted.tracker_info
        utc = tracker_info.signals.utc_timestamp
        valid_scan_indexes = tracker_info.signals.loc[(tmin.timestamp() < utc) & (utc < tmax.timestamp()), 'scan_index']
        for data_set in f360_extracted.get_data_sets_list():
            if 'scan_index' in data_set.signals:
                valid_si_ds = data_set.signals.scan_index.isin(valid_scan_indexes)
                data_set.signals = data_set.signals.loc[valid_si_ds, :]
                data_set.signals.reset_index(drop=True, inplace=True)

    def switch_time_domain_to_utc(self, extracted_f360: F360MudpExtractedData):
        """
        Switch all datasets within extracted_f360 to be in UTC domain, by using tracker_info.signals.utc_timestamp
        column.
        :param extracted_f360: f360 extracted data
        :return: None
        """
        # TODO 1. this functionality should be build within datasets
        # TODO 2, this is not very accurate - PTP (Precision Time Protocol) should be used to do it correctly
        tracker_info = extracted_f360.tracker_info
        utc = tracker_info.signals.utc_timestamp
        object_list_ts = tracker_info.signals.timestamp
        utc_to_sensor_domain_time_diff = np.nanmedian(utc - object_list_ts)
        for ds in extracted_f360.get_data_sets_list():
            if 'timestamp' in ds.signals:
                ds.signals.timestamp += utc_to_sensor_domain_time_diff

    def synchronize_data_sources(self, extracted_f360: F360MudpExtractedData, extracted_nexus: NexusExtractedData):
        """
        Interpolate extracted_nexus.auto_gt_lidar_cuboids samples to be in same timestamps as
        extracted_f360.internal_objects.
        :param extracted_f360: f360 extracted data
        :param extracted_nexus: Nexus extracted data
        :return: None
        """
        extracted_nexus.auto_gt_lidar_cuboids.signals['timestamp'] += self.auto_gt_time_shift
        self.interpolation.synch(extracted_f360, extracted_nexus, extracted_f360.internal_objects,
                                 extracted_nexus.auto_gt_lidar_cuboids)

    def transform_to_same_cs(self, extracted_f360: F360MudpExtractedData, extracted_nexus: NexusExtractedData):
        """
        Nexus data is represented in lidar position based CS. Switch it to front bumper representation (VCS).
        :param extracted_f360: f360 extracted data
        :param extracted_nexus: Nexus extracted data
        :return: None
        """
        rear_axle_to_front_bumper = extracted_f360.host.dist_of_rear_axle_to_front_bumper
        extracted_nexus.transfer_cs('VCS',
                                    rear_axle_to_front_bumper - 0.74)
        # todo hardcoded value - this should be done by using lidar data

    def calculate_aggregated_results(self, pe_out: PEObjectsEvaluationOutputMultiLog):
        """
        Calculate aggregated KPIs for multiple logs which were evaluated.
        :param pe_out: aggregated results from multiple logs evaluation
        :return: None
        """
        if len(pe_out.pe_results_obj_pairs.signals) > 0:
            pairs_kpis = self.pe_pipeline.kpi_manager_pairs_features.calculate_kpis(pe_out.pe_results_obj_pairs)
            objects_kpis = self.pe_pipeline.kpi_manager_bin_class.calculate_kpis(pe_out.pe_results_obj_ref,
                                                                                 pe_out.pe_results_obj_est)
            log_features = pe_out.logs_features_per_log.drop(columns='log_index').groupby(
                by='signature').sum().reset_index()

            pe_out.kpis_pairs_features_aggregated = pairs_kpis
            pe_out.kpis_binary_class_aggregated = objects_kpis
            pe_out.logs_features_aggregated = log_features
        else:
            print('There is no associated pairs in evaluated log set. Skipping results aggregation')

    def save_single_event_output(self, pe_output_agg: PEObjectsEvaluationOutputMultiLog, nexus_event: NexusEvent,
                                 resim_out_dir_name: str,
                                 output_save_dir: str):
        """
        Save ASPE results file
        :param pe_output_agg: output of performance evaluation
        :param nexus_event: evaluated event
        :param resim_out_dir_name: name of directory where output resimed logs were created
        :param output_save_dir: directory where evaluation output for single event should be saved
        :return: None
        """
        filename_parts = [f'{nexus_event.name}']

        if nexus_event.index is not None:
            filename_parts.append(f'{nexus_event.index}')

        if nexus_event.nexus_event_id is not None:
            filename_parts.append(f'{nexus_event.nexus_event_id}')

        filename_parts.append(resim_out_dir_name)

        pe_out_path = Path(output_save_dir) / f'{"_".join(filename_parts)}_pe_output.pickle'
        pe_output_agg.save_to_pickle(pe_out_path)

    def save_aggregated_output(self, agg_output: Dict[str, pd.DataFrame],
                               aggregated_output_save_path: Optional[str] = None,
                               sw_signature: Optional[str] = None,
                               events_source: Optional[str] = None):
        if aggregated_output_save_path:

            if not Path(aggregated_output_save_path).exists():
                Path(aggregated_output_save_path).mkdir(parents=True, exist_ok=True)

            # Save in excel format
            now_string = datetime.now().strftime('%Y%m%d_%H%M%S')  # constructs string in format: YYYYmmdd_HHMMSS
            # events_source is event name list from nexus, like 'f360_close_trucks'
            f_is_nexus_events_list = isinstance(events_source, str) and '.csv' not in events_source
            source_string = events_source if f_is_nexus_events_list else None
            filename_base = '_'.join(filter(None, ['kpi_results', source_string, sw_signature, now_string]))
            filepath_xlsx = os.path.join(aggregated_output_save_path, filename_base + '.xlsx')
            with pd.ExcelWriter(filepath_xlsx, engine='openpyxl', mode='w') as writer:
                for sheet_name, df in agg_output.items():
                    df.to_excel(excel_writer=writer, sheet_name=sheet_name, index=False)

            # Save as csv file
            for sheet_name, df in agg_output.items():
                filename_csv = os.path.join(aggregated_output_save_path, f'{filename_base}_{sheet_name}.csv')
                df.to_csv(filename_csv, sep=';')

    def count_radar_detections_in_auto_gt_boxes(self, extracted_f360: F360MudpExtractedData,
                                                extracted_nexus: NexusExtractedData,
                                                rrate_gate_threshold: float = 0.5,
                                                zone_gate_margin: float = 0.25):
        def translate_and_rotate_positions(pos_mat, translation_vec, rotation_angle):
            transformed = pos_mat - translation_vec  # translation
            rot_mat = np.array([
                [np.cos(rotation_angle), -np.sin(rotation_angle)],
                [np.sin(rotation_angle), np.cos(rotation_angle)],
            ])
            transformed = (rot_mat @ transformed.T).T  # rotation
            return transformed

        def points_inside_bbox_in_same_cs(pos_mat, bbox_len, bbox_width, zone_gate_margin):
            within_long = np.abs(pos_mat[:, 0]) < (bbox_len / 2 + zone_gate_margin)
            within_lat = np.abs(pos_mat[:, 1]) < (bbox_width / 2 + zone_gate_margin)
            return within_long & within_lat

        def rrate_error_below_threshold(bbox_vel_x, bbox_vel_y, dets_az_cos, dets_az_sin, dets_rrate_comp):
            predicted_rrate = bbox_vel_x * dets_az_cos + bbox_vel_y * dets_az_sin
            rrate_error = np.abs(dets_rrate_comp - predicted_rrate)
            return rrate_error < rrate_gate_threshold

        calc_dets_azimuth_vcs(extracted_f360)
        auto_gt_columns = ['scan_index', 'center_x', 'center_y', 'velocity_otg_x', 'velocity_otg_y',
                           'bounding_box_dimensions_x', 'bounding_box_dimensions_y', 'bounding_box_orientation']
        dets_columns = ['scan_index', 'position_x', 'position_y', 'azimuth_vcs', 'range_rate_comp']

        auto_gt = extracted_nexus.auto_gt_lidar_cuboids.signals.loc[:, auto_gt_columns]
        dets = extracted_f360.detections.signals.loc[:, dets_columns].set_index('scan_index', drop=True)

        for scan_idx, auto_gt_scan in auto_gt.groupby(by='scan_index'):

            dets_scan = dets.loc[scan_idx, :]
            dets_pos = dets_scan.loc[:, ['position_x', 'position_y']].to_numpy()
            dets_az_cos = np.cos(dets_scan.loc[:, 'azimuth_vcs'].to_numpy())
            dets_az_sin = np.sin(dets_scan.loc[:, 'azimuth_vcs'].to_numpy())
            dets_rrate_comp = dets_scan.loc[:, 'range_rate_comp'].to_numpy()

            for row_idx, bbox in auto_gt_scan.iterrows():
                dx_dy = bbox[['center_x', 'center_y']].to_numpy()
                dets_transformed = translate_and_rotate_positions(dets_pos, dx_dy, bbox['bounding_box_orientation'])

                inside_zone = points_inside_bbox_in_same_cs(dets_transformed, bbox['bounding_box_dimensions_x'],
                                                            bbox['bounding_box_dimensions_y'], zone_gate_margin)

                inside_rrate_gate = rrate_error_below_threshold(bbox['velocity_otg_x'], bbox['velocity_otg_y'],
                                                                dets_az_cos, dets_az_sin, dets_rrate_comp)

                inside_dets_sum = np.sum(inside_zone & inside_rrate_gate)

                extracted_nexus.auto_gt_lidar_cuboids.signals.loc[row_idx, 'n_dets'] = inside_dets_sum

    def get_nexus_data(self, nexus_event: NexusEvent, min_log_ts: float, max_log_ts: float):
        cached_nexus_path = self.get_nexus_cached_data_path(nexus_event, min_log_ts, max_log_ts)
        if cached_nexus_path is not None:
            return load_from_pkl(cached_nexus_path)

        # if file is not found - just extract nexus log
        extracted_nexus = extract_data_from_nexus_log(log_id=nexus_event.nexus_log_id,
                                                      nexus_user=self.config.NEXUS_USER_NAME,
                                                      nexus_password=self.config.NEXUS_PASSWORD,
                                                      min_time=datetime.fromtimestamp(min_log_ts),
                                                      max_time=datetime.fromtimestamp(max_log_ts),
                                                      cache_dir=self.config.NEXUS_CACHE_DIR,
                                                      save_extracted_data_file=True,
                                                      host=True,
                                                      auto_gt=True,
                                                      detections=True,
                                                      sensors=True,
                                                      tracker_output=True)
        return extracted_nexus

    def get_nexus_cached_data_path(self, nexus_event: NexusEvent, min_log_ts: float, max_log_ts: float) -> str:
        cached_nexus_paths = [str(p) for p in
                              Path(self.config.NEXUS_CACHE_DIR).glob(f'log_{nexus_event.nexus_log_id}*')]
        if cached_nexus_paths:
            event_tmin = datetime.fromtimestamp(min_log_ts).strftime("%Y%m%d%H%M%S%f")[:-6]
            event_tmax = datetime.fromtimestamp(max_log_ts).strftime("%Y%m%d%H%M%S%f")[:-6]

            for cached_nexus_path in cached_nexus_paths:
                if 'tmin' in cached_nexus_path and 'tmax' in cached_nexus_path:
                    file_tmin = re.search('tmin_(.+?)_', cached_nexus_path).group(1)
                    file_tmax = re.search('tmax_(.+?)_', cached_nexus_path).group(1)

                    if file_tmin <= event_tmin and event_tmax < file_tmax:  # event is within file range
                        return cached_nexus_path
                else:
                    return cached_nexus_path

    def find_resimed_mudp_files(self, mudp_paths: List[str], resim_out_dir_name: str) -> List[str]:
        '''
        Find and return resim output files in MAIN_CACHE_DIR/srr_debug_cache/resim_out_dir_name which corresponds to
        resim input mudp_paths.
        @param mudp_paths: original (resim input) files
        @param resim_out_dir_name: resim output directory name which will be searched
        @return: list of paths to ressimed mudp files
        '''
        resim_out_dir_paths = list(Path(self.config.SRR_DEBUG_CACHE_DIR).glob(f'*{resim_out_dir_name}'))
        if len(resim_out_dir_paths) == 0:
            raise AttributeError(f'Cannot find {resim_out_dir_name} directory. Correct resim_out_dir_name ')
        resim_out_dir_paths.sort(key=lambda win_path: win_path.stat().st_mtime,
                                 reverse=True)  # sort by modification time
        resim_out_dir_path = resim_out_dir_paths[0]

        resimed_paths = []
        for mudp_path in mudp_paths:
            found_files = list(resim_out_dir_path.glob(f'{Path(mudp_path).stem}*.mudp'))
            if len(found_files) > 0:
                resimed_paths.append(str(found_files[0]))
        return resimed_paths

    def merge_overlapping_auto_gt_trucks(self, extracted_nexus):
        '''
        Common problem with auto-gt output is that trucks are oversegmentated - single real object is tracked as 2/3
        auto-gt cuboids. Handle that here with merging these objects.
        @param extracted_nexus:
        @return:
        '''
        pair_builder = PEPairedObjectsPreBuilderGating(distance_threshold=20, velocity_difference_threshold=3.0)

        auto_gt = extracted_nexus.auto_gt_lidar_cuboids.signals
        trucks = IObjects
        trucks.signals = auto_gt.loc[auto_gt.object_class == ObjectClass.TRUCK, :]

        paired_objects, pairs_signals_left, pairs_signals_right = pair_builder.build(trucks, trucks)

        valid_idxs = paired_objects.signals.loc[:, 'gating_distance'] > 0.0
        paired_objects.signals = paired_objects.signals.loc[valid_idxs, :]  # remove pairs to itself
        pairs_signals_left = pairs_signals_left.loc[valid_idxs, :]
        pairs_signals_right = pairs_signals_right.loc[valid_idxs, :]

        non_repetative_indexes = []
        for scan_index, scan_group in paired_objects.signals.groupby(by='scan_index'):
            scan_set = set()
            for index, pair in scan_group.iterrows():
                indexes = tuple(sorted((pair['index_ref'], pair['index_est'])))
                if not scan_set.__contains__(indexes):
                    scan_set.add(indexes)
                    non_repetative_indexes.append(index)

        paired_objects.signals = paired_objects.signals.loc[non_repetative_indexes, :].reset_index(drop=True)
        pairs_signals_left = pairs_signals_left.loc[non_repetative_indexes, :].reset_index(drop=True)
        pairs_signals_right = pairs_signals_right.loc[non_repetative_indexes, :].reset_index(drop=True)

        iou_df = IntersectionOverUnionRatio().calculate(pairs_signals_left, pairs_signals_right)
        overlapping_trucks = paired_objects.signals.loc[iou_df.intersection_over_union > 0.0, :]

        trucks_clusters = []
        for scan_index, scan_group in overlapping_trucks.groupby(by='scan_index'):
            groups = []
            for index, row in scan_group.iterrows():
                indexes_set = set(row[['index_ref', 'index_est']])
                new_trucks_clusters = True

                for group in groups:
                    if not set(indexes_set).isdisjoint(group):
                        group.update(indexes_set)
                        new_trucks_clusters = False
                        break

                if new_trucks_clusters:
                    groups.append(indexes_set)

            trucks_clusters.extend(groups)

        for trucks_cluster_indexes in trucks_clusters:
            trucks_cluster_indexes_list = list(trucks_cluster_indexes)

            trucks_cluster_df = auto_gt.loc[trucks_cluster_indexes_list, :]
            dx, dy = trucks_cluster_df.position_x.mean(), trucks_cluster_df.position_y.mean()
            rot_angle = np.unwrap(-trucks_cluster_df.bounding_box_orientation).mean()

            trucks_cluster_df.position_x -= dx
            trucks_cluster_df.position_y -= dy

            temp_pos_x = trucks_cluster_df.position_x * np.cos(rot_angle) - trucks_cluster_df.position_y * np.sin(
                rot_angle)
            temp_pos_y = trucks_cluster_df.position_x * np.sin(rot_angle) + trucks_cluster_df.position_y * np.cos(
                rot_angle)

            front = (temp_pos_x + trucks_cluster_df.bounding_box_dimensions_x / 2).max()
            rear = (temp_pos_x - trucks_cluster_df.bounding_box_dimensions_x / 2).min()
            left = (temp_pos_y - trucks_cluster_df.bounding_box_dimensions_y / 2).min()
            right = (temp_pos_y + trucks_cluster_df.bounding_box_dimensions_y / 2).max()

            new_center_x = (front + rear) / 2
            new_center_y = (left + right) / 2
            new_len = (front - new_center_x) * 2
            new_wid = (right - new_center_y) * 2

            truck_index_to_replace = trucks_cluster_indexes_list[0]
            auto_gt.loc[truck_index_to_replace, 'position_x'] = new_center_x + dx
            auto_gt.loc[truck_index_to_replace, 'center_x'] = new_center_x + dx
            auto_gt.loc[truck_index_to_replace, 'position_y'] = new_center_y + dy
            auto_gt.loc[truck_index_to_replace, 'center_y'] = new_center_y + dy

            auto_gt.loc[truck_index_to_replace, 'bounding_box_orientation'] = -rot_angle
            auto_gt.loc[truck_index_to_replace, 'bounding_box_dimensions_x'] = new_len
            auto_gt.loc[truck_index_to_replace, 'bounding_box_dimensions_y'] = new_wid

            auto_gt.drop(index=trucks_cluster_indexes_list[1:], inplace=True)

        auto_gt.reset_index(drop=True, inplace=True)
