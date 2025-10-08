import pytest
from pandas import DataFrame, testing
from aspe.evaluation.RadarSGEvaluation.DataStructures.SGSamples import SGSamples


def test_SGSamples_init():
    expected_signals_df = DataFrame.from_dict(
        {'scan_index': {}, 'timestamp': {}, 'unique_id': {}, 'segment_unique_id': {},
         'sample_position_x': {}, 'sample_position_y': {}, 'projection_sample_position_x': {},
         'projection_sample_position_y': {}, 'distance': {}, 'deviation_x': {}, 'deviation_y': {},
         'classification': {}, 'multiple_segmentation': {}})
    sg_samples = SGSamples()
    assert testing.assert_frame_equal(expected_signals_df, sg_samples.signals, check_dtype=False) is None


def test_SGSamples_insert_samples_valid_data():
    expected_signals_df = DataFrame.from_dict(
        {'scan_index': {0: 0}, 'timestamp': {0: 0}, 'unique_id': {0: 1}, 'segment_unique_id': {0: 1},
         'sample_position_x': {0: 1.0}, 'sample_position_y': {0: 1.0}, 'projection_sample_position_x': {0: 1.0},
         'projection_sample_position_y': {0: 1.0}, 'distance': {0: 1.0}, 'deviation_x': {0: 1.0},
         'deviation_y': {0: 1.0}, 'classification': {0: True}, 'multiple_segmentation': {0: True}})
    sg_samples = SGSamples()
    sg_samples.insert_samples(expected_signals_df)
    assert testing.assert_frame_equal(expected_signals_df, sg_samples.signals) is None


def test_SGSamples_insert_samples_invalid_data():
    error_msg = "Missing columns inside SGSamples dataframe!"
    with pytest.raises(AttributeError, match=error_msg):
        expected_signals_df = DataFrame.from_dict(
            {'missing_or_invalid_index': {0: 0}, 'timestamp': {0: 0}, 'unique_id': {0: 1}, 'segment_unique_id': {0: 1},
             'sample_position_x': {0: 1.0}, 'sample_position_y': {0: 1.0}, 'projection_sample_position_x': {0: 1.0},
             'projection_sample_position_y': {0: 1.0}, 'distance': {0: 1.0}, 'deviation_x': {0: 1.0},
             'deviation_y': {0: 1.0}, 'classification': {0: True}, 'multiple_segmentation': {0: True}})
        sg_samples = SGSamples()
        sg_samples.insert_samples(expected_signals_df)


def test_SGSamples_get_base_name():
    assert 'SGSamples' == SGSamples().get_base_name()
