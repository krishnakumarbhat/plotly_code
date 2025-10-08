from warnings import warn

from aspe.extractors.F360.DataSets.F360MetaData import F360MetaData
from aspe.extractors.F360.DataSets.F360Mf4BmwExtractedData import F360Mf4BmwExtractedData
from aspe.extractors.F360.MDF4_BMW_mid.Builders.F360Mf4BmwDetectionListBuilder import F360Mdf4BmwDetectionListBuilder
from aspe.extractors.F360.MDF4_BMW_mid.Builders.F360Mf4BmwDetectionListHeaderBuilder import (
    F360Mdf4BmwDetectionListHeaderBuilder,
)
from aspe.extractors.F360.MDF4_BMW_mid.Builders.F360Mf4BmwObjectListBuilder import F360Mdf4BmwObjectListBuilder
from aspe.extractors.F360.MDF4_BMW_mid.Builders.F360Mf4BmwObjectListHeaderBuilder import (
    F360Mdf4BmwObjectListHeaderBuilder,
)
from aspe.extractors.F360.MDF4_BMW_mid.Builders.F360Mf4BmwSensorListBuilder import F360Mdf4BmwSensorListBuilder
from aspe.extractors.F360.MDF4_BMW_mid.Builders.PTPDataBuilder import PTPDataBuilder
from aspe.extractors.Interfaces.IExtractor import IExtractor
from aspe.utilities.MathFunctions import pol2cart


class F360Mdf4BmwExtractor(IExtractor):
    """
    Extractor class for extracting BMW F360 tracker data from MDF4 files.
    """

    def __init__(self, *, rear_axle_to_front_bumper_distance=None, f_extract_raw_signals=True):
        super().__init__()
        self._f_extract_raw_signals = f_extract_raw_signals
        self._rear_axle_to_front_bumper_distance = rear_axle_to_front_bumper_distance

    def extract_data(self, parsed_data: dict):
        """
        Extract data from mf4 parser output and transform it into ASPE data structures.
        """
        extracted = F360Mf4BmwExtractedData()
        extracted.metadata = self._build_metadata(parsed_data['data'])
        extracted.objects = self._build_objects_list(parsed_data['data'])
        extracted.objects_header = self._build_object_list_header(parsed_data['data'])
        extracted.detections = self._build_detections(parsed_data['data'])
        extracted.detections_header = self._build_detection_list_header(parsed_data['data'])
        extracted.sensors = self._build_sensors(parsed_data['data'])
        extracted.host = self._build_host(parsed_data['data'])
        extracted.ptp_data = self._build_ptp_data(parsed_data['data'])
        self._cross_extraction(extracted)
        return extracted

    def _build_metadata(self, parsed_data: dict):
        metadata = F360MetaData()
        return metadata

    def _build_objects_list(self, parsed_data):
        object_list_builder = F360Mdf4BmwObjectListBuilder(parsed_data, self._f_extract_raw_signals)
        objects = object_list_builder.build()
        return objects

    def _build_object_list_header(self, parsed_data):
        object_list_header_builder = F360Mdf4BmwObjectListHeaderBuilder(parsed_data, self._f_extract_raw_signals)
        object_list_header = object_list_header_builder.build()
        return object_list_header

    def _build_detections(self, parsed_data):
        detections_builder = F360Mdf4BmwDetectionListBuilder(parsed_data, self._f_extract_raw_signals)
        detections = detections_builder.build()
        return detections

    def _build_detection_list_header(self, parsed_data):
        detection_list_header_builder = F360Mdf4BmwDetectionListHeaderBuilder(parsed_data, self._f_extract_raw_signals)
        detection_list_header = detection_list_header_builder.build()
        return detection_list_header

    def _build_sensors(self, parsed_data):
        sensors_builder = F360Mdf4BmwSensorListBuilder(parsed_data, self._f_extract_raw_signals)
        sensors = sensors_builder.build()
        return sensors

    def _build_ptp_data(self, parsed_data):
        if 'PTPMessages' in parsed_data:
            ptp_builder = PTPDataBuilder(parsed_data, self._f_extract_raw_signals)
            ptp_data = ptp_builder.build()
            return ptp_data

    def _cross_extraction(self, extracted):
        self._fill_objects_timestamp_and_scan_index(extracted)
        self._fill_detections_timestamp_and_scan_index(extracted)
        self._fill_detections_positions(extracted)
        self._calculate_system_latency(extracted)

    def _fill_objects_timestamp_and_scan_index(self, extracted):
        rows = extracted.objects.signals.loc[:, 'log_data_row'].to_numpy()
        extracted.objects.signals.loc[:, 'timestamp'] = \
            extracted.objects_header.signals.loc[rows, 'timestamp'].to_numpy()
        extracted.objects.signals.loc[:, 'scan_index'] = \
            extracted.objects_header.signals.loc[rows, 'scan_index'].to_numpy()
        extracted.objects.signals.loc[:, 'vigem_timestamp'] = \
            extracted.objects_header.signals.loc[rows, 'vigem_timestamp'].to_numpy()

    def _fill_detections_timestamp_and_scan_index(self, extracted):
        for sensor_id in extracted.sensors.per_sensor['sensor_id']:
            detections_sensor_id_mask = (extracted.detections.signals['sensor_id'] == sensor_id)
            detections_header_sensor_id_mask = (extracted.detections_header.signals['sensor_id'] == sensor_id)

            rows = extracted.detections.signals.loc[detections_sensor_id_mask, 'log_data_row'].to_numpy()
            detections_header = \
                extracted.detections_header.signals.loc[detections_header_sensor_id_mask, :].reset_index().loc[rows, :]

            for signal_name in 'timestamp', 'vigem_timestamp', 'scan_index':
                extracted.detections.signals.loc[detections_sensor_id_mask, signal_name] = \
                    detections_header[signal_name].to_numpy()
        for signal, dtype in {'timestamp': 'float', 'vigem_timestamp': 'float', 'scan_index': 'int'}.items():
            extracted.detections.signals[signal] = extracted.detections.signals[signal].astype(dtype)

    def _fill_detections_positions(self, extracted):
        detection_sensor_id = extracted.detections.signals['sensor_id']
        detection_sensor_df = extracted.sensors.per_sensor.loc[detection_sensor_id, :].reset_index(drop=True)

        sensor_azimuth = detection_sensor_df['boresight_az_angle']
        sensor_position_x = detection_sensor_df['position_x']
        sensor_position_y = detection_sensor_df['position_y']

        detection_range = extracted.detections.signals['range']
        detection_azimuth_scs = extracted.detections.signals['azimuth']
        detection_azimuth_vcs = detection_azimuth_scs + sensor_azimuth

        detection_x_scs_rot, detection_scs_rot = pol2cart(detection_range, detection_azimuth_vcs)

        detection_x_vcs = detection_x_scs_rot + sensor_position_x
        detection_y_vcs = detection_scs_rot + sensor_position_y

        extracted.detections.signals['position_x'] = detection_x_vcs
        extracted.detections.signals['position_y'] = detection_y_vcs

    @staticmethod
    def _get_extractor_name():
        return 'F360Mdf4Bwm'

    def _build_host(self, parsed_data):
        import numpy as np

        from aspe.extractors.Interfaces.IHost import IHost
        host = IHost()
        host.dist_of_rear_axle_to_front_bumper = self._rear_axle_to_front_bumper_distance
        host_parsed = parsed_data['RecogShortRangeRadarEgoVehicleSpeed']['EgoSpeed']
        speed = np.array([scan['longitudinal_speed_ego_vehicle']['value'] for scan in host_parsed.values()])
        vigem_timestamps = np.array(list(host_parsed.keys()))
        host.signals['speed'] = speed
        host.signals['vigem_timestamp'] = vigem_timestamps
        host.signals['unique_id'] = 0
        return host

    def _calculate_system_latency(self, extracted):
        if extracted.ptp_data is not None:
            vtgtd_median = extracted.ptp_data.vigem_to_global_time_diff_median
            vtgtd_spread_ms = extracted.ptp_data.vigem_to_global_time_diff_spread * 1e3
            if vtgtd_spread_ms > 0.2:
                warn(f"Spread of differences between vigem timestamp and system timestamp is equal to {vtgtd_spread_ms}"
                     f" [ms]. System latency evaluation can be not precise.")
            obj_list_header = extracted.objects_header.signals
            obj_list_header['publish_time_in_global_domain'] = obj_list_header['vigem_timestamp'] - vtgtd_median
            obj_list_header['system_latency'] = \
                obj_list_header['publish_time_in_global_domain'] - obj_list_header['timestamp']

            det_list_header = extracted.detections_header.signals
            det_list_header['publish_time_in_global_domain'] = det_list_header['vigem_timestamp'] - vtgtd_median
            det_list_header['system_latency'] = \
                det_list_header['publish_time_in_global_domain'] - det_list_header['timestamp']
