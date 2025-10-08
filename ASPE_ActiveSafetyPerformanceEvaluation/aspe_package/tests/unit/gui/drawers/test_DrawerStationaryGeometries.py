from unittest.mock import MagicMock, patch

import pandas as pd
from pandas import DataFrame, testing

from aspe.gui.drawers.DrawerPoints import DrawerPoints
from aspe.gui.drawers.DrawerStationaryGeometries import DrawerStationaryGeometries, ellipse_data_prepare


def test_ellipse_data_prepare():
    in_dict = {"scan_index": {0: 0}, "timestamp": {0: 10}, "unique_id": {0: 1}, "num_contours": {0: 1},
               "f_valid": {0: True}, "drivability_class": {0: "UT"}, "contour_id": {0: 1}, "vertex_id": {0: 0},
               "num_vertices": {0: 1}, "contour_type": {0: "UT"}, "position_x": {0: 1.0}, "position_y": {0: 1.0},
               "position_variance_y": {0: 1.0}, "position_variance_x": {0: 1.0}, "position_covariance": {0: 1.0}}
    in_df = DataFrame.from_dict(in_dict)
    out_dict = {"scan_index": {0: 0}, "timestamp": {0: 10}, "unique_id": {0: 1}, "num_contours": {0: 1},
                "f_valid": {0: True}, "drivability_class": {0: "UT"}, "contour_id": {0: 1}, "vertex_id": {0: 0},
                "num_vertices": {0: 1}, "contour_type": {0: "UT"}, "position_x": {0: 1.0}, "position_y": {0: 1.0},
                "var_y": {0: 1.0}, "var_x": {0: 1.0}, "cov_xy": {0: 1.0}}
    out_expected = pd.DataFrame.from_dict(out_dict)
    dummy = MagicMock()
    out = ellipse_data_prepare(dummy, in_df)

    assert testing.assert_frame_equal(out, out_expected) is None


@patch("aspe.gui.drawers.DrawerStationaryGeometries.DrawerPoints")
def test_create_selection_drawer(mock_DrawerPoints):
    mock_DrawerPoints.return_value = MagicMock()
    dummy = MagicMock()
    plot_item = MagicMock()
    dummy_dict = {"scan_index": {0: 0}, "timestamp": {0: 10}, "unique_id": {0: 1}, "num_contours": {0: 1},
                  "f_valid": {0: True}, "drivability_class": {0: "UT"}, "contour_id": {0: 1}, "vertex_id": {0: 0},
                  "num_vertices": {0: 1}, "contour_type": {0: "UT"}, "position_x": {0: 1.0}, "position_y": {0: 1.0},
                  "position_variance_y": {0: 1.0}, "position_variance_x": {0: 1.0}, "position_covariance": {0: 1.0}}
    dummy.data_set.signals = DataFrame.from_dict(dummy_dict)
    dummy.data_set.raw_signals = DataFrame.from_dict(dummy_dict)
    out = DrawerStationaryGeometries.create_selection_drawer(dummy, plot_item)

    assert isinstance(out, MagicMock)


@patch("aspe.gui.drawers.DrawerStationaryGeometries.DrawerContour")
@patch("aspe.gui.drawers.DrawerStationaryGeometries.DrawerPoints")
@patch("aspe.gui.drawers.DrawerStationaryGeometries.DrawerLabels")
@patch("aspe.gui.drawers.DrawerStationaryGeometries.DrawerErrCovPosition")
def test_create_drawers(mock_DrawerErrCovPosition, mock_DrawerLabels, mock_DrawerPoints, mock_DrawerContour):
    mock_DrawerErrCovPosition.return_value = MagicMock()
    mock_DrawerLabels.return_value = MagicMock()
    mock_DrawerPoints.return_value = MagicMock()
    mock_DrawerContour.return_value = MagicMock()
    dummy = MagicMock()
    plot_item = MagicMock()
    out = DrawerStationaryGeometries.create_drawers(dummy, plot_item)
    num_drawer = 4

    assert len(out) == num_drawer
    assert isinstance(out[0], MagicMock)
    assert isinstance(out[1], MagicMock)
    assert isinstance(out[2], MagicMock)
    assert isinstance(out[3], MagicMock)


def test_select():
    dummy = MagicMock()
    clicked_drawer = MagicMock()
    dummy_dict = {"scan_index": {0: 0}, "timestamp": {0: 10}, "unique_id": {0: 1}, "num_contours": {0: 1},
                  "f_valid": {0: True}, "drivability_class": {0: "UT"}, "contour_id": {0: 1},
                  "num_vertices": {0: 1}, "contour_type": {0: "UT"}, "position_x": {0: 1.0}, "position_y": {0: 1.0},
                  "position_variance_y": {0: 1.0}, "position_variance_x": {0: 1.0}, "position_covariance": {0: 1.0},
                  "df_indexes": {0: 0}}
    dummy.data_set.signals = DataFrame.from_dict(dummy_dict)
    dummy.data_set.raw_signals = dummy.data_set.signals
    dummy.selected_unique_id = 0
    df_index = 0
    out_dict = {"scan_index": {0: 0}, "timestamp": {0: 10}, "unique_id": {0: 1}, "num_contours": {0: 1},
                "f_valid": {0: True}, "drivability_class": {0: "UT"}, "contour_id": {0: 1}, "num_vertices": {0: 1},
                "contour_type": {0: "UT"}, "position_x": {0: 1.0}, "position_y": {0: 1.0},
                "position_variance_y": {0: [1.0]}, "position_variance_x": {0: [1.0]}, "position_covariance": {0: [1.0]},
                "df_indexes": {0: 0}, "position_x_agg": {0: [1.0]}, "position_y_agg": {0: [1.0]},
                "scan_index_raw": {0: 0}, "timestamp_raw": {0: 10}, "unique_id_raw": {0: 1}, "num_contours_raw": {0: 1},
                "f_valid_raw": {0: True}, "drivability_class_raw": {0: "UT"}, "contour_id_raw": {0: 1},
                "num_vertices_raw": {0: 1}, "contour_type_raw": {0: "UT"}, "position_x_raw": {0: 1.0},
                "position_y_raw": {0: 1.0}, "position_variance_y_raw": {0: 1.0}, "position_variance_x_raw": {0: 1.0},
                "position_covariance_raw": {0: 1.0}, "df_indexes_raw": {0: 0}}

    out_expected = pd.DataFrame.from_dict(out_dict)
    DrawerStationaryGeometries.select(dummy, df_index, clicked_drawer)

    assert testing.assert_frame_equal(dummy.selected_data, out_expected) is None

    DrawerStationaryGeometries.select(dummy, df_index, clicked_drawer)

    assert dummy.deselect.call_count == 1

    dummy.data_set.raw_signals = None
    out_expected = dummy.data_set.signals
    plot_item = MagicMock()
    clicked_drawer = DrawerPoints(dummy, "SGSelection", plot_item, symbol="o", color="#ff0000", symbol_size=10)
    dummy.selected_unique_id = 0
    DrawerStationaryGeometries.select(dummy, df_index, clicked_drawer)

    assert testing.assert_frame_equal(dummy.selected_data, out_expected) is None


def test_on_click():
    clicked_drawer = MagicMock()
    dummy = MagicMock()
    click_event = MagicMock()
    click_event.pos.return_value = [1.0, 1.0]
    clicked_drawer.get_df_index_by_position.return_value = 0
    DrawerStationaryGeometries.on_click(dummy, clicked_drawer, click_event)

    assert dummy.select.call_count == 1

    dummy.selection_drawer = clicked_drawer
    DrawerStationaryGeometries.on_click(dummy, clicked_drawer, click_event)

    assert dummy.deselect.call_count == 1

    dummy.selection_drawer = None
    clicked_drawer.get_df_index_by_position.return_value = None
    DrawerStationaryGeometries.on_click(dummy, clicked_drawer, click_event)

    assert dummy.select.call_count == 1


def test_append_legend_part():
    out_expected_dict = {"color": {0: "UT"}, "drawer_name": {0: "UT"}, "log_name": {0: "UT"}, "style": {0: "-"},
                         "subdrawer_name": {0: "UT"}}
    out_expected_df = DataFrame.from_dict(out_expected_dict)
    dummy = MagicMock()
    dummy.is_enabled = True
    dummy.name = "UT"
    buff = MagicMock()
    buff.append_legend_part.return_value = out_expected_df
    dummy.drawers = [buff]
    legend = {"color": {}, "drawer_name": {}, "log_name": {}, "style": {}, "subdrawer_name": {}}
    legend_df = DataFrame.from_dict(legend)
    legend_dict = {"color": [], "drawer_name": [], "log_name": ["UT"], "style": [], "subdrawer_name": []}
    out = DrawerStationaryGeometries.append_legend_part(dummy, legend_df, legend_dict)

    assert testing.assert_frame_equal(out, out_expected_df) is None

    dummy.is_enabled = False
    out_expected_df = legend_df
    out = DrawerStationaryGeometries.append_legend_part(dummy, legend_df, legend_dict)

    assert testing.assert_frame_equal(out, out_expected_df) is None
