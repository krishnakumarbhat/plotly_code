from copy import deepcopy

from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.conversions import \
    create_ideal_est_from_ref, shift_objects, change_ref_point, shift_objects_by_indexes, join_two_ext_data
from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.ref_test_vectors import \
    get_ref_test_vectors
import numpy as np
import pandas as pd


def get_single_ref_tp_fn_fp_data():
    """
    Get data where there is maximum one reference object (one unique-id) and one or several estimated objects
    There may be False Positives and False Negatives as well - it is described in test vector name
    :return: dict with named test vectors,
             each test vector contains dict with'ref_data' and 'est_data' containing ExtractedData
    """
    ref_data = get_ref_test_vectors()
    data = dict()

    ref_test_vector = deepcopy(ref_data['single_long_leaving'])
    ref_test_vector_for_fp = deepcopy(ref_data['single_moving_same_dir_front'])

    est_test_vector_tp_ideal = create_ideal_est_from_ref(ref_test_vector)

    est_test_vector_tp_shift = deepcopy(est_test_vector_tp_ideal)
    shift_objects(est_test_vector_tp_shift.objects.signals, position_x=np.array(0.5), position_y=np.array(0.5))
    est_test_vector_tp_shift_split = deepcopy(est_test_vector_tp_ideal)
    shift_objects(est_test_vector_tp_shift_split.objects.signals,
                  unique_id=10, position_x=np.array(1.5), position_y=np.array(-1.5))

    est_test_vector_fp = create_ideal_est_from_ref(ref_test_vector_for_fp)

    tp_all_indexes = est_test_vector_tp_shift.objects.signals.index
    tp_first_index = est_test_vector_tp_shift.objects.signals.head(1).index
    tp_first5_indexes = est_test_vector_tp_shift.objects.signals.head(5).index
    tp_last4_indexes = est_test_vector_tp_shift.objects.signals.tail(4).index
    tp_last_index = est_test_vector_tp_shift.objects.signals.tail(1).index
    tp_first5_last1_indexes = pd.Index.union(tp_first5_indexes, tp_last_index)
    tp_middle_indexes = pd.Index.difference(tp_all_indexes, tp_first5_last1_indexes)

    fp_all_indexes = est_test_vector_fp.objects.signals.index
    fp_first_index = est_test_vector_fp.objects.signals.head(1).index
    fp_last_index = est_test_vector_fp.objects.signals.tail(1).index
    fp_first_last_indexes = pd.Index.union(fp_first_index, fp_last_index)
    fp_middle_indexes = pd.Index.difference(fp_all_indexes, fp_first_last_indexes)

    est_test_vector_one_ideal_tp = deepcopy(est_test_vector_tp_ideal)
    data['one_ideal_tp'] = {'ref_data': ref_test_vector,
                            'est_data': est_test_vector_one_ideal_tp,
                            'expected_kpis': {'TPR': 1.0, 'PPV': 1.0}}

    est_test_vector_one_tp = deepcopy(est_test_vector_tp_shift)
    data['one_tp'] = {'ref_data': ref_test_vector,
                      'est_data': est_test_vector_one_tp,
                      'expected_kpis': {'TPR': 1.0, 'PPV': 1.0}}

    est_test_vector_one_tp_one_fp = join_two_ext_data(est_test_vector_tp_shift, est_test_vector_fp)
    data['one_tp_one_fp'] = {'ref_data': ref_test_vector,
                             'est_data': est_test_vector_one_tp_one_fp,
                             'expected_kpis': {'TPR': 1.0, 'PPV': 0.7857}}

    est_test_vector_one_tp_one_split = join_two_ext_data(est_test_vector_tp_shift, est_test_vector_tp_shift_split)
    data['one_tp_one_split'] = {'ref_data': ref_test_vector,
                                'est_data': est_test_vector_one_tp_one_split,
                                'expected_kpis': {'TPR': 1.0, 'PPV': 0.5}}

    data['no_tp_one_fp_one_fn'] = {'ref_data': ref_test_vector,
                                   'est_data': est_test_vector_fp,
                                   'expected_kpis': {'TPR': 0.0, 'PPV': 0.0}}

    est_test_vector_one_ref_first_fn = deepcopy(est_test_vector_tp_shift)
    est_test_vector_one_ref_first_fn.objects.signals.drop(index=tp_first_index, inplace=True)
    data['one_ref_first_fn'] = {'ref_data': ref_test_vector,
                                'est_data': est_test_vector_one_ref_first_fn,
                                'expected_kpis': {'TPR': 0.909, 'PPV': 1.0}}

    est_test_vector_one_ref_first5_fn = deepcopy(est_test_vector_tp_shift)
    est_test_vector_one_ref_first5_fn.objects.signals.drop(index=tp_first5_indexes, inplace=True)
    data['one_ref_first5_fn'] = {'ref_data': ref_test_vector,
                                 'est_data': est_test_vector_one_ref_first5_fn,
                                 'expected_kpis': {'TPR': 0.5454, 'PPV': 1.0}}

    est_test_vector_one_ref_fn_in_middle = deepcopy(est_test_vector_tp_shift)
    est_test_vector_one_ref_fn_in_middle.objects.signals.drop(index=tp_middle_indexes, inplace=True)
    data['one_ref_fn_in_middle'] = {'ref_data': ref_test_vector,
                                    'est_data': est_test_vector_one_ref_fn_in_middle,
                                    'expected_kpis': {'TPR': 0.5454, 'PPV': 1.0}}

    est_test_vector_one_ref_last4_fn = deepcopy(est_test_vector_tp_shift)
    est_test_vector_one_ref_last4_fn.objects.signals.drop(index=tp_last4_indexes, inplace=True)
    data['one_ref_last4_fn'] = {'ref_data': ref_test_vector,
                                'est_data': est_test_vector_one_ref_last4_fn,
                                'expected_kpis': {'TPR': 0.6363, 'PPV': 1.0}}

    est_test_vector_one_ref_last_one_fn = deepcopy(est_test_vector_tp_shift)
    est_test_vector_one_ref_last_one_fn.objects.signals.drop(index=tp_last_index, inplace=True)
    data['one_ref_last_one_fn'] = {'ref_data': ref_test_vector,
                                   'est_data': est_test_vector_one_ref_last_one_fn,
                                   'expected_kpis': {'TPR': 0.909, 'PPV': 1.0}}

    est_test_vector_one_tp_one_split_in_beg_and_end = join_two_ext_data(est_test_vector_one_ref_fn_in_middle,
                                                                        est_test_vector_tp_shift_split)
    data['one_tp_one_split_in_beg_and_end'] = {'ref_data': ref_test_vector,
                                               'est_data': est_test_vector_one_tp_one_split_in_beg_and_end,
                                               'expected_kpis': {'TPR': 1.0, 'PPV': 0.6471}}

    est_test_vector_fp_without_last_one = deepcopy(est_test_vector_fp)
    est_test_vector_fp_without_last_one.objects.signals.drop(index=fp_last_index, inplace=True)
    est_test_one_ref_one_tp_single_fp_without_last_one = join_two_ext_data(est_test_vector_tp_shift,
                                                                           est_test_vector_fp_without_last_one)
    data['one_ref_one_tp_single_fp_without_last_one'] = {'ref_data': ref_test_vector,
                                                         'est_data': est_test_one_ref_one_tp_single_fp_without_last_one,
                                                         'expected_kpis': {'TPR': 1.0, 'PPV': 0.8461}}

    est_test_vector_one_ref_single_fn_in_middle_fp_last_one = join_two_ext_data(est_test_vector_one_ref_fn_in_middle,
                                                                                est_test_vector_fp_without_last_one)

    data['one_ref_single_fn_in_middle_single_fp_without_last_one'] = {'ref_data': ref_test_vector,
                                                                      'est_data': est_test_vector_one_ref_single_fn_in_middle_fp_last_one,
                                                                      'expected_kpis': {'TPR': 0.5454, 'PPV': 0.75}}

    return data


