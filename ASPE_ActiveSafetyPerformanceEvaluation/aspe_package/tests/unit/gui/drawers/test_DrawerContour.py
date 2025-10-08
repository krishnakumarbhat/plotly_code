from unittest.mock import MagicMock

from pandas import DataFrame, testing

from aspe.gui.drawers.DrawerContour import DrawerContour


def test_append_legend_part():
    legend = {"log_name": [], "drawer_name": [], "subdrawer_name": [], "color": [], "style": []}
    legend_df = DataFrame.from_dict(legend)
    legend_dict = {"log_name": ["UT.keg"], "drawer_name": "UT", "subdrawer_name": [], "color": [], "style": []}
    dummy = MagicMock()
    dummy.name = "Contour"
    dummy.color = "#AA0000"
    dummy.line_style = "-"
    dummy.is_enabled = True
    dict_expected = {"log_name": {0: "UT.keg"}, "drawer_name": {0: "UT"}, "subdrawer_name": {0: "Contour"},
                     "color": {0: "#AA0000"}, "style": {0: "-"}}
    out_expected = DataFrame.from_dict(dict_expected)
    out = DrawerContour.append_legend_part(dummy, legend_df, legend_dict)

    assert testing.assert_frame_equal(out, out_expected) is None

    dummy.is_enabled = False
    out_expected = legend_df
    out = DrawerContour.append_legend_part(dummy, legend_df, legend_dict)

    assert testing.assert_frame_equal(out, out_expected) is None


def test_set_data():
    dummy = MagicMock()
    in_dict = {"scan_index": {0: 0}, "timestamp": {0: 10}, "unique_id": {0: 1}, "num_contours": {0: 1},
               "f_valid": {0: True}, "drivability_class": {0: "UT"}, "contour_id": {0: 1}, "vertex_id": {0: 0},
               "num_vertices": {0: 1}, "contour_type": {0: "UT"}, "position_iso_x": {0: 1}, "position_iso_y": {0: 1.0},
               "position_variance_iso_x": {0: 1.0}, "position_variance_iso_y": {0: 1.0},
               "position_covariance_iso_xy": {0: 1.0}, "position_x": {0: 1.0}, "position_y": {0: 1.0},
               "position_variance_y": {0: 1.0}, "position_variance_x": {0: 1.0}, "position_covariance": {0: 1.0}}
    in_df = DataFrame.from_dict(in_dict)
    DrawerContour.set_data(dummy, in_df, "position_x", "position_y")

    assert dummy.x_data == {0: [1.]}
    assert dummy.y_data == {0: [1.]}
    assert dummy.is_visible == {0: [True]}
    assert dummy.df_indexes == {0: [0]}
