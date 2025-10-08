import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from tqdm import tqdm

from aspe.extractors import ExtractedData
from aspe.extractors.API.mudp import extract_f360_from_mudp
from aspe.extractors.Interfaces.IObjects import IObjects
from aspe.utilities.nexus50k_events_finder.detections_analysis.size_error_analysis.calculate_features import (
    calculate_aspect_angle,
)
from aspe.utilities.SupportingFunctions import calc_dets_azimuth_vcs, save_to_pkl
from aspe.utilities.velocity_profile import calculate_velocity_profile


def predict_past_dets_position_for_scan(scan: int, obj_dets: pd.DataFrame, gt: pd.DataFrame, prev_scans: int,
                                        next_scans: int):
    """
    Calculates predicted detection position in given scan index range. Position is calculated based on the detection
    range rate value and orientation.
    @param scan: current scan index
    @param obj_dets: object detections
    @param gt: ground truth dataframe
    @param prev_scans: previous scans that should be analyzed
    @param next_scans: next scans that should be analyzed
    @return: enhanced object detections in current scan index
    """
    DT = 0.05
    scan_range = list(range(scan - prev_scans, scan + next_scans))
    dets_scan_range = obj_dets.loc[obj_dets.scan_index.isin(scan_range), :]
    past_scans = np.unique(dets_scan_range.scan_index)[:-1]

    for past_scan in past_scans:
        sub_gt = gt.loc[gt.scan_index.isin(range(past_scan, scan)), :]
        vel_x_sum = sub_gt.velocity_rel_x.sum()
        vel_y_sum = sub_gt.velocity_rel_y.sum()

        dets_scan_range.loc[dets_scan_range.scan_index == past_scan, 'vel_x_sum'] = vel_x_sum
        dets_scan_range.loc[dets_scan_range.scan_index == past_scan, 'vel_y_sum'] = vel_y_sum

    dets_scan_range.loc[dets_scan_range.scan_index == scan, 'vel_x_sum'] = 0.0
    dets_scan_range.loc[dets_scan_range.scan_index == scan, 'vel_y_sum'] = 0.0
    dets_scan_range.loc[:, 'predicted_position_x'] = dets_scan_range.position_x + dets_scan_range.vel_x_sum * DT
    dets_scan_range.loc[:, 'predicted_position_y'] = dets_scan_range.position_y + dets_scan_range.vel_y_sum * DT
    return dets_scan_range


def estimate_pseudo_size(dets_with_prediction: pd.DataFrame, gt_scan: pd.DataFrame):
    """
    Estimates bounding box pseudo size. Length and width of bbox are calculated based on prediction enhanced
    detections. At first function performs rotation by orientation angle, then it calculates quantiles for length and
    width values. Based on these values we set bbox size in current scan index. Center is set as mean value of
    calculated quantiles. Values are rotated back to their previous position in function place_bbox, by function
    rotate_center_position.
    @param dets_with_prediction: prediction enhanced detections
    @param gt_scan: ground truth in current scan index
    @return: bbox length, width, center x and center y
    """
    orientation = gt_scan.bounding_box_orientation.iloc[0]
    pos_mat = dets_with_prediction.loc[:, ['predicted_position_x', 'predicted_position_y']].to_numpy().T
    rot_mat = np.array([
        [np.cos(orientation), -np.sin(orientation)],
        [np.sin(orientation),  np.cos(orientation)],
    ])
    pos_rot = rot_mat @ pos_mat
    len1 = np.quantile(pos_rot[0, :], 0.95)
    len2 = np.quantile(pos_rot[0, :], 0.05)
    wid1 = np.quantile(pos_rot[1, :], 0.95)
    wid2 = np.quantile(pos_rot[1, :], 0.05)
    len = len1 - len2
    wid = wid1 - wid2
    center_x = (len1 + len2) / 2
    center_y = (wid1 + wid2) / 2

    return len, wid, center_x, center_y


def place_bbox(gt_signals, dets_signals, stat_u_ids=[], prev_scans=4, next_scans=1, range_value=30):
    """
    Place bbox for moving objects or with given unique id.
    @param gt_signals: ground truth data
    @param dets_signals: detections data
    @param stat_u_ids: unique ids of static objects which should be considered
    @param prev_scans: previous scans that should be analyzed
    @param next_scans: next scans that should be analyzed
    @param range_value: range value in which objects should be analyzed
    """
    pd.options.mode.chained_assignment = None  # suppresses pandas warning, default='warn',

    dets = dets_signals.join(gt_signals.set_index(['scan_index', 'f360_obj_id']).loc[:, ['velocity_otg_x', 'velocity_otg_y', 'unique_id']],
                             on=['scan_index', 'assigned_obj_id'],
                             rsuffix='_gt')
    print('Placing bounding boxes.')
    for f360_obj_unique_id, obj_dets in tqdm(dets.groupby(by='unique_id_gt')):
        gt = gt_signals.loc[gt_signals.unique_id == f360_obj_unique_id, :]
        if gt.loc[:, 'f_moving'].any() or f360_obj_unique_id in stat_u_ids:
            for scan in np.unique(obj_dets.scan_index):
                dets_with_prediction = predict_past_dets_position_for_scan(scan, obj_dets, gt, prev_scans=prev_scans,
                                                                           next_scans=next_scans)
                gt_scan = gt.loc[gt.scan_index == scan, :]
                len, wid, center_x, center_y = estimate_pseudo_size(dets_with_prediction, gt_scan)
                gt_signals.loc[gt_scan.index, 'bounding_box_dimensions_x'] = len
                gt_signals.loc[gt_scan.index, 'bounding_box_dimensions_y'] = wid
                gt_signals.loc[gt_scan.index, 'center_x'] = center_x
                gt_signals.loc[gt_scan.index, 'center_y'] = center_y

                calculate_aspect_angle(gt_signals)

            rotate_center_position(gt_signals, f360_obj_unique_id)
            calc_aspect_angle_based_size(gt_signals, f360_obj_unique_id, range_value=range_value)


def calc_velocity_profile_for_objects(extracted: ExtractedData, stat_u_ids=[], prev_scans=1, next_scans=1):
    """
    Calculates velocity profile speed and orientation for moving objects. If objects are stationary tracker values of
    given signals are written. Function fits detections to velocity profile curve. It also uses data from previous and
    next scans to improve accuracy.
    @param extracted: extracted data
    @param stat_u_ids: stationary objects unique ids
    @param prev_scans: previous scans which should be analyzed
    @param next_scans: next scans which should be analyzed
    """
    calc_dets_azimuth_vcs(extracted)
    dets = extracted.detections.signals.query('assigned_obj_id > 0').drop(columns=['unique_id']).drop_duplicates()
    objects = extracted.internal_objects.signals
    objects.set_index(['scan_index', 'id'], inplace=True)

    dets_grouped = dets.groupby(by='assigned_obj_id')
    print('Calculating velocity profile for objects.')
    for obj_id, same_obj_dets in tqdm(dets_grouped):
        scans = np.unique(same_obj_dets.scan_index)

        for scan_index in scans:
            prev_scan_index, next_scan_index = scan_index - prev_scans, scan_index + next_scans
            scans_to_take = [scan_index]
            if prev_scan_index in scans:
                scans_to_take.append(prev_scan_index)

            if next_scan_index in scans:
                scans_to_take.append(next_scan_index)

            obj_dets = same_obj_dets.loc[same_obj_dets.scan_index.isin(scans_to_take)]
            # filter detections not selected by trk
            obj_dets = obj_dets.loc[extracted.detections.raw_signals.iloc[obj_dets.index].loc[:, 'f_rr_inlier'].astype(dtype=bool)]
            if len(obj_dets) < 2:
                continue

            if not (scan_index, obj_id) in objects.index:
                continue

            if not objects.loc[(scan_index, obj_id), 'f_moving']:
                if objects.loc[(scan_index, obj_id), 'unique_id'] in stat_u_ids:
                    objects.loc[(scan_index, obj_id), 'velocity_profile_speed'] = \
                        objects.loc[(scan_index, obj_id), 'speed']
                    objects.loc[(scan_index, obj_id), 'velocity_profile_orientation'] = \
                        objects.loc[(scan_index, obj_id), 'bounding_box_orientation']
                    objects.loc[(scan_index, obj_id), 'mean_dets_position_x'] = obj_dets.position_x.mean()
                    objects.loc[(scan_index, obj_id), 'mean_dets_position_y'] = obj_dets.position_y.mean()
                    objects.loc[(scan_index, obj_id), 'velocity_profile_speed_var'] = np.nan
                    objects.loc[(scan_index, obj_id), 'velocity_profile_orientation_var'] = np.nan
                continue

            try:
                speed, orientation, covariance = calculate_velocity_profile(obj_dets)
            except RuntimeError as e:
                print(e)

            objects.loc[(scan_index, obj_id), 'velocity_profile_speed'] = speed
            objects.loc[(scan_index, obj_id), 'velocity_profile_orientation'] = orientation
            objects.loc[(scan_index, obj_id), 'mean_dets_position_x'] = obj_dets.position_x.mean()
            objects.loc[(scan_index, obj_id), 'mean_dets_position_y'] = obj_dets.position_y.mean()
            objects.loc[(scan_index, obj_id), 'velocity_profile_speed_var'] = covariance[0, 0]
            objects.loc[(scan_index, obj_id), 'velocity_profile_orientation_var'] = covariance[1, 1]

    objects.reset_index(inplace=True)


def create_ground_truth(extracted, stat_u_ids=[], prev_scans=4, next_scans=1, range_value=30):
    """
    Main function for ground truth creation. Only moving objects and stationary with given u ids are considered.
    Range value allows filtering poor bbox size estimation when objects are away.
    @param extracted: extracted data
    @param stat_u_ids: stationary objects ids
    @param prev_scans: previous scans which should be analyzed
    @param next_scans: next scans which should be analyzed
    @param range_value: range value in which objects should be analyzed
    @return: dataframe with ground truth data
    """
    objs = extracted.internal_objects.signals

    cols_to_copy = ['scan_index', 'timestamp', 'unique_id', 'id',
                    'mean_dets_position_x', 'mean_dets_position_y',
                    'velocity_profile_speed', 'velocity_profile_orientation',
                    'velocity_profile_speed_var', 'velocity_profile_orientation_var', 'n_dets',
                    'bounding_box_refpoint_long_offset_ratio', 'bounding_box_refpoint_lat_offset_ratio',
                    'f_moving']
    gt_signals = objs.loc[:, cols_to_copy]
    gt_signals.rename(
        columns={
            'mean_dets_position_x': 'center_x',
            'mean_dets_position_y': 'center_y',
            'id': 'f360_obj_id',
            'velocity_profile_speed': 'speed',
            'velocity_profile_orientation': 'bounding_box_orientation',
            'velocity_profile_speed_var': 'speed_variance',
            'velocity_profile_orientation_var': 'bounding_box_orientation_var'},
        inplace=True)

    gt_signals.loc[:, 'position_x'] = gt_signals.center_x
    gt_signals.loc[:, 'position_y'] = gt_signals.center_y
    gt_signals.loc[:, 'velocity_otg_x'] = gt_signals.speed * np.cos(gt_signals.bounding_box_orientation)
    gt_signals.loc[:, 'velocity_otg_y'] = gt_signals.speed * np.sin(gt_signals.bounding_box_orientation)
    gt_signals.loc[:, 'bounding_box_dimensions_x'] = np.nan
    gt_signals.loc[:, 'bounding_box_dimensions_y'] = np.nan
    # if position equals center then offset ratio is equal 0.5
    gt_signals.loc[:, 'bounding_box_refpoint_long_offset_ratio'] = 0.5
    gt_signals.loc[:, 'bounding_box_refpoint_lat_offset_ratio'] = 0.5
    gt_signals = gt_signals.loc[np.isfinite(gt_signals.speed), :]

    calc_relative_vel_for_objects(extracted.host.signals, gt_signals)
    place_bbox(gt_signals, extracted.detections.signals, stat_u_ids=stat_u_ids, prev_scans=prev_scans,
               next_scans=next_scans,
               range_value=range_value)
    # Drop rows where bbox were not created (bounding_box_x or y is a nan value)
    gt_signals = \
        gt_signals.drop(gt_signals[gt_signals.bounding_box_dimensions_x != gt_signals.bounding_box_dimensions_x].index)
    gt_objects = IObjects()
    gt_objects.signals = gt_signals
    print('Ground truth created.')
    return gt_objects


def calc_relative_vel_for_objects(host, objs):
    """
    Calculates relative velocity for objects.
    @param host: host data dataframe
    @param objs: objects data dataframe
    """
    host = host.loc[host.scan_index.drop_duplicates().index, :]
    objs_w_host_vel = objs.join(host.set_index('scan_index').loc[:, ['velocity_otg_x', 'velocity_otg_y']],
                                on='scan_index',
                                rsuffix='_host')

    objs.loc[:, 'velocity_rel_x'] = objs_w_host_vel.velocity_otg_x - objs_w_host_vel.velocity_otg_x_host
    objs.loc[:, 'velocity_rel_y'] = objs_w_host_vel.velocity_otg_y - objs_w_host_vel.velocity_otg_y_host


def calc_aspect_angle_based_size(df, unique_id, range_value=30):
    """
    Calculates constant bbox size considering object aspect angle. Only objects in given range are considered for bbox
    size calculation.
    @param df: data
    @param unique_id: object unique id
    @param range_value: range value
    """
    len_df = df.loc[df.unique_id == unique_id, 'bounding_box_dimensions_x']
    wid_df = df.loc[df.unique_id == unique_id, 'bounding_box_dimensions_y']
    mean_len = len_df.mean()
    mean_wid = wid_df.mean()
    df.loc[:, 'range'] = \
        np.hypot(df.loc[df.unique_id == unique_id, 'center_x'], df.loc[df.unique_id == unique_id, 'center_y'])

    # consider only objects within given range
    f_range = df['range'] < range_value

    # calculate absolute value of aspect angle to shorten condition lines
    aspect_angle = np.abs(df['aspect_angle'])
    # consider only objects in given region to calculate length
    f_angle_region_len = (np.deg2rad(45) < aspect_angle) & (aspect_angle < np.deg2rad(135))
    f_valid_len = f_range & f_angle_region_len
    # consider only objects in given region to calculate width
    f_angle_region_wid_1 = (np.deg2rad(180) > df['aspect_angle']) & (df['aspect_angle'] > np.deg2rad(135))
    f_angle_region_wid_2 = (np.deg2rad(45) > df['aspect_angle']) & (df['aspect_angle'] > np.deg2rad(0))
    f_valid_wid = f_range & (f_angle_region_wid_1 | f_angle_region_wid_2)

    len_df = len_df[f_valid_len].dropna()
    wid_df = wid_df[f_valid_wid].dropna()

    df.loc[df.unique_id == unique_id, 'bounding_box_dimensions_x'] = calc_constant_bbox_size(len_df, mean_len)
    df.loc[df.unique_id == unique_id, 'bounding_box_dimensions_y'] = calc_constant_bbox_size(wid_df, mean_wid)


def calc_constant_bbox_size(size_df, mean):
    """
    Set constant bbox size as dominant histogram value od length and width,
    if there is not enough data set given constant size.
    @param size_df: calculated size values, lengths or widths
    @param mean: mean of bbox size values
    @return: constant bbox size
    """
    if len(size_df) > 2:
        hist = np.histogram(size_df, density=True)
    else:
        return mean
    return hist[1][np.argmax(hist[0])]


def rotate_center_position(df, unique_id):
    """
    Performs rotation of center x and y positions. Function works for one object without looping through scan indexes.
    @param df: dataframe which includes bbox center positions
    @param unique_id: object unique id
    """
    center_x = df.loc[df.unique_id == unique_id, 'center_x']
    center_y = df.loc[df.unique_id == unique_id, 'center_y']
    orientation = df.loc[df.unique_id == unique_id, 'bounding_box_orientation']
    center = np.array([[center_x],
                       [center_y]]).T
    back_rot_mat = np.array([
        [np.cos(-orientation), -np.sin(-orientation)],
        [np.sin(-orientation), np.cos(-orientation)],
    ]).T
    center_back_rot = np.transpose(center @ back_rot_mat)
    df.loc[df.unique_id == unique_id, 'center_x'] = center_back_rot[0].T
    df.loc[df.unique_id == unique_id, 'center_y'] = center_back_rot[1].T



if __name__ == '__main__':
    log_p = r"E:\logfiles\DFT-1978\20220704_BMWSP25_iBrake_Asta\KPI_calculation\F360_HGR770_20220704_TrackerPC_7p07p00_SRR5_CrossTraffic_Ego0_Target60_20220704_185858_001.mudp"

    extracted = extract_f360_from_mudp(log_p,
                                       detections=True,
                                       internal_objects=True,
                                       raw_signals=True,
                                       execution_time_info=True)

    calc_velocity_profile_for_objects(extracted, stat_u_ids=[])
    extracted.detections_based_gt = create_ground_truth(extracted, stat_u_ids=[])
    save_to_pkl(extracted, log_p.replace('.mudp', '_f360_mudp_extracted.pickle'))

    obj = extracted.internal_objects.signals.query('unique_id == 923')

    speed_f, speed_axes = plt.subplots(nrows=3, sharex=True)
    speed_axes[0].plot(obj.scan_index, obj.speed)
    speed_axes[0].plot(obj.scan_index, obj.velocity_profile_speed)
    speed_axes[1].plot(obj.scan_index, obj.velocity_profile_speed_var)
    speed_axes[2].plot(obj.scan_index, obj.n_dets)
    for a in speed_axes:
        a.grid()

    ori_f, ori_axes = plt.subplots(nrows=3, sharex=True)
    ori_axes[0].plot(obj.scan_index, obj.bounding_box_orientation)
    ori_axes[0].plot(obj.scan_index, obj.velocity_profile_orientation)
    ori_axes[1].plot(obj.scan_index, obj.velocity_profile_orientation_var)
    ori_axes[2].plot(obj.scan_index, obj.n_dets)
    plt.grid()
    plt.show()