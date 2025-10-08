"""
Unittests for SGSegments builder.
"""
import pandas as pd
import pytest
from pandas.testing import assert_frame_equal

from aspe.evaluation.RadarSGEvaluation.Builders.SGSegments import build_segments
from aspe.evaluation.RadarSGEvaluation.DataStructures.SGSegments import SGSegments
from aspe.extractors.ENV.DataSets.ENVExtractedData import ENVExtractedData
from aspe.extractors.ENV.DataSets.ENVStationaryGeometries import ENVStationaryGeometries


@pytest.fixture()
def test_SGSegments_builder_setup():
    extracted = ENVExtractedData()
    sg_data = pd.DataFrame({'scan_index': [0, 0, 0, 2, 2, 4, 4, 4, 4],
                            'timestamp': [0.1, 0.1, 0.1, 0.2, 0.2, 0.4, 0.4, 0.4, 0.4],
                            'unique_id': [1, 1, 1, 5, 5, 7, 7, 7, 7],
                            'position_x': [0.0, 1.0, 1.0, -3.0, -4.0, 1.5, 1.0, 3.0, 4.0],
                            'position_y': [1.0, 3.0, 4.0, -2.0, -4.0, -1.5, -1.0, -3.0, -2.0],
                            'some_trash': 9 * [0.0003],
                            })
    extracted.stationary_geometries = ENVStationaryGeometries()
    extracted.stationary_geometries.signals = sg_data

    expected_output_df = pd.DataFrame({'scan_index': [0, 0, 2, 4, 4, 4],
                                       'timestamp': [0.1, 0.1, 0.2, 0.4, 0.4, 0.4],
                                       'unique_id': [0, 1, 2, 3, 4, 5],
                                       'contour_unique_id': [1, 1, 5, 7, 7, 7],
                                       'start_position_x': [0.0, 1.0, -3.0, 1.5, 1.0, 3.0],
                                       'start_position_y': [1.0, 3.0, -2.0, -1.5, -1.0, -3.0],
                                       'end_position_x': [1.0, 1.0, -4.0, 1.0, 3.0, 4.0],
                                       'end_position_y': [3.0, 4.0, -4.0, -1.0, -3.0, -2.0],
                                       })
    expected_output = SGSegments()
    expected_output.insert_segments(expected_output_df)

    return extracted, expected_output


def test_SGSegments_builder_1(test_SGSegments_builder_setup):
    extracted, expected_output = test_SGSegments_builder_setup
    output = build_segments(extracted)
    assert assert_frame_equal(output.signals, expected_output.signals) is None


@pytest.fixture()
def test_empty_setup():
    extracted = ENVExtractedData()
    extracted.stationary_geometries = None
    return extracted


def test_SGSegments_builder_empty_dataset(test_empty_setup):
    extracted = test_empty_setup
    with pytest.raises(ValueError,
                       match='Missing Stationary Geometries Data!'):
        build_segments(extracted)


@pytest.fixture()
def test_missing_column_setup():
    extracted = ENVExtractedData()
    sg_data = pd.DataFrame({'scan_index': [0, 0, 0, 2, 2, 4, 4, 4, 4],
                            'timestamp': [0.1, 0.1, 0.1, 0.2, 0.2, 0.4, 0.4, 0.4, 0.4],
                            'unique_id': [1, 1, 1, 5, 5, 7, 7, 7, 7],
                            'position_x': [0.0, 1.0, 1.0, -3.0, -4.0, 1.5, 1.0, 3.0, 4.0],
                            'missing_position_y': [0.0, 1.0, 1.0, -3.0, -4.0, 1.5, 1.0, 3.0, 4.0],
                            'some_trash': 9 * [0.0003],
                            })
    extracted.stationary_geometries = ENVStationaryGeometries()
    extracted.stationary_geometries.signals = sg_data
    return extracted


def test_SGSegments_builder_missing_column(test_missing_column_setup):
    extracted = test_missing_column_setup
    with pytest.raises(AttributeError,
                       match='Missing columns inside extracted dataframe!'):
        build_segments(extracted)
