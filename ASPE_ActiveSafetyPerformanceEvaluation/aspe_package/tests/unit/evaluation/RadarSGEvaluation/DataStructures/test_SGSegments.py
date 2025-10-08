import pytest
from pandas import DataFrame, testing
from aspe.evaluation.RadarSGEvaluation.DataStructures.SGSegments import SGSegments


def test_SGSegments_init():
    expected_signals_df = DataFrame.from_dict(
        {'scan_index': {}, 'timestamp': {}, 'unique_id': {}, 'start_position_x': {}, 'start_position_y': {},
         'end_position_x': {}, 'end_position_y': {}, 'contour_unique_id': {}})
    sg_segments = SGSegments()
    assert testing.assert_frame_equal(expected_signals_df, sg_segments.signals, check_dtype=False) is None


def test_SGSegments_insert_segments_valid_data():
    expected_signals_df = DataFrame.from_dict(
        {'scan_index': {0: 0}, 'timestamp': {0: 0}, 'unique_id': {0: 0}, 'start_position_x': {0: 0.0},
         'start_position_y': {0: 0.0}, 'end_position_x': {0: 0.0}, 'end_position_y': {0: 0.0},
         'contour_unique_id': {0: 0}})
    sg_segments = SGSegments()
    sg_segments.insert_segments(expected_signals_df)
    assert testing.assert_frame_equal(expected_signals_df, sg_segments.signals, check_dtype=False) is None


def test_SGSegments_insert_segments_invalid_data():
    error_msg = "Missing columns inside SGSegments dataframe!"
    with pytest.raises(AttributeError, match=error_msg):
        expected_signals_df = DataFrame.from_dict(
            {'timestamp': {0: 0}, 'unique_id': {0: 0}, 'start_position_x': {0: 0.0},
             'start_position_y': {0: 0.0}, 'end_position_x': {0: 0.0}, 'end_position_y': {0: 0.0},
             'contour_unique_id': {0: 0}})
        sg_segments = SGSegments()
        sg_segments.insert_segments(expected_signals_df)


def test_SGSegments_get_base_name():
    assert 'SGSegments' == SGSegments().get_base_name()
