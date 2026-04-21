import numpy as np
import pandas as pd

from InteractivePlot.kpi_client.sil_radar_validation import match_points


def test_match_points_uses_4_fields_and_excludes_exponential_values():
    veh_df = pd.DataFrame(
        {
            "scan_index": [1, 1, 2],
            "ran": [10.0, 20.0, 30.0],
            "vel": [1.0, 1.0, 2.0],
            "azimuth": [2.0, 2.0, 3.0],
            "elevation": [3.0, 2.5e5, 4.0],
        }
    )
    resim_df = pd.DataFrame(
        {
            "scan_index": [1, 1, 2],
            "ran": [10.0, 20.0, 30.0],
            "vel": [1.0, 1.0, 2.0],
            "azimuth": [2.0, 2.0, 3.0],
            "elevation": [3.0, 3.0, 2.0e5],
        }
    )

    match = match_points(veh_df, resim_df, gate_threshold=0.01, metric="euclidean")

    np.testing.assert_array_equal(match.matched_veh_idx, np.array([0], dtype=np.int64))
    np.testing.assert_array_equal(match.matched_resim_idx, np.array([0], dtype=np.int64))
    np.testing.assert_array_equal(np.sort(match.orphan_veh_idx), np.array([1], dtype=np.int64))
    np.testing.assert_array_equal(np.sort(match.orphan_resim_idx), np.array([1, 2], dtype=np.int64))


def test_match_points_requires_elevation_for_matching():
    veh_df = pd.DataFrame(
        {
            "scan_index": [1, 2],
            "ran": [10.0, 20.0],
            "vel": [1.0, 2.0],
            "azimuth": [2.0, 3.0],
        }
    )
    resim_df = pd.DataFrame(
        {
            "scan_index": [1, 2],
            "ran": [10.0, 20.0],
            "vel": [1.0, 2.0],
            "azimuth": [2.0, 3.0],
        }
    )

    match = match_points(veh_df, resim_df, gate_threshold=0.01, metric="euclidean")

    assert match.matched_veh_idx.size == 0
    assert match.matched_resim_idx.size == 0
    np.testing.assert_array_equal(np.sort(match.orphan_veh_idx), np.array([0, 1], dtype=np.int64))
    np.testing.assert_array_equal(np.sort(match.orphan_resim_idx), np.array([0, 1], dtype=np.int64))


def test_match_points_reports_scan_level_totals():
    veh_df = pd.DataFrame(
        {
            "scan_index": [1, 1, 2],
            "ran": [10.0, 20.0, 40.0],
            "vel": [1.0, 2.0, 4.0],
            "azimuth": [3.0, 4.0, 5.0],
            "elevation": [0.5, 0.7, 0.9],
        }
    )
    resim_df = pd.DataFrame(
        {
            "scan_index": [1, 3],
            "ran": [10.0, 60.0],
            "vel": [1.0, 6.0],
            "azimuth": [3.0, 7.0],
            "elevation": [0.5, 1.1],
        }
    )

    match = match_points(veh_df, resim_df, gate_threshold=1.0, metric="euclidean")

    np.testing.assert_array_equal(match.common_scan_indices, np.array([1], dtype=np.int64))
    np.testing.assert_array_equal(match.input_only_scan_indices, np.array([2], dtype=np.int64))
    np.testing.assert_array_equal(match.output_only_scan_indices, np.array([3], dtype=np.int64))
    np.testing.assert_allclose(match.scan_match_percentages, np.array([50.0], dtype=np.float64))
    assert match.total_veh_points == 3
    assert match.total_resim_points == 2
