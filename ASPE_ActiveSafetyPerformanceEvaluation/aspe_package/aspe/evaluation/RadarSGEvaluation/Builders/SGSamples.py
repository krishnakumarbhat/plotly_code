import numpy as np
import pandas as pd

from aspe.evaluation.RadarSGEvaluation.DataStructures.SGSamples import SGSamples

"""
KPI is looking for projection points of evaluated
segments on associated segments. Projection line 
in this case is perpendicular to base segment.
"""


def associate_samples(segments: pd.DataFrame, segments_associated: pd.DataFrame,
                      segment_pairs: pd.DataFrame, distance_threshold=2.0, sample_interval=0.5) -> SGSamples:
    """
    Function takes paired segments and return samples
    with projection samples (associated) on paired segments.

    :param segments: frame with evaluated Segments
    :type segments: pd.DataFrame
    :param segments_associated:  frame with associated Segments
    :type segments_associated: pd.DataFrame
    :param segment_pairs: frame with association data
    :type segment_pairs: pd.DataFrame
    :param distance_threshold: maximum distance between valid samples
    :type distance_threshold: float
    :param sample_interval: distance between samples
    :type sample_interval: float

    :rtype: SGSamples
    """
    sample_position_x, sample_position_y = _get_point_positions(
        segments['start_position_x'].to_numpy(),
        segments['start_position_y'].to_numpy(),
        segments['end_position_x'].to_numpy(),
        segments['end_position_y'].to_numpy(),
        sample_interval)
    segments['sample_position_x'] = list(sample_position_x)
    segments['sample_position_y'] = list(sample_position_y)
    samples_df = _add_associated_segments_to_samples(segments, segments_associated, segment_pairs)
    samples_df.reset_index(inplace=True)
    samples_df['unique_id'] = samples_df.index
    samples_df['projection_sample_position_x'], samples_df['projection_sample_position_y'] = _projection_point(
        samples_df['sample_position_x'].to_numpy(),
        samples_df['sample_position_y'].to_numpy(),
        samples_df['start_position_x'].to_numpy(),
        samples_df['start_position_y'].to_numpy(),
        samples_df['projection_start_position_x'].to_numpy(),
        samples_df['projection_start_position_y'].to_numpy(),
        samples_df['unit_vector_x_paired'].to_numpy(),
        samples_df['unit_vector_y_paired'].to_numpy(),
        samples_df['dot_product'].to_numpy())
    _calculate_parameters(samples_df, distance_threshold)
    _remove_duplicated_results(samples_df)
    samples = SGSamples()
    samples.insert_samples(samples_df)
    return samples


def _calculate_parameters(samples_df: pd.DataFrame, distance_threshold):
    samples_df['deviation_x'] = _get_deviation(samples_df['sample_position_x'].to_numpy(),
                                               samples_df['projection_sample_position_x'].to_numpy())
    samples_df['deviation_y'] = _get_deviation(samples_df['sample_position_y'].to_numpy(),
                                               samples_df['projection_sample_position_y'].to_numpy())
    samples_df['distance'] = _get_distance(samples_df['deviation_x'].to_numpy(),
                                           samples_df['deviation_y'].to_numpy())
    samples_df['classification'] = _get_classification(samples_df['projection_sample_position_x'].to_numpy(),
                                                       samples_df['projection_sample_position_y'].to_numpy(),
                                                       samples_df['start_position_x_paired'].to_numpy(),
                                                       samples_df['start_position_y_paired'].to_numpy(),
                                                       samples_df['end_position_x'].to_numpy(),
                                                       samples_df['end_position_y'].to_numpy(),
                                                       samples_df['distance'].to_numpy(),
                                                       distance_threshold)
    samples_df['multiple_segmentation'] = samples_df.duplicated(
        subset=['scan_index', 'sample_position_x', 'sample_position_y', 'classification'], keep=False)


def _remove_duplicated_results(samples_df: pd.DataFrame):
    samples_df.sort_values('distance', ascending=True, inplace=True)
    samples_df.drop_duplicates(
        subset=['sample_position_x', 'sample_position_y',
                'scan_index', 'classification', 'segment_unique_id'],
        keep='first', inplace=True)
    samples_df.sort_values('classification', ascending=False, inplace=True)
    samples_df.drop_duplicates(subset=['sample_position_x', 'sample_position_y',
                                       'scan_index', 'segment_unique_id'],
                               keep='first', inplace=True)


def _add_associated_segments_to_samples(segments: pd.DataFrame, segments_associated: pd.DataFrame,
                                        segment_pairs: pd.DataFrame):
    samples_df = segments[
        ['scan_index', 'timestamp', 'sample_position_x', 'sample_position_y', 'unique_id', 'start_position_x',
         'start_position_y']]
    samples_df = samples_df.explode(['sample_position_x', 'sample_position_y'])
    samples_df = pd.merge(samples_df, segment_pairs,
                          how='left',
                          left_on='unique_id',
                          right_on='segment_unique_id',
                          suffixes=('', '_paired'))
    samples_df.drop(columns=['scan_index_paired', 'timestamp_paired', 'segment_unique_id', 'unique_id_paired'],
                    axis=1,
                    inplace=True)
    samples_df = pd.merge(samples_df, segments_associated[
        ['unique_id', 'start_position_x', 'start_position_y', 'end_position_x', 'end_position_y']],
                          right_on='unique_id',
                          left_on='segment_unique_id_paired',
                          suffixes=('', '_paired'),
                          how='left')
    samples_df.rename(columns={'unique_id': 'segment_unique_id'}, inplace=True)
    return samples_df


def _get_point_positions(start_position_x, start_position_y, end_position_x, end_position_y, sample_interval):
    x_length = end_position_x - start_position_x
    y_length = end_position_y - start_position_y
    length = (x_length ** 2 + y_length ** 2) ** 0.5
    number_of_points = np.ceil(length / sample_interval).astype(int) + 1
    points_positions_x = (np.linspace(a, b, c) for a, b, c in zip(start_position_x, end_position_x, number_of_points))
    points_positions_y = (np.linspace(a, b, c) for a, b, c in zip(start_position_y, end_position_y, number_of_points))
    return points_positions_x, points_positions_y


def _get_deviation(point_position, projection_point_position):
    return point_position - projection_point_position


def _get_distance(deviation_x, deviation_y):
    return (deviation_x ** 2 + deviation_y ** 2) ** 0.5


def _is_on_paired_segment(projection_point_x,
                          projection_point_y,
                          start_position_x,
                          start_position_y,
                          end_position_x,
                          end_position_y):
    """
    Function checks whether projection point
    belongs to segment.
    There is an assumption that projection point
    lies at line defined by (start,end) points.
    """
    x_is_between = _is_between(projection_point_x, start_position_x, end_position_x)
    y_is_between = _is_between(projection_point_y, start_position_y, end_position_y)
    return x_is_between & y_is_between


@np.errstate(invalid='ignore')
def _is_between(position, start_position, end_position, error=0.05):
    min_position = np.minimum(start_position, end_position) - error
    max_position = np.maximum(start_position, end_position) + error
    return np.greater_equal(position, min_position) & np.greater_equal(max_position, position)


@np.errstate(invalid='ignore')
def _is_within_distance(distance, distance_threshold):
    return distance <= distance_threshold


def _get_classification(projection_sample_position_x,
                        projection_sample_position_y,
                        start_position_x,
                        start_position_y,
                        end_position_x,
                        end_position_y,
                        distance,
                        distance_threshold):
    f_is_on_paired_segment = _is_on_paired_segment(projection_sample_position_x,
                                                   projection_sample_position_y,
                                                   start_position_x,
                                                   start_position_y,
                                                   end_position_x,
                                                   end_position_y)
    f_is_within_distance = _is_within_distance(distance, distance_threshold)
    return f_is_on_paired_segment & f_is_within_distance


@np.errstate(all='ignore')
def _projection_point(point_position_x,
                      point_position_y,
                      start_position_x,
                      start_position_y,
                      projection_start_position_x,
                      projection_start_position_y,
                      unit_projection_vector_x,
                      unit_projection_vector_y,
                      dot_product):
    length = ((point_position_x - start_position_x) ** 2 + (point_position_y - start_position_y) ** 2) ** 0.5
    projection_length = length / dot_product
    return (projection_start_position_x + projection_length * unit_projection_vector_x,
            projection_start_position_y + projection_length * unit_projection_vector_y)
