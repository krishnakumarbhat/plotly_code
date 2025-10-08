import pandas as pd

from aspe.extractors.Interfaces.IExtractor import IExtractor
from aspe.extractors.Nexus.builders.NexusAutoGTCuboidsBuilder import NexusAutoGTCuboidsBuilder
from aspe.extractors.Nexus.builders.NexusDetectionListBuilder import NexusDetectionListBuilder
from aspe.extractors.Nexus.builders.NexusHostBuilder import NexusHostBuilder
from aspe.extractors.Nexus.builders.NexusObjectListBuilder import NexusObjectListBuilder
from aspe.extractors.Nexus.builders.NexusSensorBuilder import NexusSensorBuilder
from aspe.extractors.Nexus.NexusExtractedData import NexusExtractedData


class NexusExtractor(IExtractor):
    def __init__(self, *,
                 extract_host=True,
                 extract_radar_info=True,
                 extract_radar_detections=True,
                 extract_auto_gt_lidar_cuboids=True,
                 extract_bmw_tracker_objects=True,
                 save_raw_signals=True,
                 rear_axle_to_front_bumper_distance=None):
        super().__init__()
        self.host_should_be_extracted = extract_host
        self.radar_info_should_be_extracted = extract_radar_info
        self.radar_detections_should_be_extracted = extract_radar_detections
        self.auto_gt_lidar_cuboids_should_be_extracted = extract_auto_gt_lidar_cuboids
        self.bmw_tracker_objects_should_be_extracted = extract_bmw_tracker_objects
        self.save_raw_signals = save_raw_signals
        self.rear_axle_to_front_bumper_distance = rear_axle_to_front_bumper_distance

    def extract_data(self, parsed_data) -> NexusExtractedData:
        """
        Transform raw parsed data downloaded from Nexus to ASPE structure (pandas.DataFrame based).
        :return: NexusExtractedData object
        """
        extracted = NexusExtractedData()

        extracted.host = self.build_host(parsed_data)
        extracted.sensors = self.build_radar_sensors(parsed_data)
        if extracted.sensors is not None:
            extracted.detections = self.build_detections(parsed_data, extracted.sensors.per_sensor)
            extracted.auto_gt_lidar_cuboids = self.build_auto_gt_lidar_cuboids(parsed_data,
                                                                               extracted.sensors.per_sensor)
        extracted.objects = self.build_tracker_objects_list(parsed_data)

        self.fill_scan_indexes(extracted)
        self.extract_rel_velocity_for_auto_gt(extracted)
        return extracted

    def build_host(self, parsed_data):
        if 'host' in parsed_data:
            builder = NexusHostBuilder(parsed_data, self.save_raw_signals, self.rear_axle_to_front_bumper_distance)
            return builder.build()

    def build_radar_sensors(self, parsed_data):
        if self.radar_info_should_be_extracted and 'radar_info' in parsed_data:
            builder = NexusSensorBuilder(parsed_data, self.save_raw_signals,
                                         dist_of_rear_axle_to_front_bumper=self.rear_axle_to_front_bumper_distance)
            return builder.build()

    def build_detections(self, parsed_data, sensors_data):
        if self.radar_detections_should_be_extracted and 'radar_detections' in parsed_data:
            detections_builder = NexusDetectionListBuilder(parsed_data, self.save_raw_signals, sensors_data)
            return detections_builder.build()

    def build_auto_gt_lidar_cuboids(self, parsed_data, sensors_data):
        if self.auto_gt_lidar_cuboids_should_be_extracted and 'auto_gt_lidar_cuboids' in parsed_data:
            builder = NexusAutoGTCuboidsBuilder(parsed_data, self.save_raw_signals, sensors_data)
            return builder.build()

    def build_tracker_objects_list(self, parsed_data):
        if self.bmw_tracker_objects_should_be_extracted and 'bwm_tracker_objects' in parsed_data:
            objects_builder = NexusObjectListBuilder(parsed_data, self.save_raw_signals)
            return objects_builder.build()

    def fill_scan_indexes(self, extracted_data):
        if extracted_data.detections is not None and \
                extracted_data.objects is not None and \
                extracted_data.auto_gt_lidar_cuboids is not None and \
                extracted_data.host is not None:
            radar_time_scan_info = extracted_data.detections.signals[['scan_index', 'timestamp', 'sensor_id']]
            found_sensor = radar_time_scan_info.loc[0, 'sensor_id']
            radar_time_scan_info = radar_time_scan_info[radar_time_scan_info['sensor_id'] == found_sensor].drop(
                columns='sensor_id')

            objects = extracted_data.objects.signals
            cuboids = extracted_data.auto_gt_lidar_cuboids.signals
            host = extracted_data.host.signals

            merge_dir = 'nearest'
            objects = pd.merge_asof(objects.sort_values('timestamp'),
                                    radar_time_scan_info.sort_values('timestamp'),
                                    on='timestamp',
                                    direction=merge_dir,
                                    suffixes=['_2', '']).drop(columns='scan_index_2')
            cuboids = pd.merge_asof(cuboids.sort_values('timestamp'),
                                    radar_time_scan_info.sort_values('timestamp'),
                                    on='timestamp',
                                    direction=merge_dir,
                                    suffixes=['_2', '']).drop(columns='scan_index_2')
            host = pd.merge_asof(host.sort_values('timestamp'),
                                 radar_time_scan_info.sort_values('timestamp'),
                                 on='timestamp',
                                 direction=merge_dir,
                                 suffixes=['_2', '']).drop(columns='scan_index_2')

            host.drop_duplicates(subset='scan_index', keep='last', inplace=True)

            extracted_data.objects.signals = objects
            extracted_data.auto_gt_lidar_cuboids.signals = cuboids
            extracted_data.host.signals = host

    @staticmethod
    def _get_extractor_name():
        return "NexusExtractor"

    def extract_rel_velocity_for_auto_gt(self, extracted):
        auto_gt = extracted.auto_gt_lidar_cuboids.signals
        host = extracted.host.signals

        tmp = auto_gt.join(host.set_index('scan_index'), on='scan_index', rsuffix='_host')
        auto_gt.loc[:, 'velocity_rel_x'] = tmp.velocity_otg_x - tmp.raw_speed
        auto_gt.loc[:, 'velocity_rel_y'] = tmp.velocity_otg_y
