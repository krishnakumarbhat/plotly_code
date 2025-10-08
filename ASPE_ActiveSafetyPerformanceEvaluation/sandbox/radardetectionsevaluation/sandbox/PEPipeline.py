import warnings
from copy import deepcopy
from dataclasses import dataclass
from typing import List, Dict
import numpy as np
import pandas as pd
from AptivDataExtractors.utilities.MathFunctions import vcs2scs, calc_position_in_bounding_box, \
    calc_velocity_in_position
from radardetseval.utilities.bb_support import is_point_in_bb
from scipy.stats import chi2

from radardetseval.stats.nees import nees_value

from radardetseval.unc_prop.pos_vel_cart_to_polar import unc_prop_pos_rel_vel_cart_to_polar
from radardetseval.conversions.object_conversion import otg_to_rel_vel, otg_to_rel_vel_cov
from scipy import optimize
from radardetseval.configs.default_radar import DefaultRadar
from radardetseval.configs.defaut_reference import DefaultReference
from radardetseval.data_provider.nerest_timestamp_synch import RtF360NearestTimestampSynchronization
from radardetseval.utilities.cs_transformation import vcs2scs_object, get_reference_sensor_data
from radardetseval.utilities.data_filters import filter_out_single_radar, slice_df_based_on_other


def pre_process_data(estimated_data, reference_data, sensor_id=1):

    estimated_data_single_sensor = filter_out_single_radar(estimated_data, sensor_id)

    synchronizer = RtF360NearestTimestampSynchronization(max_ts_diff=0.026)
    est_synch, ref_synch = synchronizer.synch(estimated_data_single_sensor, reference_data,
                                              estimated_data_single_sensor.sensors, reference_data.host)

    # TODO: should be fixed on extractor level
    ref_synch.host.signals = ref_synch.host.signals.convert_dtypes(convert_integer=False)
    ref_synch.objects.signals = ref_synch.objects.signals.convert_dtypes(convert_integer=False)

    ref_host_signal = ref_synch.host.signals
    mounting_loc = est_synch.sensors.per_sensor.iloc[0][:]

    # Reference sensor
    ref_synch.sensors = get_reference_sensor_data(est_synch.sensors, ref_synch.host)

    scs_x, scs_y = vcs2scs(est_synch.detections.signals['position_x'], est_synch.detections.signals['position_y'],
                           mounting_loc.position_x, mounting_loc.position_y, mounting_loc.boresight_az_angle)

    est_synch.detections.signals['position_x'] = scs_x
    est_synch.detections.signals['position_y'] = scs_y

    ref_synch.objects = vcs2scs_object(ref_synch.objects, mounting_loc)

    object_fov_info = calc_object_fov_info(ref_synch.objects.signals, ref_synch.sensors)
    ref_synch.objects.signals = pd.concat([ref_synch.objects.signals, object_fov_info], axis=1)

    return est_synch, ref_synch


def calc_object_fov_info(obj_signals, sensors):
    sensor_ext_signals = sensors.signals.merge(sensors.per_look, on='look_id')
    signals_si = pd.DataFrame(obj_signals['scan_index'])
    sensor_ext_signals_synch = signals_si.merge(sensor_ext_signals, on='scan_index')  # Synchronized with Objects

    ref_points = np.array([[0.0, 0.0],
                           [0.5, 0.0],
                           [1.0, 0.0],
                           [1.0, 0.5],
                           [1.0, 1.0],
                           [0.5, 1.0],
                           [0.0, 1.0],
                           [0.0, 0.5]])

    range_list = []
    azimuth_list = []
    range_rate_list = []
    for single_ref_point in ref_points:
        det_state = calc_corresponding_detection_state_on_ref_point(obj_signals,
                                                                    sensor_ext_signals_synch,
                                                                    single_ref_point)
        range_list.append(det_state.range)
        azimuth_list.append(det_state.azimuth)
        range_rate_list.append(det_state.range_rate)

    range_array = np.array(range_list)
    azimuth_array = np.array(azimuth_list)
    range_rate_array = np.array(range_rate_list)

    valid_range = range_array <= sensor_ext_signals_synch.max_range.values
    valid_azimuth = np.logical_and(azimuth_array <= sensor_ext_signals_synch.max_azimuth.values,
                                   azimuth_array >= sensor_ext_signals_synch.min_azimuth.values)

    f_in_fov = np.logical_and(valid_range, valid_azimuth)
    f_not_in_fov = np.logical_not(f_in_fov)
    range_array[f_not_in_fov] = np.nan
    azimuth_array[f_not_in_fov] = np.nan
    range_rate_array[f_not_in_fov] = np.nan

    fov_info = pd.DataFrame()
    with warnings.catch_warnings():
        # Caution - in multi-thread operations this will be undefined behavior
        warnings.filterwarnings('ignore', r'All-NaN slice encountered')
        fov_info['f_in_fov_object'] = f_in_fov.any(axis=0)
        fov_info['min_range'] = np.nanmin(range_array, axis=0)
        fov_info['max_range'] = np.nanmax(range_array, axis=0)
        fov_info['range_spread'] = fov_info['max_range'] - fov_info['min_range']
        fov_info['min_azimuth'] = np.nanmin(azimuth_array, axis=0)
        fov_info['max_azimuth'] = np.nanmax(azimuth_array, axis=0)
        fov_info['azimuth_spread'] = fov_info['max_azimuth'] - fov_info['min_azimuth']
        fov_info['min_range_rate'] = np.nanmin(range_rate_array, axis=0)
        fov_info['max_range_rate'] = np.nanmax(range_rate_array, axis=0)
        fov_info['range_rate_spread'] = fov_info['max_range_rate'] - fov_info['min_range_rate']
    return fov_info


def gating(det_signals, ref_signals, ref_sensor_signals):
    rel_det_signals = det_signals[['scan_index', 'unique_id', 'position_x', 'position_y']]
    rel_ref_signals = ref_signals[['scan_index', 'unique_id', 'center_x', 'center_y',
                                   'bounding_box_dimensions_x', 'bounding_box_dimensions_y']]
    threshold_gain = 1.0
    threshold_offset = 2.0
    pairs = rel_det_signals.merge(rel_ref_signals, on='scan_index', suffixes=('_det', '_obj'))
    pairs['gating_distance'] = np.hypot(
        pairs['position_x'].values.astype(np.float32) - pairs['center_x'].values.astype(np.float32),
        pairs['position_y'].values.astype(np.float32) - pairs['center_y'].values.astype(np.float32))
    pairs['bounding_box_diagonal'] = np.hypot(pairs['bounding_box_dimensions_x'].values.astype(np.float32),
                                              pairs['bounding_box_dimensions_y'].values.astype(np.float32))
    pairs['gating_threshold'] = pairs['bounding_box_diagonal'] * threshold_gain + threshold_offset
    pairs['f_is_associated'] = pairs['gating_distance'] < pairs['gating_threshold']
    valid_pairs = pairs[pairs['f_is_associated']].reset_index(drop=True)
    valid_pairs = valid_pairs[['scan_index', 'unique_id_obj', 'unique_id_det', 'gating_distance']]

    est_det_synch_pairs_signals = slice_df_based_on_other(det_signals, valid_pairs, ['scan_index', 'unique_id'],
                                                          ['scan_index', 'unique_id_det'])

    ref_obj_synch_pairs_signals = slice_df_based_on_other(ref_signals, valid_pairs, ['scan_index', 'unique_id'],
                                                          ['scan_index', 'unique_id_obj'])

    ref_sensor_synch_pairs_signals = slice_df_based_on_other(ref_sensor_signals, valid_pairs, ['scan_index'],
                                                             ['scan_index'])

    return valid_pairs, est_det_synch_pairs_signals, ref_obj_synch_pairs_signals, ref_sensor_synch_pairs_signals


def nees_3d_dets_features(valid_pairs_est_det, valid_pairs_ref_obj, valid_pairs_sensor):
    nees_list = []
    for slot_idx, single_det in valid_pairs_est_det.iterrows():
        single_obj = valid_pairs_ref_obj.iloc[slot_idx, :]
        single_sensor_data = valid_pairs_sensor.iloc[slot_idx, :]

        nees_list.append(find_nearest_det_on_object_by_nees(single_obj, single_sensor_data, single_det))
    nees = pd.DataFrame(nees_list)

    nees['deviation_azimuth_deg'] = np.rad2deg(nees['deviation_azimuth'])
    f_inside_x = np.logical_and(nees.ref_point_x > 0.001, nees.ref_point_x < 0.999)
    f_inside_y = np.logical_and(nees.ref_point_y > 0.001, nees.ref_point_y < 0.999)
    f_corner = np.logical_not(np.logical_or(f_inside_x, f_inside_y))
    f_side = np.logical_xor(np.logical_not(f_inside_x), np.logical_not(f_inside_y))
    nees['dof'] = 1.0  # Inside bounding box
    nees.loc[f_side, 'dof'] = 2.0
    nees.loc[f_corner, 'dof'] = 3.0
    nees['p_value'] = 1.0 - chi2.cdf(nees['nees_3d'], nees['dof'])

    return nees


def is_det_in_solid_bb(valid_pairs_est_det, valid_pairs_ref_obj):

    f_in_solid_bb = is_point_in_bb(valid_pairs_est_det['position_x'], valid_pairs_est_det['position_y'],
                                   valid_pairs_ref_obj['center_x'], valid_pairs_ref_obj['center_y'],
                                   valid_pairs_ref_obj['bounding_box_dimensions_x'],
                                   valid_pairs_ref_obj['bounding_box_dimensions_y'],
                                   valid_pairs_ref_obj['bounding_box_orientation'])
    return pd.DataFrame([f_in_solid_bb], columns=['f_in_solid_bb'])


def is_det_in_ext_bb(valid_pairs_est_det, valid_pairs_ref_obj, length_extension=2.0, width_extension=1.0):

    f_in_ext_bb = is_point_in_bb(valid_pairs_est_det['position_x'], valid_pairs_est_det['position_y'],
                                 valid_pairs_ref_obj['center_x'], valid_pairs_ref_obj['center_y'],
                                 valid_pairs_ref_obj['bounding_box_dimensions_x'] + length_extension,
                                 valid_pairs_ref_obj['bounding_box_dimensions_y'] + width_extension,
                                 valid_pairs_ref_obj['bounding_box_orientation'])
    return pd.DataFrame([f_in_ext_bb], columns=['f_in_ext_bb'])


def association(nees_features: pd.DataFrame, p_value_thr):

    f_associated = nees_features['p_value'].gt(p_value_thr)
    return f_associated.to_frame('f_associated')


def check_error_patterns(valid_pairs):
    f_azimuth_error = np.logical_not(valid_pairs['f_associated'])


def find_nearest_det_on_object_by_nees(radar_object, sensor_synch, detection):
    """
    Faster version of finding ref point on object for which minimum NEES metric is observed for given detection.
    Constant reference covariance matrix is assumed.
    :param radar_object:
    :param sensor_synch:
    :param detection:
    :return:
    """
    ref_cov = get_object_pos_rel_vel_cov(radar_object, sensor_synch)
    det_state = detection[['range', 'azimuth', 'range_rate']]
    det_cov = np.diag([detection['range_variance'], detection['azimuth_variance'], detection['range_rate_variance']])

    initial_ref_point = np.array([0.5, 0.5])
    bounds = ((0.0, 1.0), (0.0, 1.0))
    opt_out = optimize.minimize(nees_objective_function,
                                x0=initial_ref_point,
                                args=(radar_object, sensor_synch, ref_cov, det_state, det_cov),
                                bounds=bounds)
    ref_state = calc_object_rel_state_at_ref_point(radar_object, sensor_synch, opt_out.x,)
    deviation = deviation_cart_vs_polar(ref_state, det_state)
    nees = nees_components(ref_state, ref_cov, det_state, det_cov)
    output = get_optimization_outputs(opt_out, ref_state, deviation, nees)
    return output


def get_object_pos_rel_vel_cov(single_obj, sensor_series):
    cov = np.zeros([4, 4])
    velocity_rel_variance_x, velocity_rel_variance_y, velocity_rel_covariance, = \
        otg_to_rel_vel_cov(single_obj.velocity_otg_variance_x, single_obj.velocity_otg_variance_y,
                           single_obj.velocity_otg_covariance, sensor_series.velocity_otg_variance_x,
                           sensor_series.velocity_otg_variance_y, sensor_series.velocity_otg_covariance,
                           sensor_series.yaw_rate_variance, single_obj.position_x, single_obj.position_y)
    cov[0, 0] = single_obj.position_variance_x
    cov[1, 1] = single_obj.position_variance_y
    cov[1, 0] = single_obj.position_covariance
    cov[0, 1] = single_obj.position_covariance
    cov[2, 2] = velocity_rel_variance_x
    cov[3, 3] = velocity_rel_variance_y
    cov[2, 3] = velocity_rel_covariance
    cov[3, 2] = velocity_rel_covariance
    return cov


def get_optimization_outputs(opt_out, ref_state, deviation, nees):
    opt_out_series = pd.Series({
        'ref_point_x': opt_out.x[0],
        'ref_point_y': opt_out.x[1],
        'opt_success': opt_out.success,
        'opt_nit': opt_out.nit,
    })
    deviation_series = pd.Series({
        'deviation_range': deviation[0],
        'deviation_azimuth': deviation[1],
        'deviation_range_rate': deviation[2]
    })
    output = pd.concat([opt_out_series, ref_state, deviation_series, nees])
    return output


def nees_objective_function(ref_point, radar_object, sensor_synch, ref_cov, det_state, det_cov):
    ref_state = calc_object_rel_state_at_ref_point(radar_object, sensor_synch, ref_point)
    nees = nees_cart_vs_polar(ref_state, ref_cov, det_state, det_cov)
    return nees


def calc_object_rel_state_at_ref_point(radar_object, sensor_synch, ref_point):
    position_x, position_y = calc_position_in_bounding_box(radar_object.position_x, radar_object.position_y,
                                                           radar_object.bounding_box_dimensions_x,
                                                           radar_object.bounding_box_dimensions_y,
                                                           radar_object.bounding_box_orientation,
                                                           radar_object.bounding_box_refpoint_long_offset_ratio,
                                                           radar_object.bounding_box_refpoint_lat_offset_ratio,
                                                           ref_point[0], ref_point[1])

    # Target yaw rate influence
    velocity_otg_x, velocity_otg_y = calc_velocity_in_position(radar_object.position_x, radar_object.position_y,
                                                               radar_object.velocity_otg_x, radar_object.velocity_otg_y,
                                                               radar_object.yaw_rate, position_x, position_y)

    # Relative velocity calculation
    velocity_rel_x, velocity_rel_y = otg_to_rel_vel(velocity_otg_x, velocity_otg_y,
                                                    sensor_synch.velocity_otg_x, sensor_synch.velocity_otg_y,
                                                    sensor_synch.yaw_rate,
                                                    position_x, position_y)

    data = {'position_x': position_x,
            'position_y': position_y,
            'velocity_rel_x': velocity_rel_x,
            'velocity_rel_y': velocity_rel_y}

    if isinstance(radar_object, pd.DataFrame):
        rel_state = pd.DataFrame(data)
    else:
        rel_state = pd.Series(data)

    return rel_state


def calc_corresponding_detection_state_on_ref_point(radar_object, sensor_synch, ref_point):
    rel_state = calc_object_rel_state_at_ref_point(radar_object, sensor_synch, ref_point)
    polar_state = cart_rel_state_to_detection_state(rel_state)

    return polar_state


def nees_cart_vs_polar(cart_state, cart_cov, polar_state, polar_cov):

    # TODO: handle 2d case
    ref_cov_polar = unc_prop_pos_rel_vel_cart_to_polar(cart_state.position_x,
                                                       cart_state.position_y,
                                                       cart_state.velocity_rel_x,
                                                       cart_state.velocity_rel_y,
                                                       cart_cov)

    cov_sum = ref_cov_polar + polar_cov

    deviation = deviation_cart_vs_polar(cart_state, polar_state)

    nees = nees_value(deviation, cov_sum)
    return nees


def nees_components(cart_state, cart_cov, polar_state, polar_cov):

    ref_cov_polar = unc_prop_pos_rel_vel_cart_to_polar(cart_state.position_x,
                                                       cart_state.position_y,
                                                       cart_state.velocity_rel_x,
                                                       cart_state.velocity_rel_y,
                                                       cart_cov)

    cov_sum = ref_cov_polar + polar_cov

    deviation = deviation_cart_vs_polar(cart_state, polar_state)
    nees_azimuth_range_rate = nees_value(deviation[1:], cov_sum[1:, 1:])
    nees_range = deviation[0] ** 2 / cov_sum[0, 0]
    nees_azimuth = deviation[1] ** 2 / cov_sum[1, 1]
    nees_range_rate = deviation[2] ** 2 / cov_sum[2, 2]

    nees_3d = nees_value(deviation, cov_sum)
    nees = pd.Series({
        'nees_3d': nees_3d,
        'nees_range': nees_range,
        'nees_azimuth': nees_azimuth,
        'nees_range_rate': nees_range_rate,
        'nees_azimuth_range_rate': nees_azimuth_range_rate
    })

    return nees


def deviation_cart_vs_polar(cart_state, detection_state):
    ref_polar_state = cart_rel_state_to_detection_state(cart_state)
    deviation = np.array([ref_polar_state.range - detection_state.range,
                          ref_polar_state.azimuth - detection_state.azimuth,
                          ref_polar_state.range_rate - detection_state.range_rate])
    return deviation


def cart_rel_state_to_detection_state(cart_state):
    det_range = np.hypot(cart_state.position_x, cart_state.position_y)
    azimuth = np.arctan2(cart_state.position_y, cart_state.position_x)
    sin_az = np.sin(azimuth)
    cos_az = np.cos(azimuth)
    range_rate = cos_az * cart_state.velocity_rel_x + sin_az * cart_state.velocity_rel_y
    cross_radial_velocity = -sin_az * cart_state.velocity_rel_x + cos_az * cart_state.velocity_rel_y

    data = {'range': det_range,
            'azimuth': azimuth,
            'range_rate': range_rate,
            'cross_radial_velocity': cross_radial_velocity}
    if isinstance(cart_state, pd.DataFrame):
        polar_state = pd.DataFrame(data)
    else:
        polar_state = pd.Series(data)

    return polar_state


def evaluate_single_sensor(est_synch, ref_synch, association_thr=0.001):
    valid_pairs, valid_pairs_est_det, valid_pairs_ref_obj, valid_pairs_ref_sensor = \
        gating(est_synch.detections.signals, ref_synch.objects.signals, ref_synch.sensors.signals)

    nees = nees_3d_dets_features(valid_pairs_est_det, valid_pairs_ref_obj, valid_pairs_ref_sensor)

    assoc_df = association(nees, association_thr)

    pairs_df = pd.concat([valid_pairs, nees, assoc_df], axis=1)
    output = {
        'pairs_df': pairs_df,
        'pairs_dets': valid_pairs_est_det,
        'pairs_ref': valid_pairs_ref_obj,
    }
    return output


def evaluate(estimated_data, reference_data):
    output_dict = dict()
    sensors_ids = estimated_data.sensors.per_sensor.sensor_id
    for single_sensor_id in sensors_ids:
        print(f'Evaluation of sensor: {single_sensor_id}')
        est_synch, ref_synch = pre_process_data(estimated_data, reference_data, single_sensor_id)
        output_dict[single_sensor_id] = evaluate_single_sensor(est_synch, ref_synch)

    print(f'Accumulating different sensor data')
    flatten_dict_of_df = dict_based_concat(output_dict, 'sensor_id_key')
    print('Single log evaluation done')
    return flatten_dict_of_df


def dict_based_concat(dict_of_dict_of_df: Dict, first_key_id: str):
    flatten_dict_of_df = dict()
    dict_of_df = next(iter(dict_of_dict_of_df.values()))

    # Initialize output by list
    for key in dict_of_df.keys():
        flatten_dict_of_df[key] = list()

    # Assign key from first dict keys and accumulate data-frames into list
    for first_dict_key, single_output in dict_of_dict_of_df.items():
        for output_key, df in single_output.items():
            df[first_key_id] = first_dict_key
            flatten_dict_of_df[output_key].append(df)

    # Execute flattening by concat
    for key, df_list in flatten_dict_of_df.items():
        flatten_dict_of_df[key] = pd.concat(df_list, ignore_index=True)

    # Add as reference data frame with first dict keys
    flatten_dict_of_df[first_key_id] = pd.DataFrame(dict_of_dict_of_df.keys(), columns=[first_key_id])

    return flatten_dict_of_df


def evaluate_multi_log(list_of_log: List, data_provider):

    output_dict = dict()
    for idx, log_path in enumerate(list_of_log):
        print(f'Evaluation of log: {log_path}')
        estimated_data, reference_data = data_provider.get_single_log_data(log_path)
        output_dict[idx] = evaluate(estimated_data, reference_data)
        output_dict[idx]['logs'] = pd.DataFrame([log_path], columns=['log_path'])

    print(f'Accumulating different log data')
    flatten_dict_of_df = dict_based_concat(output_dict, 'log_id_key')
    print('Multi-log evaluation done')
    return flatten_dict_of_df





