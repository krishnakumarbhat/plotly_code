import numpy as np
import pandas as pd
from typing import List
from LogSets.utils.enums import ObjectClass


class FeaturesCalculator:
    def __init__(self, extracted_data, components):
        """
        :param extracted_data: extracted object's signals
        :param components: data frame with already calculated components
        """
        self.df = extracted_data
        self.features = components
        self.available_features = dict()
        self._init_supporting_variables()
        self._init_available_features()

    def calculate_all_available_features(self) -> pd.DataFrame:
        for feature in self.available_features.values():
            try:
                feature()
            except:
                print(f'Cant calculate: {str(feature)}')
        return self.features

    def calculate_features(self, features: List[str]) -> pd.DataFrame:
        for feature in features:
            if feature in self.available_features.keys():
                try:
                    self.available_features[feature]()
                except:
                    print(f'Cant calculate: {str(feature)}')
            else:
                print(f'feature: "{feature}" is not a valid feature name. Available features are: '
                      f'{" , ".join(list(self.available_features.keys()))} ')
        return self.features

    def _init_available_features(self):
        #   If you create new feature it has to be added here to self.available_features dictionary
        self.available_features['mean_max_num_objs'] = self.mean_max_num_objs
        self.available_features['mean_max_num_moving_objs'] = self.mean_max_num_moving_objs
        self.available_features['mean_max_num_stat_objs'] = self.mean_max_num_stat_objs
        self.available_features['mean_max_num_stat_movable_objs'] = self.mean_max_num_stat_movable_objs
        self.available_features['num_unique_objs_divided_by_log_length'] = self.num_unique_objs_divided_by_log_length
        self.available_features['mean_unique_objs_lifetime'] = self.mean_unique_objs_lifetime
        self.available_features['num_unique_objs_lifetime_in_range'] = self.num_unique_objs_lifetime_in_range
        self.available_features['mean_max_objs_distance_in_range'] = self.mean_max_objs_distance_in_range
        self.available_features['mean_max_moving_objs_speed_in_range'] = self.mean_max_moving_objs_speed_in_range
        self.available_features['mean_max_stat_objs_speed_in_range'] = self.mean_max_stat_objs_speed_in_range
        self.available_features['mean_max_moving_objs_point_angle_in_range'] = self.mean_max_moving_objs_point_angle_in_range
        self.available_features['mean_max_movable_objs_conf_lvl_in_range'] = self.mean_max_movable_objs_conf_lvl_in_range
        self.available_features['mean_max_moving_objs_point_angle_spread'] = self.mean_max_moving_objs_point_angle_spread
        self.available_features['mean_max_movable_objs_speed_spread'] = self.mean_max_movable_objs_speed_spread
        self.available_features['mean_max_non_movable_objs_speed_spread'] = self.mean_max_non_movable_objs_speed_spread
        self.available_features['min_mean_max_obj_class'] = self.min_mean_max_obj_class

    def _init_supporting_variables(self):
        try:
            self.movable_series = self.df['f_movable']
        except:
            self.movable_series = self.df['f_moveable']

        self.log_len = self.df['scan_idx'].nunique()
        self.num_unique_objs = self.df['unique_obj_id'].nunique()
        try:
            self.df['distance'] = np.vectorize(lambda x, y: np.sqrt(x ** 2 + y ** 2))(self.df['position_lateral'],
                                                                                  self.df['position_longitudinal'])
        except:
            print('position_lateral or position_longitudinal are not in signals')

        try:
            self.df['speed_km/h'] = np.vectorize(lambda x: x * 3.6)(self.df['speed'])
        except:
            print('speed not in signals')

        try:
            self.moving_objs_df = self.df[self.df['f_moving'] == 1]
            self.stat_objs_df = self.df[self.df['f_moving'] == 0]
            self.moving_speed_more_25 = self.df[(self.df['speed_km/h'] > 25) & (self.df['f_moving'] == 1)]
        except:
            print('f_moving not in signals')

        try:
            self.movable_objs_df = self.df[self.movable_series == 1]
            self.non_movable_objs_df = self.df[self.movable_series == 0]
        except:
            print('f_movable not in signals')

        try:
            self.stat_movable_objs_df = self.df[(self.df['f_moving'] == 0) & (self.movable_series == 1)]
        except:
            print('f_movable or f_moving are not in signals')

    def mean_max_num_objs(self):
        #   Mean and max number of objects per scan index
        mean_, max_ = self._calc_mean_max(self.df, 'scan_idx')
        self.features['mean_num_objs'] = mean_
        self.features['max_num_objs'] = max_

    def mean_max_num_moving_objs(self):
        #   Mean and max number of moving objects per scan index
        mean_, max_ = self._calc_mean_max(self.moving_objs_df, 'scan_idx')
        self.features['mean_num_moving_objs'] = mean_
        self.features['max_num_moving_objs'] = max_

    def mean_max_num_stat_objs(self):
        #   Mean and max number of stationary objects per scan index
        mean_, max_ = self._calc_mean_max(self.stat_objs_df, 'scan_idx')
        self.features['mean_num_stat_objs'] = mean_
        self.features['max_num_stat_objs'] = max_

    def mean_max_num_stat_movable_objs(self):
        #   Mean and max number of stationary movable objects per scan index
        mean_, max_ = self._calc_mean_max(self.stat_movable_objs_df, 'scan_idx')
        self.features['mean_num_stat_movable_objs'] = mean_
        self.features['max_num_stat_movable_objs'] = max_

    def num_unique_objs_divided_by_log_length(self):
        #   Number of unique all/moving/stationary objects normalised by log length
        self.features['num_unique_objs/log_length'] = self.num_unique_objs / self.log_len
        self.features['num_unique_moving_objs/log_length'] = self.moving_objs_df['unique_obj_id'].nunique() \
                                                             / self.log_len
        self.features['num_unique_stationary_objs/log_length'] = self.stat_objs_df['unique_obj_id'].nunique() \
                                                                 / self.log_len

    def mean_unique_objs_lifetime(self):
        #   Mean lifetime of unique objects
        count = self.df.groupby(['unique_obj_id'])['unique_obj_id'].count()
        self.features['mean_unique_objs_lifetime'] = count.mean()
        self.features['max_unique_objs_lifetime'] = count.max()

    def num_unique_objs_lifetime_in_range(self, less_than_range=(5, 10, 25), more_than_range=(50, 200, 500, 1000)):
        #   Number of unique objects with lifetime less than (5, 10, 25) and more than (50, 200, 500, 1000) scan indexes
        count = self.df.groupby(['unique_obj_id'])['unique_obj_id'].count()
        for threshold in less_than_range:
            self.features['num_unique_objs_lifetime_less' + str(threshold)] = count[count < threshold].count()
        for threshold in more_than_range:
            self.features['num_unique_objs_lifetime_more' + str(threshold)] = count[count > threshold].count()

    def mean_max_objs_distance_in_range(self, rng=(0, 2, 5, 10, 20, 40, 90, 150, 250)):
        """
        Mean and max number of objects with distance to host in range (0, 2, 5, 10, 20, 40, 90, 150, 250)
        :param rng: distance in meters
        :return:
        """
        self._mean_max_in_range(self.df, 'objs_distance_', rng, 'distance', 'm')

    def mean_max_moving_objs_speed_in_range(self, rng=(0, 1, 5, 10, 25, 50, 70, 90, 120, 150, 350)):
        """
        Mean and max number of moving objects with speed in range (0, 1, 5, 10, 25, 50, 70, 90, 120, 150, 350)
        :param rng: speed in km/h
        :return:
        """
        self._mean_max_in_range(self.moving_objs_df, 'moving_objs_speed_', rng, 'speed_km/h', 'km/h')

    def mean_max_stat_objs_speed_in_range(self, rng=(0, 0.1, 0.5, 1, 2, 5, 10, 25, 350)):
        """
        Mean and max number of stationary objects with speed in range (0, 0.1, 0.5, 1, 2, 5, 10, 25, 350)
        :param rng: speed in km/h
        :return:
        """
        self._mean_max_in_range(self.stat_objs_df, 'stat_objs_speed_', rng, 'speed_km/h', 'km/h')

    def mean_max_moving_objs_point_angle_in_range(self, rng=(0, 2.5, 5, 15, 30, 60, 75, 85, 95, 105, 120, 150, 165,
                                                             175, 180)):
        """
        Mean and max number of moving objects with pointing angle in range (0, 2.5, 5, 15, 30, 60, 75, 85, 95, 105, 120,
                                                                            150, 165,175, 180)
        :param rng: angle in degrees????
        :return:
        """
        self._mean_max_in_range(self.moving_speed_more_25, 'moving_objs_point_angle_+-', rng, 'pointing_angle',
                                'degrees', False, True)

    def mean_max_movable_objs_conf_lvl_in_range(self,
                                                rng=(0, 0.1, 0.3, 0.5, 0.7, 0.9, 0.95, 0.99, 0.999, 0.9999, 1.0000)):
        """
        Mean and max number of movable objects with confidence level in range (0, 0.1, 0.3, 0.5, 0.7, 0.9, 0.95, 0.99,
                                                                                0.999, 0.9999, 1.0000)
        :param rng: confidence level (0-1)
        :return:
        """
        self._mean_max_in_range(self.movable_objs_df, 'movable_unique_objs_conf_lvl_', rng, 'confidenceLevel', '', True)

    def mean_max_moving_objs_point_angle_spread(self):
        #   Mean and max pointing angle spread per scan index for moving objects with speed more than 25 km/h
        mean_, max_ = self._calc_mean_max_spread(self.moving_speed_more_25, 'pointing_angle')
        self.features['mean_moving_objs_point_angle_spread'] = mean_
        self.features['max_moving_objs_point_angle_spread'] = max_

    def mean_max_movable_objs_speed_spread(self):
        #   Mean and max speed spread per scan index for movable objects
        mean_, max_ = self._calc_mean_max_spread(self.movable_objs_df, 'speed_km/h')
        self.features['mean_movable_objs_speed_spread'] = mean_
        self.features['max_movable_objs_speed_spread'] = max_

    def mean_max_non_movable_objs_speed_spread(self):
        #   Mean and max speed spread per scan index for non movable objects
        mean_, max_ = self._calc_mean_max_spread(self.non_movable_objs_df, 'speed_km/h')
        self.features['mean_non_movable_objs_speed_spread'] = mean_
        self.features['max_non_movable_objs_speed_spread'] = max_

    def min_mean_max_obj_class(self):
        #   Min, mean and max number of objects per scan index that belongs to one of the object classes
        for obj_class_num in range(10):
            count = self.df[self.df['object_class'] == obj_class_num].groupby(['scan_idx'])['object_class'].count()
            self.features['min_num_of_' + ObjectClass(obj_class_num).name] = count.min()
            self.features['mean_num_of_' + ObjectClass(obj_class_num).name] = count.sum() / self.log_len
            self.features['max_num_of_' + ObjectClass(obj_class_num).name] = count.max()

    def _calc_mean_max(self, data, grp_by_signal):
        count = data.groupby([grp_by_signal])[grp_by_signal].count()
        return count.sum() / self.log_len, count.max()

    def _calc_mean_max_spread(self, data, grp_by_signal):
        grp_by = data.groupby(['scan_idx'])[grp_by_signal]
        return (grp_by.max() - grp_by.min()).sum() / self.log_len, (grp_by.max() - grp_by.min()).max()

    def _mean_max_in_range(self, df, f_name, rng, param, unit, f_nunique=False, f_abs=False):
        """

        :param df: filtered data frame
        :param f_name: feature name
        :param rng: range
        :param param: parameter name
        :param unit: unit ( m, km/h ...)
        :param nunique: boolean flag if count unique
        :return:
        """
        for i in range(len(rng) - 1):
            if f_nunique:
                count = df[(df[param] >= rng[i]) & (df[param] < rng[i + 1])].groupby(['scan_idx'])[
                    'unique_obj_id'].nunique()
            elif f_abs:
                count = df[(abs(df[param]) >= rng[i]) & (abs(df[param]) < rng[i + 1])].groupby(['scan_idx'])[
                    param].count()
            else:
                count = df[(df[param] >= rng[i]) & (df[param] < rng[i + 1])].groupby(['scan_idx'])[param].count()
            self.features[
                'mean_num_' + f_name + str(rng[i]) + '-' + str(rng[i + 1]) + unit] = count.sum() / self.log_len
            self.features['max_num_' + f_name + str(rng[i]) + '-' + str(rng[i + 1]) + unit] = count.max()


def example():
    from LogSets.utils.file_handling import load
    extracted_log_path = r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\Utils\example_extracted_logs' \
                         r'\example_extracted_log.pickle'

    extracted_log = load(extracted_log_path)['data']['objects']['signals']
    components = pd.DataFrame()
    components['path'] = pd.Series(extracted_log_path)
    FeaturesCalculator(extracted_log, components)
    features = FeaturesCalculator(extracted_log, components)
    components = features.calculate_all_available_features()
    # components = features.calculate_features(['mean_max_num_objs', 'mean_max_num_moving_objs',
    #                                           'mean_max_movable_objs_conf_lvl_in_range'])
    print()


if __name__ == '__main__':
    example()
