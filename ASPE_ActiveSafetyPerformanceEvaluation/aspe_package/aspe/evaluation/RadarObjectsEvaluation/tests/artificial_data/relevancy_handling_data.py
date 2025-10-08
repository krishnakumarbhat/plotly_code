from copy import deepcopy

from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.conversions import \
    create_ideal_est_from_ref, shift_objects, change_ref_point, shift_objects_by_indexes, join_two_ext_data
from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.ref_test_vectors import \
    get_ref_test_vectors

import numpy as np


def get_relevancy_handling_data():
    """
    Get data where there is maximum one reference object (one unique-id) and one or several estimated objects
    There may be False Positives and False Negatives as well - it is described in test vector name
    :return: dict with named test vectors,
             each test vector contains dict with'ref_data' and 'est_data' containing ExtractedData
    """
    ref_data = get_ref_test_vectors()
    data = dict()

    ref_entering_fov_data = deepcopy(ref_data['single_entering_fov'])
    ref_out_of_fov_data = deepcopy(ref_data['single_moving_out_of_fov'])
    ref_moving_in_fov = deepcopy(ref_data['single_long_leaving'])
    ref_stationary_in_fov = deepcopy(ref_data['single_stationary_front'])
    ref_entering_and_in_fov = join_two_ext_data(ref_entering_fov_data, ref_moving_in_fov)
    ref_entering_and_out_fov = join_two_ext_data(ref_entering_fov_data, ref_out_of_fov_data)
    ref_moving_stat = join_two_ext_data(ref_moving_in_fov, ref_stationary_in_fov)
    ref_all = join_two_ext_data(ref_entering_and_out_fov, ref_moving_stat)

    est_entering_fov_data_shift = create_ideal_est_from_ref(ref_entering_fov_data)
    shift_objects(est_entering_fov_data_shift.objects.signals, position_x=np.array(0.5), position_y=np.array(0.5))

    est_entering_fov_data_late_init = deepcopy(est_entering_fov_data_shift)
    first_4_indexes = est_entering_fov_data_late_init.objects.signals.head(4).index
    est_entering_fov_data_late_init.objects.signals.drop(index=first_4_indexes, inplace=True)

    est_moving = create_ideal_est_from_ref(ref_moving_in_fov)
    shift_objects(est_moving.objects.signals, position_x=np.array(1.0), position_y=np.array(-0.3))

    est_stationary = create_ideal_est_from_ref(ref_stationary_in_fov)
    shift_objects(est_stationary.objects.signals, position_x=np.array(-0.3), position_y=np.array(0.3))

    est_moving_stat = join_two_ext_data(est_moving, est_stationary)
    est_all = join_two_ext_data(est_moving_stat, est_entering_fov_data_late_init)

    data['entering_fov_all_tp'] = {'ref_data': ref_entering_fov_data,
                                   'est_data': est_entering_fov_data_shift}

    data['late_init'] = {'ref_data': ref_entering_fov_data,
                         'est_data': est_entering_fov_data_late_init}

    data['not_rel_tp_fp_fn'] = {'ref_data': ref_entering_and_in_fov,
                                'est_data': est_moving_stat}

    data['moving_stat'] = {'ref_data': ref_moving_stat,
                           'est_data': est_moving_stat}

    data['all'] = {'ref_data': ref_all,
                   'est_data': est_all}

    return data
