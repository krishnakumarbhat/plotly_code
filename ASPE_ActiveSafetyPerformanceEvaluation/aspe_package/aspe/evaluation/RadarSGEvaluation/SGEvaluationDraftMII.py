import numpy as np
import pandas as pd
import shapely as sh

"""
This is first draft implementation of METHOD II of KPI calculations
In this approach whole contour is taken as one LineString,
Evaluated data are samples. 
Association data are contours.
Sample inside contour association gate is TP,   
outside FP/FN.
"""


def create_data(SG_estimated_data: pd.DataFrame, SG_reference_data: pd.DataFrame) -> (object, object):
    estimated = Create(SG_estimated_data)
    reference = Create(SG_reference_data)
    return estimated, reference


def associate(estimated: object, reference: object) -> (pd.DataFrame, pd.DataFrame):
    est_to_ref = Associate(estimated.samples.SG_samples, reference.contours.SG_contours)
    ref_to_est = Associate(reference.samples.SG_samples, estimated.contours.SG_contours)
    return est_to_ref.associated_df, ref_to_est.associated_df


def classify(SG_estimated_samples: pd.DataFrame, SG_reference_samples: pd.DataFrame) -> (pd.DataFrame, pd.DataFrame):
    SG_estimated = classification(SG_estimated_samples, 'FP')
    SG_reference = classification(SG_reference_samples, 'FN')
    return SG_estimated, SG_reference


def classification(SG_samples: pd.DataFrame, false_value: str) -> pd.DataFrame:
    SG_samples.rename(columns={'associated': 'BinaryClassification'}, inplace=True)
    SG_samples['BinaryClassification'].replace(True, 'TP', inplace=True)
    SG_samples['BinaryClassification'].replace(False, false_value, inplace=True)
    SG_samples['sample'] = SG_samples.apply(lambda row: np.array(row['sample'].coords[0]), axis=1)
    return SG_samples


class SGEvaluation:
    def __init__(self, SG_estimated_data: pd.DataFrame, SG_reference_data: pd.DataFrame):
        estimated, reference = create_data(SG_estimated_data, SG_reference_data)
        estimated_samples_df, reference_samples_df = associate(estimated, reference)
        self.SG_estimation, self.SG_reference = classify(estimated_samples_df, reference_samples_df)


class Create:
    def __init__(self, SG_data: pd.DataFrame):
        self.contours = Contours(SG_data)
        self.samples = Samples(self.contours.SG_contours)


class Associate:
    def __init__(self, SG_samples: pd.DataFrame, SG_contours: pd.DataFrame):
        self._SG_samples = SG_samples
        self._SG_contours = SG_contours
        self.associated_df = self._associate_points_to_contour()

    def _associate_points_to_contour(self) -> pd.DataFrame:
        self._SG_samples['sample_unique_id'] = np.arange(0, len(self._SG_samples))

        # merge samples with contours in same scan_index
        merged_df = pd.merge(self._SG_samples, self._SG_contours, how='outer', on='scan_index',
                             suffixes=('', '_associated'))

        merged_df.dropna(subset=['unique_id'], inplace=True)

        # check if sample is in associated contour buffer and distance
        merged_df['associated'] = merged_df.apply(self._polygon_contains_point, axis=1)
        merged_df['distance'] = merged_df.apply(self._calculate_distance, axis=1)

        # leave only best matches, TP if exists and smallest distances
        merged_df = merged_df.sort_values(['sample_unique_id', 'associated', 'distance'], ascending=True)
        valid_association_df = merged_df.drop_duplicates(subset=['sample_unique_id', 'associated'], keep='first')
        valid_association_df = valid_association_df.drop_duplicates(subset=['sample_unique_id'], keep='last')

        # cast unique_id_associated to int
        valid_association_df['unique_id_associated'].fillna(-1, inplace=True)
        valid_association_df = valid_association_df.astype({'unique_id_associated': 'int32'})

        return valid_association_df[
            ['scan_index', 'unique_id', 'sample', 'sample_unique_id', 'unique_id_associated', 'associated',
             'distance']]

    @staticmethod
    def _polygon_contains_point(row: pd.Series) -> bool:
        if row['polygon'] is np.nan:
            return False
        return row['polygon'].intersects(row['sample'])

    @staticmethod
    def _calculate_distance(row: pd.Series) -> float:
        if row['associated'] is False:
            return np.nan
        return row['sample'].distance(row['contours'])


class Contours:
    def __init__(self, SG_data: pd.DataFrame):
        self._SG_data = SG_data
        self.SG_contours = self._get_contours()

    def _get_contours(self) -> pd.DataFrame:
        SG_contours = self._SG_data.copy()
        SG_contours = SG_contours[['scan_index', 'unique_id']].drop_duplicates(subset=['scan_index', 'unique_id'])
        SG_contours['contours'] = None
        SG_contours['polygon'] = None

        for name, group in self._SG_data.groupby(['scan_index', 'unique_id']):
            x_es = group['position_x']
            y_es = group['position_y']
            contour = sh.LineString(list(zip(x_es, y_es)))
            group_scan_index, group_unique_id = name
            f_valid = ((SG_contours['scan_index'] == group_scan_index) &
                       (SG_contours['unique_id'] == group_unique_id))
            SG_contours.loc[f_valid, 'contours'] = contour
        SG_contours['polygon'] = SG_contours.apply(self._create_contour_buffer, axis=1)
        return SG_contours

    @staticmethod
    def _create_contour_buffer(row: pd.Series) -> sh.Polygon:
        return row['contours'].buffer(2, join_style='mitre', cap_style='flat')


class Samples:
    def __init__(self, SG_contours: pd.DataFrame):
        self._SG_contours = SG_contours
        self.SG_samples = self._get_samples()

    def _get_samples(self) -> pd.DataFrame:
        SG_contours = self._SG_contours.copy()
        SG_contours['sample'] = SG_contours.apply(self._create_points_from_contour, axis=1)
        SG_contours = SG_contours.drop(['contours', 'polygon'], axis=1)
        return SG_contours.explode('sample')

    @staticmethod
    def _create_points_from_contour(row: pd.Series) -> list:
        distance_delta = 0.2
        distances = np.arange(0, row['contours'].length + distance_delta, distance_delta)
        return [row['contours'].interpolate(distance) for distance in distances]


if __name__ == "__main__":
    estimated_contours = pd.DataFrame(
        {
            'scan_index': [0, 0, 0, 1, 1, 1, 3, 3],
            'unique_id': [1, 1, 1, 1, 1, 1, 4, 4],
            'position_x': [1, 3, 5, 1, 3, 5, 1, 3],
            'position_y': [2, 3, 3, 2, 3, 3, 2, 5],
        })

    reference_contours = pd.DataFrame(
        {
            'scan_index': [0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1],
            'unique_id': [1, 1, 1, 2, 2, 1, 1, 1, 2, 2, 3, 3],
            'position_x': [0.9, 3.1, 6, 2, 3, 0.9, 3.1, 6, 2, 3, 100, 101],
            'position_y': [2.2, 3.2, 3.2, 2, 2.5, 2.2, 3.2, 3.2, 2, 2.5, 100, 101],
        })

    output = SGEvaluation(estimated_contours, reference_contours)
