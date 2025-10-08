import numpy as np

from aspe.extractors.Interfaces import IDataSet
from aspe.utilities.MathFunctions import (
    calc_acceleration_in_position,
    calc_position_in_bounding_box,
    calc_velocity_in_position,
    normalize_angle_vector,
)


class RtCrossDataSetPostExtraction:
    def __init__(self):
        pass

    def extract(self, host: IDataSet, objects: IDataSet):
        self._host = host
        self._objects = objects

        self._host_ts_index = self._host.signals.set_index('utc_timestamp')
        self._host_suffix = '_host'

        self._extract_objects_bounding_box_orientation()
        self._extract_objects_center()
        self._extract_objects_velocity_otg()
        self._extract_objects_speed()
        self._extract_accelerations_otg()
        self._extract_relative_acceleration()

        # D-types cleaning
        self._host.signals = self._host.signals.convert_dtypes(convert_integer=False)
        self._objects.signals = self._objects.signals.convert_dtypes(convert_integer=False)

        return self._host, self._objects

    def _extract_objects_bounding_box_orientation(self):
        target_with_host_bbo = self._objects.signals.join(self._host.signals, rsuffix=self._host_suffix)

        target_bbo = target_with_host_bbo['wcs_bounding_box_orientation'] - \
                     target_with_host_bbo['wcs_bounding_box_orientation_host']

        target_bbo_normalized = normalize_angle_vector(target_bbo)

        self._objects.signals['bounding_box_orientation'] = target_bbo_normalized

    def _extract_objects_velocity_otg(self):
        target_with_host_vel_pos_yaw = \
            self._objects.signals.join(self._host.signals[['velocity_otg_y', 'velocity_otg_x', 'yaw_rate']],
                                       rsuffix=self._host_suffix)

        objects_rel_vel_y = target_with_host_vel_pos_yaw['velocity_rel_y']
        objects_rel_vel_x = target_with_host_vel_pos_yaw['velocity_rel_x']

        host_otg_vel_y = target_with_host_vel_pos_yaw['velocity_otg_y' + self._host_suffix]
        host_otg_vel_x = target_with_host_vel_pos_yaw['velocity_otg_x' + self._host_suffix]

        """
        It seems that hunter absolute velocity is reported in center of gravity - NOT in reference point (front bumper).
        Because of that velocity in reference point must be calculated. 
        """
        out_pos_long = np.full(self._objects.signals.shape[0], self._host.bounding_box_dimensions_x / 2)
        out_pos_lat = np.full(self._objects.signals.shape[0], 0)

        in_pos_lat = np.full(self._objects.signals.shape[0], 0)
        in_pos_long = np.full(self._objects.signals.shape[0], 0)

        host_yaw_rate = target_with_host_vel_pos_yaw['yaw_rate' + self._host_suffix]

        host_otg_vel_x, host_otg_vel_y = calc_velocity_in_position(in_pos_long, in_pos_lat,
                                                                   host_otg_vel_x, host_otg_vel_y, host_yaw_rate,
                                                                   out_pos_long, out_pos_lat)

        object_to_host_pos_y_diff = target_with_host_vel_pos_yaw['position_y']
        object_to_host_pos_x_diff = target_with_host_vel_pos_yaw['position_x']

        host_yaw_rate_influence_y = object_to_host_pos_x_diff * host_yaw_rate
        host_yaw_rate_influence_x = -object_to_host_pos_y_diff * host_yaw_rate

        objects_otg_vel_y = host_otg_vel_y + objects_rel_vel_y + host_yaw_rate_influence_y
        objects_otg_vel_x = host_otg_vel_x + objects_rel_vel_x + host_yaw_rate_influence_x

        self._objects.signals['velocity_otg_y'] = objects_otg_vel_y
        self._objects.signals['velocity_otg_x'] = objects_otg_vel_x

    def _extract_objects_center(self):
        """
        Calculate center position of bounding box.
        :return:
        """
        pos_x = self._objects.signals['position_x']
        pos_y = self._objects.signals['position_y']

        bbox_length = self._objects.signals['bounding_box_dimensions_x']
        bbox_width = self._objects.signals['bounding_box_dimensions_y']

        bounding_box_refpoint_long_offset_ratio = self._objects.signals['bounding_box_refpoint_long_offset_ratio']
        bounding_box_refpoint_lat_offset_ratio = self._objects.signals['bounding_box_refpoint_lat_offset_ratio']

        bounding_box_orientation = self._objects.signals['bounding_box_orientation']

        new_bounding_box_refpoint_long_offset_ratio = 0.5  # center is represented by point (0.5, 0.5)
        new_bounding_box_refpoint_lat_offset_ratio = 0.5

        # FIXME add exception handling in case of missing data (nan coming). Should be solved in CEA-241
        center_lon, center_lat = calc_position_in_bounding_box(pos_x, pos_y, bbox_length, bbox_width,
                                                               bounding_box_orientation,
                                                               np.array(bounding_box_refpoint_long_offset_ratio),
                                                               np.array(bounding_box_refpoint_lat_offset_ratio),
                                                               np.array(new_bounding_box_refpoint_long_offset_ratio),
                                                               np.array(new_bounding_box_refpoint_lat_offset_ratio))
        self._objects.signals['center_x'] = center_lon
        self._objects.signals['center_y'] = center_lat

    def _extract_objects_speed(self):
        self._objects.signals['speed'] = np.hypot(self._objects.signals['velocity_otg_y'],
                                                  self._objects.signals['velocity_otg_x'])

    def _extract_accelerations_otg(self):
        accel_tcs_x = self._objects.signals.loc[:, 'acceleration_tcs_x'].to_numpy()
        accel_tcs_y = self._objects.signals.loc[:, 'acceleration_tcs_y'].to_numpy()
        orientation = self._objects.signals.loc[:, 'bounding_box_orientation'].to_numpy()

        rot_matrix = np.array([
            [np.cos(orientation), -np.sin(orientation)],
            [np.sin(orientation), np.cos(orientation)]]).transpose((2, 0, 1))

        accel_matrix = np.array([accel_tcs_x, accel_tcs_y]).T.reshape((-1, 2, 1))
        accel_rotated = (rot_matrix @ accel_matrix).reshape((-1, 2))
        acc_x = accel_rotated[:, 0]
        acc_y = accel_rotated[:, 1]

        # calculated acceleration now needs to be transferred to reference point position - RtRange provides absolute
        # signals values in position of the box with antena
        yaw_rate = self._objects.signals.loc[:, 'yaw_rate'].to_numpy()
        yaw_acc = self._objects.signals.loc[:, 'yaw_acceleration'].to_numpy()

        pos_x = self._objects.signals.loc[:, 'position_x'].to_numpy()
        pos_y = self._objects.signals.loc[:, 'position_y'].to_numpy()
        bbox_length = self._objects.signals['bounding_box_dimensions_x']
        bbox_width = self._objects.signals['bounding_box_dimensions_y']

        old_rel_x, old_rel_y = (self._objects.signals['bounding_box_refpoint_long_offset_ratio'].to_numpy(),
                                self._objects.signals['bounding_box_refpoint_lat_offset_ratio'].to_numpy())
        new_rel_x, new_rel_y = 0.3, 0.5  # assume that RtRange box is placed on center, rear seat
        rt_pos_x, rt_pos_y = calc_position_in_bounding_box(pos_x, pos_y, bbox_length, bbox_width, orientation,
                                                           old_rel_x,
                                                           old_rel_y,
                                                           new_rel_x,
                                                           new_rel_y)

        acc_x_in_ref_point, acc_y_in_ref_point = calc_acceleration_in_position(rt_pos_x, rt_pos_y, acc_x, acc_y,
                                                                               yaw_rate, yaw_acc, pos_x, pos_y)

        self._objects.signals.loc[:, 'acceleration_otg_x'] = acc_x_in_ref_point.astype(float)
        self._objects.signals.loc[:, 'acceleration_otg_y'] = acc_y_in_ref_point.astype(float)

    def _extract_relative_acceleration(self):
        host_acc_x = self._host.signals.loc[:, 'acceleration_otg_x'].to_numpy()
        host_acc_y = self._host.signals.loc[:, 'acceleration_otg_y'].to_numpy()

        obj_acc_x = self._objects.signals.loc[:, 'acceleration_otg_x'].to_numpy()
        obj_acc_y = self._objects.signals.loc[:, 'acceleration_otg_y'].to_numpy()

        self._objects.signals.loc[:, 'acceleration_rel_x'] = obj_acc_x - host_acc_x
        self._objects.signals.loc[:, 'acceleration_rel_y'] = obj_acc_y - host_acc_y
