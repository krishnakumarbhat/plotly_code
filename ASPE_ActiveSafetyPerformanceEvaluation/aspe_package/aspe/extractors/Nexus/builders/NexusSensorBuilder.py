from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.Interfaces.Enums.Sensor import MountingLocation
from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.extractors.Interfaces.IRadarSensors import IRadarSensors
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals
from aspe.utilities.MathFunctions import euler_from_quaternion


class NexusSensorBuilder(IBuilder):
    signal_mapper = {
        # ASPE signature                         F360 signature
        GeneralSignals.unique_id:   'sensorID',
        GeneralSignals.sensor_id:   'sensorID',
        GeneralSignals.look_id:     'look_id',
        GeneralSignals.look_index:  'look_index',
    }
    sensor_id_from_mounting = {
        'FrontLeft':   0,
        'FrontRight':  1,
        'RearLeft':    2,
        'RearRight':   3,
        'CenterLeft':  4,
        'CenterRight': 5,
        'Rooftop':     6,

    }
    sensor_mounting = {
        'FrontLeft': MountingLocation.FRONT_LEFT,
        'FrontRight': MountingLocation.FRONT_RIGHT,
        'RearLeft': MountingLocation.REAR_LEFT,
        'RearRight': MountingLocation.REAR_RIGHT,
        'CenterLeft': MountingLocation.CENTER_LEFT,
        'CenterRight': MountingLocation.CENTER_RIGHT,
        'Rooftop': MountingLocation.ROOFTOP,
    }
    look_id_from_lookType = {
        'LRLL': 0,
        'LRML': 1,
        'MRLL': 2,
        'MRML': 3,
    }
    max_range = {
        'UNKNOWN': -1,
        'SRR2_RADAR': 90,
        'SRR4_RADAR': 90,
        'SRR4_MM_RADAR': 90,
        'srr5': 90,
        'ESR_RADAR': 220,
        'MRR1_RADAR': 220,
        'MRR2_RADAR': 220,
        'MRR3_RADAR': 220,
        'MRR360_RADAR': 180,
        'LIDAR': -1,
        'VISION': -1,
        'VEHICLE': -1,
        'DEFAULT': 90,
    }
    elevation = (-5, 5)
    min_max_elevation = {
        'UNKNOWN': None,
        'SRR2_RADAR': elevation,
        'SRR4_RADAR': elevation,
        'SRR4_MM_RADAR': elevation,
        'srr5': elevation,
        'ESR_RADAR': elevation,
        'MRR1_RADAR': elevation,
        'MRR2_RADAR': elevation,
        'MRR3_RADAR': elevation,
        'MRR360_RADAR': elevation,
        'LIDAR': None,
        'VISION': None,
        'VEHICLE': None,
        'DEFAULT': elevation,
    }

    def __init__(self, parsed_data, save_raw_signals=False, dist_of_rear_axle_to_front_bumper=3.7):
        super().__init__(parsed_data)
        self._lifespan_mask = None
        self._parsed_data_shape = None
        self._raw_signals = None
        self.data_set = IRadarSensors()
        self.save_raw_signals = save_raw_signals
        self.dist_of_rear_axle_to_front_bumper = dist_of_rear_axle_to_front_bumper

    def build(self):
        self._extract_properties()
        return self.data_set

    def _extract_properties(self):
        """
        Extraction of default properties from parsed_data by modifying parameter->properties with extracted data
        """
        self._extract_properties_per_sensor()
        self._extract_properties_per_look()

    def _extract_properties_per_sensor(self):
        """
        Extract properties per sensor, sensor type and mount location are being mapped into their enum class
        counterparts
        """
        sensors_info = self._parsed_data['radar_info']['sensorInfoLog']
        radars_props = self.data_set.per_sensor
        for sensor in sensors_info:
            sensor_pos = sensor.extrinsics['position']
            if self.dist_of_rear_axle_to_front_bumper is not None:
                sensor_pos['x'] = sensor_pos['x'] - self.dist_of_rear_axle_to_front_bumper
            sensor_rot = sensor.extrinsics['rotation']
            _, _, boresight_az_angle = euler_from_quaternion(sensor_rot['w'], sensor_rot['x'], sensor_rot['y'],
                                                             sensor_rot['z'])
            if sensor.type == 'lidar':
                boresight_az_angle -= np.pi / 2  # handling lidar CS where x axis goes to the left, and y goes back
            radars_prop_dict = {
                "sensor_id":            self.sensor_id_from_mounting[sensor.mountingPosition],
                "nexus_id":             sensor.id,
                "sensor_type":          sensor.type,
                "sensor_name":          sensor.name,
                "polarity":             -1 if sensor.orientation == 'Leftward' else 1,
                "mount_location":       self.sensor_mounting[sensor.mountingPosition],
                "boresight_az_angle":   boresight_az_angle,
                "boresight_elev_angle": np.nan,
                "position_x":           sensor_pos['x'],
                "position_y":           sensor_pos['y'],
                "position_z":           sensor_pos['z'],
            }
            radars_props = radars_props.append(radars_prop_dict, ignore_index=True)
        self.data_set.per_sensor = radars_props.sort_values(by=['sensor_id']).reset_index().drop(columns='index')

    def _extract_properties_per_look(self):
        """
        Extract properties per look, per sensor, such as:
        - hard coded for now: max range, min and max elevation
        - 3d signal FOV split into two signals min and max azimuth
        """
        sensors_info = self._parsed_data['radar_info']['sensorInfoLog']
        radars_props = self.data_set.per_look
        for sensor in sensors_info:
            sensor_id = self.sensor_id_from_mounting[sensor.mountingPosition]
            if sensor.type == 'radar':
                min_azimuth = sensor.minApertureAngle
                max_azimuth = sensor.maxApertureAngle

                #   Hard coded signals
                sensor_type = sensor.calibration['type']
                if sensor_type in self.max_range:
                    max_range = self.max_range[sensor_type]
                else:
                    max_range = self.max_range['DEFAULT']
                    warn(f'SensorBuilder: Max range for {sensor_type} not found. Using default value.')

                if sensor_type in self.min_max_elevation:
                    min_max_elevation = self.min_max_elevation[sensor_type]
                else:
                    min_max_elevation = self.min_max_elevation['DEFAULT']
                    warn(f'SensorBuilder: Min/max elevation for {sensor_type} not found. Using default value.')

                min_elevation = min_max_elevation[0] if min_max_elevation is not None else np.nan
                max_elevation = min_max_elevation[1] if min_max_elevation is not None else np.nan

                for look_type in sensor.lookTypes:
                    look_id = self.look_id_from_lookType[look_type]
                    range_rate_interval_width = sensor.lookTypes[look_type]['vua']
                    radars_prop_dict = {
                        'sensor_id':                    sensor_id,
                        'look_id':                      look_id,
                        'min_azimuth':                  min_azimuth,
                        'max_azimuth':                  max_azimuth,
                        'range_rate_interval_width':    range_rate_interval_width,
                        'max_range':                    max_range,
                        'min_elevation':                min_elevation,
                        'max_elevation':                max_elevation,
                    }
                    radars_props = radars_props.append(radars_prop_dict, ignore_index=True)
        self.data_set.per_look = \
            radars_props.sort_values(by=['sensor_id', 'look_id']).reset_index().drop(columns='index')
