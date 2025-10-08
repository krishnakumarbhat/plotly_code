from datetime import datetime
from pathlib import Path
from typing import Optional

from psac.contrib.psacutils.psacutils.timestamps import datetimeToTimestampStr
from psac.interface import PSACInterface

from aspe.extractors.Nexus.NexusExtractedData import NexusExtractedData
from aspe.extractors.Nexus.NexusExtractor import NexusExtractor
from aspe.utilities.SupportingFunctions import load_from_pkl, save_to_pkl


class NexusDownloader:
    def __init__(self, nexus_user: str, nexus_password: str, pi: Optional[PSACInterface] = None,
                 cache_dir: str = None):
        if isinstance(pi, PSACInterface):
            self.pi = pi
        else:
            self.pi = PSACInterface(url="https://nexus.aptiv.com/api", user=nexus_user, pass_word=nexus_password)
        self.cache_dir = cache_dir
        self._timestamp_dict = {}

    def update_timestamp_dict(self, min_time: datetime = None, max_time: datetime = None):
        if min_time is not None:
            self._timestamp_dict["$gt"] = datetimeToTimestampStr(min_time)

        if max_time is not None:
            self._timestamp_dict["$lt"] = datetimeToTimestampStr(max_time)

    def download_log_data(self, log_id: str,
                          min_time: datetime = None,
                          max_time: datetime = None,
                          auto_gt: bool = True,
                          sensors: bool = True,
                          host: bool = True,
                          detections: bool = False,
                          tracker_output: bool = False, ):

        self.update_timestamp_dict(min_time=min_time, max_time=max_time)
        output = {}
        if auto_gt:
            output['auto_gt_lidar_cuboids'] = self._download_auto_gt_lidar_cuboids_samples(log_id)
        if host:
            output['host'] = self._download_host_samples(log_id)
        if detections:
            output['radar_detections'] = self._download_radar_detection_samples(log_id)
        if sensors:
            output['radar_info'] = self._download_radar_info(log_id)
        if tracker_output:
            output['bwm_tracker_objects'] = self._download_bmw_tracker_objects_samples(log_id)

        return output

    def _download_radar_detection_samples(self, log_id: str):
        radar_streams = self.pi.getStreamList(
            query={
                "log": log_id,
                "dtype": "radarDetectionList",
            },
        )

        radar_stream_samples = []
        for r_stream in radar_streams:
            current_radar_samples = self._get_stream_samples(stream_id=r_stream.id)
            radar_stream_samples.append(current_radar_samples)

        output = {
            'radar_detections_info': radar_streams,
            'radar_detections_samples': radar_stream_samples,
        }
        return output

    def _download_auto_gt_lidar_cuboids_samples(self, log_id: str):
        cuboid_streams = self.pi.getStreamList(query={"log": log_id, "dtype": "cuboidList"},
                                               options={"sort": "-createdAt"})
        for stream in cuboid_streams:
            algo_info = self.pi.getAlgorithmList(algorithm_id=stream.algorithm)[0]
            # algo infos should be sorted by creation time - first autolabeling is newest
            if algo_info.name == 'autolabeling':
                auto_gt_samples = self._get_stream_samples(stream_id=stream.id)
                return {'auto_gt_samples': auto_gt_samples, 'auto_gt_version': algo_info.version}

    def _download_host_samples(self, log_id: str):
        log_info = self.pi.getLogList(log_id=log_id)[0]
        host_properties = self.pi.getVehicleList(vehicle_id=log_info.vehicle)[0]
        host_stream = self.pi.getStreamList(query={"log": log_id, "dtype": "host"})
        host_stream_samples = self._get_stream_samples(stream_id=host_stream[-1].id)

        output = {
            'properties': host_properties,
            'signals': host_stream_samples,
        }
        return output

    def _download_bmw_tracker_objects_samples(self, log_id: str):
        algorithms = self.pi.getAlgorithmList(query={'name': 'BM SRR5 Tracker'})
        all_streams = self.pi.getStreamList(query={"log": log_id, "dtype": "cuboidList"},
                                            options={"sort": "-createdAt"})
        tracker_objects_info = [(stream) for stream in all_streams if algorithms[0].id == stream.algorithm]
        tracker_objects_samples = self._get_stream_samples(tracker_objects_info[0].id)
        tracker_objects_output = {
            'tracker_objects_info': tracker_objects_info,
            'tracker_objects_samples': tracker_objects_samples}
        return tracker_objects_output

    def _download_radar_info(self, log_id: str):
        log_info = self.pi.getLogList(log_id=log_id)[0]
        sensors_list = self.pi.getVehicleList(vehicle_id=log_info.vehicle)[0].sensors

        radar_info = []
        for sensor in sensors_list:
            sensor_data = self.pi.getSensorList(sensor_id=sensor['sensor'])
            if sensor_data[0].type in ['radar', 'lidar']:
                sensor_data[0].extrinsics = sensor['extrinsics']
                radar_info = radar_info + sensor_data
        output = {'sensorInfoLog': radar_info}
        return output

    def _get_stream_samples(self, stream_id: str):
        query = {"stream": stream_id}
        if self._timestamp_dict:
            query.update({"timestamp": self._timestamp_dict})
        return self.pi.getSampleList(query=query)


def extract_data_from_nexus_log(log_id: str,
                                nexus_user: str,
                                nexus_password: str,
                                pi: Optional[PSACInterface] = None,
                                cache_dir: str = None,
                                save_nexus_downloaded_streams_file: bool = True,
                                save_extracted_data_file: bool = True,
                                save_raw_signals: bool = True,
                                rear_axle_to_front_bumper_distance: float = None,
                                min_time: datetime = None,
                                max_time: datetime = None,
                                auto_gt: bool = True,
                                host: bool = True,
                                sensors: bool = True,
                                detections: bool = False,
                                tracker_output: bool = False,
                                force_extract: bool = False,
                                ) -> NexusExtractedData:
    filename_parts = [f'log_{log_id}']

    if min_time is not None:
        tmin_string = min_time.strftime("%Y%m%d%H%M%S%f")[:-6]
        filename_parts.append(f'tmin_{tmin_string}')

    if max_time is not None:
        tmax_string = max_time.strftime("%Y%m%d%H%M%S%f")[:-6]
        filename_parts.append(f'tmax_{tmax_string}')

    cache_filename = f'{"_".join(filename_parts)}_nexus_extracted.pickle'
    if cache_dir and not force_extract:
        save_file_path = Path(cache_dir) / cache_filename
        if save_file_path.exists():
            print(f"Already extracted log found in file {save_file_path}. Loading it...")
            extracted_data = load_from_pkl(save_file_path)
            return extracted_data

    nexus_downloader = NexusDownloader(nexus_user, nexus_password, pi, cache_dir)
    nexus_extractor = NexusExtractor(save_raw_signals=save_raw_signals,
                                     rear_axle_to_front_bumper_distance=rear_axle_to_front_bumper_distance,
                                     extract_auto_gt_lidar_cuboids=auto_gt,
                                     extract_host=host,
                                     extract_radar_detections=detections,
                                     extract_bmw_tracker_objects=tracker_output)

    nexus_streams = nexus_downloader.download_log_data(log_id, min_time, max_time,
                                                       host=host,
                                                       sensors=sensors,
                                                       auto_gt=auto_gt,
                                                       tracker_output=tracker_output,
                                                       detections=detections)
    extracted_data = nexus_extractor.extract_data(parsed_data=nexus_streams)
    if save_extracted_data_file and cache_dir:
        cache_dir_path = Path(cache_dir)
        cache_dir_path.mkdir(parents=True, exist_ok=True)
        save_file_path = cache_dir_path / cache_filename
        save_to_pkl(extracted_data, save_file_path)
        print(f"Saved extracted log to {save_file_path}")
    return extracted_data
