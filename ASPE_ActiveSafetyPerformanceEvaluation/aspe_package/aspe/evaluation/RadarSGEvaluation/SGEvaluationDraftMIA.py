import typing as tp

import numpy as np
import pandas as pd
import shapely as sh


def create_data(SG_estimated: pd.DataFrame, SG_reference: pd.DataFrame) -> (pd.DataFrame, pd.DataFrame):
    estimated = Segments(SG_estimated)
    reference = Segments(SG_reference)
    return estimated.SG_segments, reference.SG_segments


def pair(SG_segments_estimated: pd.DataFrame, SG_segments_reference: pd.DataFrame) -> (pd.DataFrame, pd.DataFrame):
    SG_pair_est_to_ref = SegmentToSegment(SG_segments_estimated, SG_segments_reference)
    SG_pair_ref_to_est = SegmentToSegment(SG_segments_reference, SG_segments_estimated)
    return SG_pair_est_to_ref.paired_df, SG_pair_ref_to_est.paired_df


def associate(pair_est_to_ref: pd.DataFrame, pair_ref_to_est: pd.DataFrame) -> (pd.DataFrame, pd.DataFrame):
    association_estimate = Association(pair_est_to_ref)
    association_reference = Association(pair_ref_to_est)
    return association_estimate.sample_df, association_reference.sample_df


def classify(association_estimate: pd.DataFrame, association_reference: pd.DataFrame,
             SG_segments_estimated: pd.DataFrame, SG_segments_reference: pd.DataFrame) -> (pd.DataFrame, pd.DataFrame):
    SG_classified_estimation = Classification(association_estimate)
    SG_classified_reference = Classification(association_reference)
    SG_estimation_output = add_samples_from_not_classified_segments(SG_classified_estimation.classified_df,
                                                                    SG_segments_estimated, 'FP')
    SG_reference_output = add_samples_from_not_classified_segments(SG_classified_reference.classified_df,
                                                                   SG_segments_reference, 'FN')

    return SG_estimation_output, SG_reference_output


def add_samples_from_not_classified_segments(classified_df: pd.DataFrame, SG_segments_estimated: pd.DataFrame,
                                             false_to_replace: str) -> pd.DataFrame:
    segment_unique_ids = classified_df['segment_unique_id'].drop_duplicates()
    segment_unique_ids = segment_unique_ids.to_numpy()
    f_classified = SG_segments_estimated.apply(lambda row: row['segment_unique_id'] in segment_unique_ids,
                                               axis=1)
    other_segments = SG_segments_estimated[~f_classified]
    if other_segments.empty:
        classified_df['BinaryClassification'].replace(False, false_to_replace, inplace=True)
        output = classified_df
    else:
        samples = create_samples_from_segments(other_segments)
        output = pd.concat([classified_df, samples])
        output['BinaryClassification'].replace(False, false_to_replace, inplace=True)
    return output


def create_samples_from_segments(segments: pd.DataFrame) -> pd.DataFrame:
    other_segments = segments.copy(deep=True)
    other_segments.drop('polygon', inplace=True, axis=1)
    other_segments['sample'] = other_segments.apply(_create_points_from_segment, axis=1)
    other_segments['sample_unique_id'] = other_segments.apply(_get_sample_unique_id, axis=1)
    samples = other_segments.explode(['sample', 'sample_unique_id'])
    samples['BinaryClassification'] = False
    return samples


class SGEvaluation:
    def __init__(self, SG_estimated: pd.DataFrame, SG_reference: pd.DataFrame):
        SG_segments_estimated, SG_segments_reference = create_data(SG_estimated, SG_reference)
        pair_est_to_ref, pair_ref_to_est = pair(SG_segments_estimated, SG_segments_reference)
        association_estimate, association_reference = associate(pair_est_to_ref, pair_ref_to_est)
        self.SG_estimation_output, self.SG_reference_output = classify(association_estimate, association_reference,
                                                                       SG_segments_estimated, SG_segments_reference)


class Segments:
    def __init__(self, SG_data: pd.DataFrame):
        self._SG_data = SG_data
        self.SG_segments = self._get_segments()

    def _get_segments(self) -> pd.DataFrame:
        SG_segments = self._SG_data[['scan_index', 'unique_id', 'position_x', 'position_y']]
        SG_segments['next_position_x'] = SG_segments['position_x'].shift(-1)
        SG_segments['next_position_y'] = SG_segments['position_y'].shift(-1)
        SG_segments = self._drop_last_vertex(SG_segments)

        SG_segments['segment'] = SG_segments.apply(self._create_segment, axis=1)
        SG_segments['polygon'] = SG_segments.apply(self._create_polygon, axis=1)
        SG_segments.reset_index(inplace=True)
        SG_segments['segment_unique_id'] = SG_segments.index
        return SG_segments[['scan_index', 'unique_id', 'segment_unique_id', 'segment', 'polygon']]

    @staticmethod
    def _drop_last_vertex(df: pd.DataFrame) -> pd.DataFrame:
        return df.drop(df.groupby(['unique_id', 'scan_index']).tail(1).index, axis=0)

    @staticmethod
    def _create_segment(row: pd.Series) -> sh.LineString:
        return sh.LineString([(row['position_x'], row['position_y']), (row['next_position_x'], row['next_position_y'])])

    @staticmethod
    def _create_polygon(row: pd.Series) -> sh.Polygon:
        return row['segment'].buffer(2, cap_style='flat')


class SegmentToSegment:
    def __init__(self, eval_segment: pd.DataFrame, assoc_segment: pd.DataFrame):
        self._eval_segment = eval_segment
        self._assoc_segment = assoc_segment
        self.paired_df = self._split_paired_df()
        self._get_segment_to_segment_parameters()

    def _split_paired_df(self) -> pd.DataFrame:
        df = self._get_segment_to_segment_pairs()
        f_valid_pairs = (df['paired'] == True)
        paired_df = (df.loc[f_valid_pairs, :]).copy(deep=True)
        paired_df['segment_paired_buffer'] = paired_df.apply(
            lambda row: row['segment_paired'].buffer(0.05, cap_style='flat'), axis=1)
        return paired_df

    def _get_segment_to_segment_pairs(self) -> pd.DataFrame:
        merged_df = pd.merge(self._eval_segment, self._assoc_segment, how='outer', on='scan_index',
                             suffixes=('', '_paired'))
        merged_df.dropna(subset=['unique_id'], inplace=True)
        merged_df['paired'] = merged_df.apply(self._does_segment_intersects_polygon, axis=1)

        # cast unique_id_paired to int
        merged_df['unique_id_paired'].fillna(-1, inplace=True)
        merged_df['segment_unique_id_paired'].fillna(-1, inplace=True)
        merged_df = merged_df.astype({'unique_id_paired': 'int32', 'segment_unique_id_paired': 'int32'})

        # drop not needed columns
        return merged_df.drop(['polygon_paired'], axis=1)

    def _get_segment_to_segment_parameters(self):
        self.paired_df['paired_parameters'] = self.paired_df.apply(
            lambda row: LineToLineParams(row['segment'], row['segment_paired']), axis=1)

    @staticmethod
    def _does_segment_intersects_polygon(row: pd.Series) -> bool:
        if row['segment_paired'] is np.nan:
            return False
        return row['segment_paired'].intersects(row['polygon'])


def _create_points_from_segment(row: pd.Series, distance_delta=0.2) -> np.array:
    distances = np.arange(0, row['segment'].length + distance_delta, distance_delta)
    points = [row['segment'].interpolate(distance) for distance in distances]
    return np.array([sample.coords[0] for sample in points])


def _get_sample_unique_id(row: pd.Series) -> np.array:
    sample_number, _ = row['sample'].shape
    return np.arange(0, sample_number)


class Association:
    def __init__(self, paired_df: pd.DataFrame):
        self._paired_df = paired_df
        self.sample_df = self._get_sample_association_df()

    def _get_sample_association_df(self) -> pd.DataFrame:
        self._paired_df['sample'] = self._paired_df.apply(lambda row: _create_points_from_segment(row), axis=1)
        self._paired_df['projection_sample'] = self._paired_df.apply(self._get_projection_samples, axis=1)
        self._paired_df['distance'] = self._paired_df.apply(self._get_distances, axis=1)
        self._paired_df['sample_unique_id'] = self._paired_df.apply(_get_sample_unique_id, axis=1)
        sample_df = self._get_sample_df(self._paired_df)

        return sample_df[
            ['scan_index', 'unique_id', 'segment', 'segment_unique_id', 'unique_id_paired', 'polygon', 'segment_paired',
             'segment_paired_buffer',
             'sample', 'projection_sample', 'distance', 'sample_unique_id']]

    @staticmethod
    def _get_projection_samples(row: pd.Series) -> tp.List:
        intersection_Px, intersection_Py = row['paired_parameters'].intersection_point
        # parallel lines
        if intersection_Px is None:
            base_Px, base_Py = row['segment_paired'].coords[0]
            base_P = np.array([base_Px, base_Py])
            sample_base_vector = row['sample'] - base_P
            projection_length = np.array(
                [np.dot(vector, row['paired_parameters'].paired_unit_direction_vector) for vector in
                 sample_base_vector])
            output = [(length * row['paired_parameters'].paired_unit_direction_vector + base_P) for
                      length in
                      projection_length]
        else:
            intersection_P = np.array([intersection_Px, intersection_Py])
            sample_intersection_vector = row['sample'] - intersection_P
            projection_length = np.array(
                [np.linalg.norm(vector) / np.dot(row['paired_parameters'].unit_direction_vector,
                                                 row['paired_parameters'].paired_unit_direction_vector) for vector
                 in
                 sample_intersection_vector])

            output = [(length * row['paired_parameters'].paired_unit_direction_vector + intersection_P) for
                      length in
                      projection_length]
        return output

    @staticmethod
    def _get_distances(row: pd.Series) -> tp.List:
        distances = row['sample'] - row['projection_sample']
        return np.apply_along_axis(np.linalg.norm, axis=1, arr=distances)

    @staticmethod
    def _get_sample_df(df: pd.DataFrame) -> pd.DataFrame:
        return df.explode(['sample', 'projection_sample', 'distance', 'sample_unique_id'])


class Classification:
    def __init__(self, associated_df: pd.DataFrame):
        self._samples_df = associated_df
        self.classified_df = self._get_classified_df()

    def _get_classified_df(self) -> pd.DataFrame:
        f_sample_in_polygon = self._samples_df.apply(self._is_in_polygon,
                                                     axis=1)
        f_sample_on_paired_segment = self._samples_df.apply(self._is_sample_on_segment,
                                                            axis=1)
        self._samples_df['BinaryClassification'] = f_sample_in_polygon & f_sample_on_paired_segment

        self._samples_df = self._samples_df.sort_values('distance', ascending=True)
        self._samples_df = self._samples_df.drop_duplicates(
            subset=['scan_index', 'segment_unique_id', 'sample_unique_id', 'BinaryClassification'], keep='first')
        self._samples_df = self._samples_df.sort_values('BinaryClassification', ascending=False)
        self._samples_df = self._samples_df.drop_duplicates(
            subset=['scan_index', 'segment_unique_id', 'sample_unique_id'], keep='first')

        self._samples_df['BinaryClassification'].replace(True, 'TP', inplace=True)
        # keep name convention from second method
        self._samples_df.rename(columns={'unique_id_paired': 'unique_id_associated'}, inplace=True)
        return self._samples_df[
            ['scan_index', 'unique_id', 'segment_unique_id', 'sample_unique_id', 'segment', 'unique_id_associated',
             'segment_paired', 'sample',
             'projection_sample', 'distance', 'BinaryClassification']]

    @staticmethod
    def _is_in_polygon(row: pd.Series) -> sh.Point:
        return sh.Point(row['projection_sample']).intersects(row['polygon'])

    @staticmethod
    def _is_sample_on_segment(row: pd.Series) -> sh.Point:
        return sh.Point(row['projection_sample']).intersects(row['segment_paired_buffer'])


class LineToLineParams:
    def __init__(self, line: sh.LineString, paired_line: sh.LineString):
        self._line_params = LineParam(line)
        self._paired_line_params = LineParam(paired_line)
        self.intersection_point = self._calculate_intersection_point()
        self.paired_unit_direction_vector = self._paired_line_params.unit_direction_vector
        self.unit_direction_vector = self._line_params.unit_direction_vector

    def _calculate_intersection_point(self) -> tp.Tuple:
        c1, m1, b1 = self._line_params.line_equation_coefficients
        c2, m2, b2 = self._paired_line_params.line_equation_coefficients
        if (c1 * m2 - c2 * m1) != 0:
            px = (c2 * b1 - c1 * b2) / (c1 * m2 - c2 * m1)
            py = (-m1 * px - b1) / c1 if c1 != 0 else (-m2 * px - b2) / c2
        else:
            # parallel lines
            px = None
            py = None
        return px, py


class LineParam:
    def __init__(self, line: sh.LineString):
        self._line = line
        self._x1, self._y1 = self._line.coords[0]
        self._x2, self._y2 = self._line.coords[1]
        self.unit_direction_vector = self._get_unit_direction_vector()
        self.unit_perpendicular_vector = self._get_unit_perpendicular_vector()
        self.line_equation_coefficients = self._get_line_equation()

    def _get_unit_direction_vector(self) -> np.array:
        direction_vector = np.array([self._x2 - self._x1, self._y2 - self._y1])
        return direction_vector / np.linalg.norm(direction_vector)

    def _get_unit_perpendicular_vector(self) -> np.array:
        return np.array([self.unit_direction_vector[1], -self.unit_direction_vector[0]])

    def _get_line_equation(self):
        """
        Finding segment line equation based on segment points:
        cy+mx+b=0
        """
        if (self._x2 - self._x1) != 0:
            m = (self._y2 - self._y1) / (self._x2 - self._x1)
            b = self._y1 - m * self._x1
            c = -1
        else:
            c = 0
            b = 0
            m = -self._x1

        return c, m, b


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

    outputs = SGEvaluation(estimated_contours, reference_contours)
