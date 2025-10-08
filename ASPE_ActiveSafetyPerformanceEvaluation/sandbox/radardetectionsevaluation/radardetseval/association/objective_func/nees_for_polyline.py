import numpy as np

from radardetseval.Iface.Detection import Detection
from radardetseval.Iface.PolyLineDS import PolyLineDS
from radardetseval.association.objective_func.nees_cart_vs_polar import nees_cart_vs_polar


def nees_value_for_single_vertex_id(vertex_id: float, polyline: PolyLineDS, detection: Detection, unique_id=0.0):
    polygon = polyline.signals[polyline.signals.unique_id == unique_id]
    vertex_ids = polygon['vertex_id'].values
    single_point_df = polyline.interpolate_polygon(np.array([vertex_id]), vertex_ids, polygon)
    point_series = single_point_df.iloc[0, :]
    ref_state = np.array([point_series.x, point_series.y, point_series.vx, point_series.vy])

    det_state = np.array([detection.det_range, detection.det_azimuth, detection.det_range_rate])
    det_cov = np.diag([detection.det_range_var, detection.det_azimuth_var, detection.det_range_rate_var])

    nees = nees_cart_vs_polar(ref_state, point_series.cov_matrix, det_state, det_cov)
    return nees


def nees_value_for_vertex_ids(vertex_ids: np.array, polyline: PolyLineDS, detection: Detection, unique_id=0.0):
    nees = np.zeros_like(vertex_ids)

    for i, vertex_id in enumerate(vertex_ids):
        nees[i] = nees_value_for_single_vertex_id(vertex_id, polyline, detection, unique_id)

    return nees

