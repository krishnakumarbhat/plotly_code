import re
from pathlib import Path

import numpy as np
import pandas as pd
from tqdm import tqdm

from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PEObjectsEvaluationOutput import (
    PEObjectsEvaluationOutputMultiLog,
)
from aspe.extractors.Nexus.NexusExtractedData import NexusExtractedData
from aspe.utilities.nexus50k_events_finder.event_finders.base_event_finder import BaseNexusEventFinder
from aspe.utilities.nexus50k_events_finder.user_config import user_config
from aspe.utilities.SupportingFunctions import load_from_pkl

REAR_AXLE_TO_FRONT_BUMPER = 3.717
LIDAR_TO_REAR_AXLE = 0.74


def align_coordinate_systems(extracted_nexus: NexusExtractedData):
    extracted_nexus.detections.signals.loc[:, 'position_x'] -= REAR_AXLE_TO_FRONT_BUMPER
    extracted_nexus.detections.raw_signals.loc[:, 'position_x'] -= REAR_AXLE_TO_FRONT_BUMPER

    extracted_nexus.auto_gt_lidar_cuboids.transfer_cs('VCS', REAR_AXLE_TO_FRONT_BUMPER - 0.74)
    # todo hardcoded value - this should be done by using lidar data


def get_dets_signals(pe_out: PEObjectsEvaluationOutputMultiLog, event_name: str, event_id: int):
    dets = pe_out.extracted_radar_detections.signals

    signals_to_save = ['scan_index', 'timestamp', 'slot_id', 'sensor_id', 'position_x', 'position_y', 'azimuth_vcs',
                       'range_rate',
                       'range_rate_comp']
    dets_out = dets.loc[:, signals_to_save]
    dets_out['even_name'] = event_name
    dets_out['event_id'] = event_id

    return dets_out


def get_ground_truth_signals(pe_out: PEObjectsEvaluationOutputMultiLog, event_name: str, event_id: int):
    gt = pe_out.extracted_reference_objects.signals

    signals_to_save = ['scan_index', 'timestamp', 'unique_id', 'object_class', 'movement_status',
                       'center_x', 'center_y',
                       'velocity_otg_x', 'velocity_otg_y',
                       'velocity_rel_x', 'velocity_rel_y',
                       'bounding_box_dimensions_x', 'bounding_box_dimensions_y', 'bounding_box_orientation',
                       'visibility_rate', 'n_dets']

    gt_out = gt.loc[:, signals_to_save]
    gt_out['even_name'] = event_name
    gt_out['event_id'] = event_id

    pairs = pe_out.pe_results_obj_pairs.signals
    pe_features_ref_idx = pairs.index_ref.to_numpy()

    errors_to_save = ['dev_position_x',
                      'dev_position_y',
                      'dev_bounding_box_dimensions_x',
                      'dev_bounding_box_dimensions_y',
                      'intersection_over_union',
                      'binary_classification']

    errors = pairs.loc[:, errors_to_save]

    gt_out.loc[pe_features_ref_idx, 'error_position_x'] = errors.loc[:, 'dev_position_x'].to_numpy()
    gt_out.loc[pe_features_ref_idx, 'error_position_y'] = errors.loc[:, 'dev_position_y'].to_numpy()
    gt_out.loc[pe_features_ref_idx, 'error_length'] = errors.loc[:, 'dev_bounding_box_dimensions_x'].to_numpy()
    gt_out.loc[pe_features_ref_idx, 'error_width'] = errors.loc[:, 'dev_bounding_box_dimensions_y'].to_numpy()
    gt_out.loc[pe_features_ref_idx, 'iou'] = errors.loc[:, 'intersection_over_union'].to_numpy()
    gt_out.loc[pe_features_ref_idx, 'binary_classification'] = errors.loc[:, 'binary_classification'].to_numpy()

    gt_out.query('visibility_rate > 0.0', inplace=True)

    return gt_out


def associate_dets_to_ground_truth(dets: pd.DataFrame, auto_gt: pd.DataFrame, rrate_gate_threshold=1.0,
                                   zone_gate_margin=0.5):
    def translate_and_rotate_positions(pos_mat, translation_vec, rotation_angle):
        transformed = pos_mat - translation_vec  # translation
        rot_mat = np.array([
            [np.cos(rotation_angle), -np.sin(rotation_angle)],
            [np.sin(rotation_angle), np.cos(rotation_angle)],
        ])
        transformed = (rot_mat @ transformed.T).T  # rotation
        return transformed

    def points_inside_bbox_in_same_cs(pos_mat, bbox_len, bbox_width, zone_gate_margin):
        within_long = np.abs(pos_mat[:, 0]) < (bbox_len / 2 + zone_gate_margin)
        within_lat = np.abs(pos_mat[:, 1]) < (bbox_width / 2 + zone_gate_margin)
        return within_long & within_lat

    def rrate_error_below_threshold(bbox_vel_x, bbox_vel_y, dets_az_cos, dets_az_sin, dets_rrate_comp):
        predicted_rrate = bbox_vel_x * dets_az_cos + bbox_vel_y * dets_az_sin
        rrate_error = np.abs(dets_rrate_comp - predicted_rrate)
        return rrate_error < rrate_gate_threshold

    dets['assoc_gt_unique_id'] = np.nan
    dets_per_scan = dets.groupby(by='scan_index')
    for scan_idx, auto_gt_scan in auto_gt.groupby(by='scan_index'):
        dets_scan = dets_per_scan.get_group(scan_idx)

        dets_pos = dets_scan.loc[:, ['position_x', 'position_y']].to_numpy()
        dets_az_cos = np.cos(dets_scan.loc[:, 'azimuth_vcs'].to_numpy())
        dets_az_sin = np.sin(dets_scan.loc[:, 'azimuth_vcs'].to_numpy())
        dets_rrate_comp = dets_scan.loc[:, 'range_rate_comp'].to_numpy()

        for row_idx, bbox in auto_gt_scan.iterrows():
            dx_dy = bbox[['center_x', 'center_y']].to_numpy()
            dets_transformed = translate_and_rotate_positions(dets_pos, dx_dy, bbox['bounding_box_orientation'])

            bbox_len, bbox_wid = bbox['bounding_box_dimensions_x'], bbox['bounding_box_dimensions_y']
            inside_zone = points_inside_bbox_in_same_cs(dets_transformed, bbox_len, bbox_wid, zone_gate_margin)

            inside_rrate_gate = rrate_error_below_threshold(bbox['velocity_otg_x'], bbox['velocity_otg_y'],
                                                            dets_az_cos, dets_az_sin, dets_rrate_comp)

            is_associated = inside_zone & inside_rrate_gate

            assoc_indexes = dets_scan.index.to_numpy()[is_associated]
            dets.loc[assoc_indexes, 'assoc_gt_unique_id'] = bbox.unique_id

            # get information about normalized (-1 to 1) tcs position in bbox
            dets_transformed[:, 0] = dets_transformed[:, 0] / (bbox_len / 2.0)
            dets_transformed[:, 1] = dets_transformed[:, 1] / (bbox_wid / 2.0)
            dets.loc[assoc_indexes, 'tcs_norm_pos_x'] = dets_transformed[is_associated, 0]
            dets.loc[assoc_indexes, 'tcs_norm_pos_y'] = dets_transformed[is_associated, 1]

            auto_gt.loc[row_idx, 'n_dets'] = is_associated.sum()


def extract_dets_and_auto_gt_info_from_events_eval_outputs(event_name: str, pe_out_dir: str, resim_suffix: str):
    pe_out_paths = [str(p) for p in Path(pe_out_dir).glob(f"{event_name}_events_*_{resim_suffix}_pe_output.pickle")]

    dets_agg = []
    gt_agg = []

    for p in tqdm(pe_out_paths):
        print(f"Processing {p}")
        event_id = int(re.search('events_(.+?)_', p).group(1))

        pe_out = load_from_pkl(p)
        if len(pe_out.extracted_radar_detections.signals):
            dets = get_dets_signals(pe_out, event_name, event_id)
            gt = get_ground_truth_signals(pe_out, event_name, event_id)
            associate_dets_to_ground_truth(dets, gt, rrate_gate_threshold=1.5, zone_gate_margin=1.0)
            gt.query('n_dets > 0', inplace=True)
            dets = dets.dropna() # drop non associated dets

            dets_agg.append(dets)
            gt_agg.append(gt)

    dets_agg = pd.concat(dets_agg).reset_index(drop=True)
    gt_agg = pd.concat(gt_agg).reset_index(drop=True)
    return dets_agg, gt_agg


def calculate_psuedo_size_err(dets: pd.DataFrame, auto_gt: pd.DataFrame):
    buffer_wing = 1
    dets_grouped = dets.groupby(by=['event_id', 'scan_index', 'assoc_gt_unique_id'])
    auto_gt_grouped = auto_gt.set_index(['event_id', 'scan_index', 'unique_id'])

    for (event_id, scan_index, gt_id), scan_dets in tqdm(dets_grouped):
        sub_dets = [scan_dets]

        for delta in range(-buffer_wing, buffer_wing + 1, 1):
            if delta == 0:
                continue
            if (event_id, scan_index + delta, gt_id) in dets_grouped.groups:
                sub_dets.append(dets_grouped.get_group((event_id, scan_index + delta, gt_id)))
        df = pd.concat(sub_dets)

        if (df.tcs_norm_pos_x > 0).any() & (df.tcs_norm_pos_x < 0).any():
            psuedo_len = df.tcs_norm_pos_x.max() - df.tcs_norm_pos_x.min()
        else:
            psuedo_len = df.tcs_norm_pos_x.abs().max()

        if (df.tcs_norm_pos_y > 0).any() & (df.tcs_norm_pos_y < 0).any():
            psuedo_wid = df.tcs_norm_pos_y.max() - df.tcs_norm_pos_y.min()
        else:
            psuedo_wid = df.tcs_norm_pos_y.abs().max()

        auto_gt_grouped.loc[(event_id, scan_index, gt_id), 'pseudo_len'] = psuedo_len
        auto_gt_grouped.loc[(event_id, scan_index, gt_id), 'pseudo_wid'] = psuedo_wid

    auto_gt = auto_gt_grouped.reset_index()
    auto_gt['pseudo_len'] = auto_gt['pseudo_len'] * auto_gt['bounding_box_dimensions_x'] / 2
    auto_gt['pseudo_wid'] = auto_gt['pseudo_wid'] * auto_gt['bounding_box_dimensions_y'] / 2

    auto_gt['pseudo_err_len'] = auto_gt.pseudo_len - auto_gt.bounding_box_dimensions_x
    auto_gt['pseudo_err_wid'] = auto_gt.pseudo_wid - auto_gt.bounding_box_dimensions_y
    return auto_gt


if __name__ == '__main__':
    event_name = 'overtaking'

    resims = ["rRf360t7020309v205p50_SW_7_02"]

    dir_to_save = r'E:\logfiles\nexus_50k_eval_data\auto_gt_and_detections'
    if not Path(dir_to_save).exists():
        Path(dir_to_save).mkdir(exist_ok=True)

    for resim_suffix in resims:
        dets, auto_gt = extract_dets_and_auto_gt_info_from_events_eval_outputs(event_name,
                                                                               pe_out_dir=r"E:\perf_eval_out",
                                                                               resim_suffix=resim_suffix)

        dets_save_p = f'{dir_to_save}\\{event_name}_{resim_suffix}_detections.csv'
        gt_save_p = f'{dir_to_save}\\{event_name}_{resim_suffix}_auto_gt.csv'

        dets.to_csv(dets_save_p)
        auto_gt.to_csv(gt_save_p)
