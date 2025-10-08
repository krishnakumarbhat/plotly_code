import numpy as np
import pandas as pd
import shapely as sh

from aspe.evaluation.RadarSGEvaluation.DataStructures.SGSegmentPairs import SGSegmentPairs

DEFAULT_PAIRS_ASSOCIATION_DISTANCE = 2.0

"""
KPI is looking for projection points of evaluated
segments on associated segments. Projection line 
in this case is perpendicular to base segment.
"""


def build_pairs(segments_est: pd.DataFrame,
                segments_ref: pd.DataFrame,
                pairs_association_distance: float = DEFAULT_PAIRS_ASSOCIATION_DISTANCE) \
        -> (SGSegmentPairs, SGSegmentPairs):
    """
    Creates data frame with paired estimated and reference segments.

    :param segments_est: frame of estimated segments created based on Stationary Geometries Data
    :type segments_est: pd.DataFrame
    :param segments_ref: frame of reference segments created based on Stationary Geometries Data
    :type segments_ref: pd.DataFrame
    :param pairs_association_distance: maximum distance between paired segments
    :type pairs_association_distance: float

    :rtype: SGSegmentPairs, SGSegmentPairs
    """
    valid_pairs = _merge_segments_dfs(segments_est, segments_ref, pairs_association_distance)

    valid_pairs.reset_index(inplace=True)
    valid_pairs['unique_id'] = valid_pairs.index

    segment_pairs_est = _get_valid_estimated_pairs(valid_pairs)
    segment_pairs_ref = _get_valid_reference_pairs(valid_pairs)

    return segment_pairs_est, segment_pairs_ref


def _get_valid_estimated_pairs(valid_pairs: pd.DataFrame):
    valid_pairs_est = valid_pairs.rename(
        columns={'segment_unique_id_est': 'segment_unique_id', 'segment_unique_id_ref': 'segment_unique_id_paired'})

    valid_pairs_est['projection_start_position_x'], valid_pairs_est['projection_start_position_y'], \
        valid_pairs_est['unit_vector_x_paired'], valid_pairs_est['unit_vector_y_paired'], valid_pairs_est[
        'dot_product'] = _get_paired_segments_parameters(
        valid_pairs_est['start_position_x_est'].to_numpy(),
        valid_pairs_est['start_position_y_est'].to_numpy(),
        valid_pairs_est['end_position_x_est'].to_numpy(),
        valid_pairs_est['end_position_y_est'].to_numpy(),
        valid_pairs_est['start_position_x_ref'].to_numpy(),
        valid_pairs_est['start_position_y_ref'].to_numpy(),
        valid_pairs_est['end_position_x_ref'].to_numpy(),
        valid_pairs_est['end_position_y_ref'].to_numpy())

    segment_pairs_est = SGSegmentPairs()
    segment_pairs_est.insert_pairs(valid_pairs_est)
    return segment_pairs_est


def _get_valid_reference_pairs(valid_pairs: pd.DataFrame):
    valid_pairs_ref = valid_pairs.rename(
        columns={'segment_unique_id_est': 'segment_unique_id_paired', 'segment_unique_id_ref': 'segment_unique_id'})

    valid_pairs_ref['projection_start_position_x'], valid_pairs_ref['projection_start_position_y'], \
        valid_pairs_ref['unit_vector_x_paired'], valid_pairs_ref['unit_vector_y_paired'], valid_pairs_ref[
        'dot_product'] = _get_paired_segments_parameters(
        valid_pairs_ref['start_position_x_ref'].to_numpy(),
        valid_pairs_ref['start_position_y_ref'].to_numpy(),
        valid_pairs_ref['end_position_x_ref'].to_numpy(),
        valid_pairs_ref['end_position_y_ref'].to_numpy(),
        valid_pairs_ref['start_position_x_est'].to_numpy(),
        valid_pairs_ref['start_position_y_est'].to_numpy(),
        valid_pairs_ref['end_position_x_est'].to_numpy(),
        valid_pairs_ref['end_position_y_est'].to_numpy())

    segment_pairs_ref = SGSegmentPairs()
    segment_pairs_ref.insert_pairs(valid_pairs_ref)
    return segment_pairs_ref


def _merge_segments_dfs(estimated_df: pd.DataFrame, reference_df: pd.DataFrame,
                        pairs_association_distance: float) -> pd.DataFrame:
    """
    Pairing estimated and reference segments in one dataframe.

    :param estimated_df: estimated segments
    :type estimated_df:  pd.DataFrame
    :param reference_df: reference segments
    :type reference_df: pd.DataFrame
    :param pairs_association_distance: maximum distance between paired segments
    :type pairs_association_distance: float

    :rtype: pd.DataFrame
    """
    merged_df = pd.merge(estimated_df, reference_df, how='outer', on='scan_index',
                         suffixes=('_est', '_ref'))
    merged_df.rename(columns={'unique_id_est': 'segment_unique_id_est',
                              'unique_id_ref': 'segment_unique_id_ref',
                              'timestamp_est': 'timestamp'},
                     inplace=True)
    merged_df.dropna(inplace=True)

    if not merged_df.empty:
        _pair_segments_within_distance_vectorized = np.vectorize(_pair_segments_within_distance)
        merged_df['paired'] = _pair_segments_within_distance_vectorized(merged_df['start_position_x_est'],
                                                                        merged_df['start_position_y_est'],
                                                                        merged_df['end_position_x_est'],
                                                                        merged_df['end_position_y_est'],
                                                                        merged_df['start_position_x_ref'],
                                                                        merged_df['start_position_y_ref'],
                                                                        merged_df['end_position_x_ref'],
                                                                        merged_df['end_position_y_ref'],
                                                                        pairs_association_distance)

        merged_df = _drop_non_paired(merged_df)

    return merged_df


def _pair_segments_within_distance(start_position_x,
                                   start_position_y,
                                   end_position_x,
                                   end_position_y,
                                   start_position_paired_x,
                                   start_position_paired_y,
                                   end_position_paired_x,
                                   end_position_paired_y,
                                   pairs_association_distance):
    """
    Pairing of estimated and reference segments
    in one dataframe.
    """
    segment = sh.LineString(
        [(start_position_x, start_position_y), (end_position_x, end_position_y)])
    segment_paired = sh.LineString(
        [(start_position_paired_x, start_position_paired_y),
         (end_position_paired_x, end_position_paired_y)])

    return segment.distance(segment_paired) < pairs_association_distance


def _drop_non_paired(raw_pairs: pd.DataFrame) -> pd.DataFrame:
    """
    Removing rows which does not contain paired segments.

    :param raw_pairs: all paired segments
    :type raw_pairs: pd.DataFrame

    :rtype: pd.DataFrame
    """

    mask_of_valid_pairs = (raw_pairs['paired'].values == True)
    return raw_pairs.loc[mask_of_valid_pairs, :].copy()


@np.errstate(invalid='ignore')
def _get_paired_segments_parameters(start_position_x,
                                    start_position_y,
                                    end_position_x,
                                    end_position_y,
                                    start_position_x_paired,
                                    start_position_y_paired,
                                    end_position_x_paired,
                                    end_position_y_paired):
    position_x_prim, position_y_prim = get_point_based_on_given_projection_point(start_position_x,
                                                                                 start_position_y,
                                                                                 end_position_x,
                                                                                 end_position_y,
                                                                                 start_position_x_paired,
                                                                                 start_position_y_paired)

    projection_start_position_x, projection_start_position_y = get_base_paired_segment_point(start_position_x,
                                                                                             start_position_y,
                                                                                             position_x_prim,
                                                                                             position_y_prim,
                                                                                             start_position_x_paired,
                                                                                             start_position_y_paired,
                                                                                             end_position_x_paired,
                                                                                             end_position_y_paired)

    unit_segment_x, unit_segment_y = get_normalized_vector(start_position_x, start_position_y, end_position_x,
                                                           end_position_y)

    f_same_points = ((projection_start_position_x == end_position_x_paired) &
                     (projection_start_position_y == end_position_y_paired))

    position_x_paired, position_y_paired = end_position_x_paired, end_position_y_paired

    position_x_paired[f_same_points] = start_position_x_paired[f_same_points]
    position_y_paired[f_same_points] = start_position_y_paired[f_same_points]

    unit_vector_x_paired, unit_vector_y_paired = get_normalized_vector(projection_start_position_x,
                                                                       projection_start_position_y,
                                                                       position_x_paired,
                                                                       position_y_paired)

    dot_product = unit_vector_x_paired * unit_segment_x + unit_vector_y_paired * unit_segment_y

    return (projection_start_position_x, projection_start_position_y,
            unit_vector_x_paired, unit_vector_y_paired, dot_product)


def get_point_based_on_given_projection_point(start_position_x,
                                              start_position_y,
                                              end_position_x,
                                              end_position_y,
                                              projection_position_x,
                                              projection_position_y):
    """
    Takes segment and point from other segments.
    It returns point on [start, end] segment
    given its counterpart - projection.

    :return: base_segment_position_x, base_segment_position_y
    """

    unit_segment_x, unit_segment_y = get_normalized_vector(start_position_x,
                                                           start_position_y,
                                                           end_position_x,
                                                           end_position_y)
    vector_x, vector_y = get_vector(start_position_x,
                                    start_position_y,
                                    projection_position_x,
                                    projection_position_y)
    dot_product = vector_x * unit_segment_x + vector_y * unit_segment_y

    return dot_product * unit_segment_x + start_position_x, dot_product * unit_segment_y + start_position_y


@np.errstate(all='ignore')
def get_base_paired_segment_point(start_position_x,
                                  start_position_y,
                                  position_x_prim,
                                  position_y_prim,
                                  start_position_x_paired,
                                  start_position_y_paired,
                                  end_position_x_paired,
                                  end_position_y_paired):
    """
    Finding projection point for segments starting point.

    :return: projection_start_position_x, projection_start_position_y
    """

    segment_prim_x, segment_prim_y = get_vector(start_position_x, start_position_y, position_x_prim, position_y_prim)

    position_prim_length = get_vector_length(segment_prim_x, segment_prim_y)

    unit_position_prim_x = np.divide(segment_prim_x, position_prim_length)
    unit_position_prim_y = np.divide(segment_prim_y, position_prim_length)

    unit_segment_paired_x, unit_segment_paired_y = get_normalized_vector(start_position_x_paired,
                                                                         start_position_y_paired,
                                                                         end_position_x_paired, end_position_y_paired)

    dot_product = unit_segment_paired_x * unit_position_prim_x + unit_segment_paired_y * unit_position_prim_y

    projection_length = position_prim_length / dot_product

    projection_start_position_x = start_position_x_paired - projection_length * unit_segment_paired_x
    projection_start_position_y = start_position_y_paired - projection_length * unit_segment_paired_y

    f_same_points = ((position_x_prim == start_position_x) &
                     (position_y_prim == start_position_y))

    projection_start_position_x[f_same_points] = start_position_x_paired[f_same_points]
    projection_start_position_y[f_same_points] = start_position_y_paired[f_same_points]

    return projection_start_position_x, projection_start_position_y


def get_normalized_vector(start_position_x,
                          start_position_y,
                          end_position_x,
                          end_position_y):
    segment_x, segment_y = get_vector(start_position_x,
                                      start_position_y,
                                      end_position_x,
                                      end_position_y)
    scaling_factor = np.divide(1, get_vector_length(segment_x, segment_y))
    return scaling_factor * segment_x, scaling_factor * segment_y


def get_vector(start_position_x,
               start_position_y,
               end_position_x,
               end_position_y):
    return end_position_x - start_position_x, end_position_y - start_position_y


def get_vector_length(vector_x, vector_y):
    return np.sqrt(vector_x ** 2 + vector_y ** 2)
