from functools import reduce
import numpy as np
from LogSets.events.event_classes.ibase_event import IBaseEvent


class Overtaking(IBaseEvent):
    def __init__(self, extracted_data, components):
        super().__init__(extracted_data, components)
        self._init_supporting_variables()

    def calculate(self):
        self.host_overtaking_target(target_lifetime_more_than=50, target_lifetime_less_than=25, target_moving=True,
                                    target_long_vel_more_than=10)
        self.target_overtaking_host(target_lifetime_more_than=50, target_lifetime_less_than=25)
        return self.components

    def _init_supporting_variables(self, target_point_angle_range=(-15, 15), target_long_dist_to_car=2,
                                   target_lat_dist_to_car=10):
        self.count_unique_obj_id = self.extracted_data.groupby(['unique_obj_id'])['unique_obj_id'].count()
        self.valid_pos = self.extracted_data[(abs(self.extracted_data['position_longitudinal'])<target_long_dist_to_car)
                     & (abs(self.extracted_data['position_lateral']) < target_lat_dist_to_car)]['unique_obj_id'].unique()
        self.first_index_position = self.extracted_data.groupby('unique_obj_id')['position_longitudinal'].nth(0)
        self.behind = self.extracted_data[self.extracted_data['position_longitudinal'] < 0]['unique_obj_id'].unique()
        self.ahead = self.extracted_data[self.extracted_data['position_longitudinal'] >= 0]['unique_obj_id'].unique()
        self.point_angle = self.extracted_data[(self.extracted_data['pointing_angle'] > target_point_angle_range[0]) &
                       (self.extracted_data['pointing_angle'] < target_point_angle_range[1])]['unique_obj_id'].unique()
        self._simple_overtaking()

    def _simple_overtaking(self):
        self.ids_intersection = reduce(np.intersect1d, (self.point_angle, self.valid_pos, self.ahead, self.behind))

    def host_overtaking_target(self, target_lifetime_more_than=None, target_lifetime_less_than=None,
                               target_moving=False, target_long_vel_more_than=None):
        host_overtaking_target_array = self.first_index_position[self.first_index_position > 0].index.to_numpy()
        ids_intersection = np.intersect1d(self.ids_intersection, host_overtaking_target_array)
        feature_name = 'num_host_overtaking'

        if target_moving:
            moving_target = self.extracted_data[self.extracted_data['f_moving'] == 1]['unique_obj_id'].unique()
            feature_name = feature_name + '_mov_target'
            ids_intersection = np.intersect1d(ids_intersection, moving_target)
        if target_long_vel_more_than is not None:
            target_long_vel_more = self.extracted_data[self.extracted_data['velocity_longitudinal'] * 3.6 >
                                                       target_long_vel_more_than]['unique_obj_id'].unique()
            ids_intersection = np.intersect1d(ids_intersection, target_long_vel_more)
        if target_lifetime_more_than is not None:
            object_lifetime_more = self.count_unique_obj_id[
                self.count_unique_obj_id > target_lifetime_more_than].index.to_numpy()
            _feature_name = feature_name + '_life_more' + str(target_lifetime_more_than)
            self.components[_feature_name] = np.intersect1d(ids_intersection, object_lifetime_more).shape[0]
        if target_lifetime_less_than is not None:
            object_lifetime_less = self.count_unique_obj_id[
                self.count_unique_obj_id < target_lifetime_less_than].index.to_numpy()
            _feature_name = feature_name + '_life_less' + str(target_lifetime_less_than)
            self.components[_feature_name] = np.intersect1d(ids_intersection, object_lifetime_less).shape[0]

    def target_overtaking_host(self, target_lifetime_more_than=None, target_lifetime_less_than=None):
        target_overtaking_host_array = self.first_index_position[self.first_index_position < 0].index.to_numpy()
        ids_intersection = np.intersect1d(self.ids_intersection, target_overtaking_host_array)
        feature_name = 'num_target_overtaking_host'

        if target_lifetime_more_than is not None:
            object_lifetime_more = self.count_unique_obj_id[
                self.count_unique_obj_id > target_lifetime_more_than].index.to_numpy()
            _feature_name = feature_name + '_life_more' + str(target_lifetime_more_than)
            self.components[_feature_name] = np.array(np.intersect1d(ids_intersection, object_lifetime_more).shape[0])
        if target_lifetime_less_than is not None:
            object_lifetime_less = self.count_unique_obj_id[
                self.count_unique_obj_id < target_lifetime_less_than].index.to_numpy()
            _feature_name = feature_name + '_life_less' + str(target_lifetime_less_than)
            self.components[_feature_name] = np.array(np.intersect1d(ids_intersection, object_lifetime_less).shape[0])
