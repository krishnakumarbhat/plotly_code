import pytest
from pandas import DataFrame, testing
from aspe.evaluation.RadarSGEvaluation.DataStructures.SGSegmentPairs import SGSegmentPairs


def test_SGSegmentPairs_init():
    expected_signals_df = DataFrame.from_dict(
        {'scan_index': {}, 'timestamp': {}, 'unique_id': {}, 'segment_unique_id': {}, 'segment_unique_id_paired': {},
         'projection_start_position_x': {}, 'projection_start_position_y': {}, 'unit_vector_x_paired': {},
         'unit_vector_y_paired': {}, 'dot_product': {}})
    sg_segment_pairs = SGSegmentPairs()
    assert testing.assert_frame_equal(expected_signals_df, sg_segment_pairs.signals, check_dtype=False) is None


def test_SGSegmentPairs_insert_pairs_valid_data():
    expected_signals_df = DataFrame.from_dict(
        {'scan_index': {0: 0}, 'timestamp': {0: 0}, 'unique_id': {0: 0}, 'segment_unique_id': {0: 0},
         'segment_unique_id_paired': {0: 0}, 'projection_start_position_x': {0: 0},
         'projection_start_position_y': {0: 0},
         'unit_vector_x_paired': {0: 0},
         'unit_vector_y_paired': {0: 0}, 'dot_product': {0: 0}})
    sg_segment_pairs = SGSegmentPairs()
    sg_segment_pairs.insert_pairs(expected_signals_df)
    assert testing.assert_frame_equal(expected_signals_df, sg_segment_pairs.signals, check_dtype=False) is None


def test_SGSegmentPairs_insert_pairs_invalid_data():
    error_msg = "Missing columns inside SGSegmentPairs dataframe!"
    with pytest.raises(AttributeError, match=error_msg):
        expected_signals_df = DataFrame.from_dict(
            {'timestamp': {0: 0}, 'unique_id': {0: 0}, 'segment_unique_id': {0: 0},
             'segment_unique_id_paired': {0: 0}, 'projection_start_position_x': {0: 0},
             'projection_start_position_y': {0: 0},
             'unit_vector_x_paired': {0: 0},
             'unit_vector_y_paired': {0: 0}, 'dot_product': {0: 0}})
        sg_segment_pairs = SGSegmentPairs()
        sg_segment_pairs.insert_pairs(expected_signals_df)


def test_SGSegmentPairs_get_base_name():
    assert 'SGSegmentPairs' == SGSegmentPairs().get_base_name()
