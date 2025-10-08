import warnings

import numpy as np
import pandas as pd

from aspe.extractors.F360.DataSets.F360BmwDetectionList import F360BmwDetectionList
from aspe.extractors.F360.DataSets.F360BmwRadarSensors import F360BmwRadarSensors
from aspe.extractors.Interfaces.Enums.Sensor import MountingLocation
from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.utilities.SupportingFunctions import recursive_dict_extraction


class F360Mdf4BmwSensorListBuilder(IBuilder):
    """
    Class for extracting someIP radar sensors list from parsed mf4 file.
    """
    def __init__(self, parsed_data: dict, f_extract_raw_signals: bool = True):
        super().__init__(parsed_data)
        self.data_set = F360BmwRadarSensors()
        self._f_extract_raw_signals = f_extract_raw_signals
        self._raw_signals = None
        
    def build(self):
        """
        Create and fill F360BmwRadarSensors with data.
        :return: filled F360BmwRadarSensors data set object
        """
        self._extract_per_sensor()
        self._set_coordinate_system()
        return self.data_set

    def _extract_per_sensor(self):
        detection_list = self._parsed_data['RecogSideRadarDetectionList']

        def first_value_from_dict(d: dict):
            return next(iter(d.values()))

        def gen():
            for sensor_id, (sensor_name, sensor_data) in enumerate(detection_list.items()):
                single_data = first_value_from_dict(sensor_data)

                mount_location = {
                    'Detection_List_Front_Left': MountingLocation.FRONT_LEFT,
                    'Detection_List_Front_Right': MountingLocation.FRONT_RIGHT,
                    'Detection_List_Rear_Left': MountingLocation.REAR_LEFT,
                    'Detection_List_Rear_Right': MountingLocation.REAR_RIGHT,
                }[sensor_name]
                origin = single_data['header']['origin']

                yield {
                    'sensor_id': sensor_id,
                    'mount_location': mount_location,
                    'position_x': origin['x']['value'],
                    'position_y': origin['y']['value'],
                    'position_z': origin['z']['value'],
                    'boresight_az_angle': origin['yaw']['value'],
                    'boresight_elev_angle': origin['pitch']['value'],
                }

        per_sensor = pd.DataFrame(gen())
        self.data_set.per_sensor = pd.concat([self.data_set.per_sensor, per_sensor], sort=False).reset_index(drop=True)

    def _set_coordinate_system(self):
        self.data_set.coordinate_system = 'ISO_VRACS'
