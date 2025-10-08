import numpy as np
import pandas as pd

from AptivPerformanceEvaluation.RadarObjectsEvaluation.DataStructures.PEObjectsEvaluationOutput import \
    PEObjectsEvaluationOutputSingleLog


def calc_event_kpis(pe_out: PEObjectsEvaluationOutputSingleLog, log_name=None):
    ref_event_idx = None
    init_time_dev = None
    init_speed_dev = None
    init_orientation_dev = None
    max_speed_error = None
    max_orientation_error = None

    ref_res = pe_out.pe_results_obj_ref.signals

    # calc object's initialization scan index
    asso_diff = ref_res['is_associated'].astype(int).diff()
    init_index = np.where(asso_diff == 1)
    if ref_res['is_associated'].iloc[0]:
        init_si = ref_res.iloc[0]['scan_index']
    elif init_index[0].size == 0:
        init_si = None
    else:
        init_si = ref_res.iloc[init_index[0][0]]['scan_index']

    # finding event's initialization object
    if init_si is not None:
        pairs_df = pe_out.pe_results_obj_pairs.signals
        si_mask = pairs_df['scan_index'].values == init_si
        asso_mask = pairs_df['is_associated'].values == 1
        init_est_obj_from_pair = pairs_df[si_mask & asso_mask]
        init_unique_id = init_est_obj_from_pair['unique_id_est'].iloc[0]

        # when init obj had been found, kpis are calculated
        # event initialization time dev calc
        asso_flags = ref_res['is_associated']
        rel_flags = ref_res['relevancy_flag']

        asso_flags_idxes = asso_flags.where(asso_flags == 1).dropna()
        rel_flags_idexes = rel_flags.where(rel_flags == 1).dropna()

        # it is known that asso array is not empty and that is objs associated
        if rel_flags_idexes.empty:
            # we dont expect any initialization so if there is at least one association so its 0
            init_time_dev = 0
        else:
            obs_init_idx_slot = rel_flags_idexes.index[0]
            exp_init_idx_slot = asso_flags_idxes.index[0]
            init_time_dev = obs_init_idx_slot - exp_init_idx_slot  # both structures filled so init time can be calc

        pairs_init_row = pairs_df[
            np.logical_and(pairs_df['scan_index'] == init_si, pairs_df['unique_id_est'] == init_unique_id)]
        ref_event_idx = pairs_init_row['unique_id_ref'].iloc[0]
        # event_init_speed_dev_calc
        init_speed_dev = pairs_init_row['dev_speed'].iloc[0]

        # event_init_orientation_dev_calc
        init_orientation_dev = pairs_init_row['dev_orientation'].iloc[0]

        # event_max_speed_error
        asso_pairs = pairs_df[pairs_df['is_associated'].values == 1]
        max_speed_error = max(abs(asso_pairs['dev_speed']))

        # event_max_orientation_error
        max_orientation_error = max(abs(asso_pairs['dev_orientation']))

    # aggregate
    out_event_df = pd.DataFrame(
        data=[[log_name,
               ref_event_idx,
               pe_out.kpis_binary_class_per_log.iloc[0, 1],
               init_time_dev,
               init_speed_dev,
               init_orientation_dev,
               max_speed_error,
               max_orientation_error]],
        columns=['log_name',
                 'ref_event_idx',
                 'TPR',
                 'init_time_dev',
                 'init_speed_dev',
                 'init_orientation_dev',
                 'max_speed_error',
                 'max_orientation_error']
    )
    return out_event_df
