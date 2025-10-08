from copy import deepcopy

from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.conversions import \
    create_ideal_est_from_ref, shift_objects, change_ref_point, shift_objects_by_indexes
from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.ref_test_vectors import \
    get_ref_test_vectors
import numpy as np


def get_shifted_data_2m_x_minus1m_y():
    """
    Get data where estimated objects are shifted by 2m in x position and 1m in y position
    Number and name of test vectors are the same as for output from get_ref_test_vectors() function
    :return: dict with named test vectors,
             each test vector contains dict with'ref_data' and 'est_data' containing ExtractedData
    """

    ref_data = get_ref_test_vectors()
    shifted_data = dict()
    for key, ref_test_vector in ref_data.items():
        est_test_vector = create_ideal_est_from_ref(ref_test_vector)
        shift_objects(est_test_vector.objects.signals, position_x=np.array(2.0), position_y=np.array(-1.0))
        shifted_data[key] = {'ref_data': ref_test_vector, 'est_data': est_test_vector}
    return shifted_data


def get_shifted_ref_point_rear_right():
    """
    Get data where estimated objects reference point is shifted to rear right reference point (x-0, y-1)
    Object Position is the same, velocity is transformed by yaw rate influence
    Number and name of test vectors are the same as for output from get_ref_test_vectors() function
    :return: dict with named test vectors,
             each test vector contains dict with'ref_data' and 'est_data' containing ExtractedData
    """

    ref_data = get_ref_test_vectors()
    shifted_data = dict()
    for key, ref_test_vector in ref_data.items():
        est_test_vector = create_ideal_est_from_ref(ref_test_vector)
        change_ref_point(est_test_vector.objects.signals,
                         out_ref_point_long=np.array(0.0), out_ref_point_lat=np.array(1.0))
        shifted_data[key] = {'ref_data': ref_test_vector, 'est_data': est_test_vector}
    return shifted_data


def get_different_shifts():
    """
    Get data where estimated objects are shifted differently. Shift type and value is described in test vector name
    :return: dict with named test vectors,
             each test vector contains dict with'ref_data' and 'est_data' containing ExtractedData
    """
    ref_data = get_ref_test_vectors()
    shifted_data = dict()

    ref_test_vector = deepcopy(ref_data['single_moving_same_dir_front'])
    est_test_vector = create_ideal_est_from_ref(ref_test_vector)

    est_test_vector_shift_10m_x = deepcopy(est_test_vector)
    shift_objects(est_test_vector_shift_10m_x.objects.signals, position_x=np.array(10.0))
    shifted_data['shift_10m_x'] = {'ref_data': ref_test_vector,
                                   'est_data': est_test_vector_shift_10m_x}

    est_test_vector_shift_15m_y = deepcopy(est_test_vector)
    shift_objects(est_test_vector_shift_15m_y.objects.signals, position_y=np.array(15.0))
    shifted_data['shift_15m_y'] = {'ref_data': ref_test_vector,
                                   'est_data': est_test_vector_shift_15m_y}

    est_test_vector_shift_0_1m_xy = deepcopy(est_test_vector)
    shift_objects(est_test_vector_shift_0_1m_xy.objects.signals,
                  position_x=np.array(0.1), position_y=np.array(0.1))
    shifted_data['shift_0_1m_xy'] = {'ref_data': ref_test_vector,
                                     'est_data': est_test_vector_shift_0_1m_xy}

    est_test_vector_shift_second_si_8m_x = deepcopy(est_test_vector)
    shift_objects_by_indexes(est_test_vector_shift_second_si_8m_x.objects.signals,
                             indexes=1, position_x=np.array(8.0))
    shifted_data['shift_second_si_8m_x'] = {'ref_data': ref_test_vector,
                                            'est_data': est_test_vector_shift_second_si_8m_x}

    est_test_vector_shift_second_si_8m_x_rest_1m = deepcopy(est_test_vector)
    shift_objects(est_test_vector_shift_second_si_8m_x_rest_1m.objects.signals, position_x=np.array(1.0))
    shift_objects_by_indexes(est_test_vector_shift_second_si_8m_x_rest_1m.objects.signals,
                             indexes=1, position_x=np.array(7.0))
    shifted_data['shift_second_si_8m_x_rest_1m'] = {'ref_data': ref_test_vector,
                                                    'est_data': est_test_vector_shift_second_si_8m_x_rest_1m}

    est_test_vector_shift_orientation_45_deg = deepcopy(est_test_vector)
    shift_objects(est_test_vector_shift_orientation_45_deg.objects.signals,
                  bounding_box_orientation=np.array(np.deg2rad(45)), velocity_otg_x=-3.0, velocity_otg_y=7.0)
    shifted_data['shift_orientation_45_deg'] = {'ref_data': ref_test_vector,
                                                'est_data': est_test_vector_shift_orientation_45_deg}

    est_test_vector_shift_orientation_45_deg_first_si = deepcopy(est_test_vector)
    shift_objects_by_indexes(est_test_vector_shift_orientation_45_deg_first_si.objects.signals, indexes=0,
                             bounding_box_orientation=np.array(np.deg2rad(45)), velocity_otg_x=-3.0, velocity_otg_y=7.0)
    shifted_data['shift_orientation_45_deg_first_si'] = {'ref_data': ref_test_vector,
                                                         'est_data': est_test_vector_shift_orientation_45_deg_first_si}

    est_test_vector_several_shifts = deepcopy(est_test_vector)
    shift_objects_by_indexes(est_test_vector_several_shifts.objects.signals, indexes=[0, 1],
                             bounding_box_orientation=np.array(np.deg2rad(45)), velocity_otg_x=-3.0, velocity_otg_y=7.0)
    shift_objects_by_indexes(est_test_vector_several_shifts.objects.signals, indexes=[1, 2],
                             position_x=2.0, position_y=0.3)
    shift_objects_by_indexes(est_test_vector_several_shifts.objects.signals, indexes=2,
                             velocity_otg_x=-2.0, velocity_otg_y=1.0)
    change_ref_point(est_test_vector_several_shifts.objects.signals,
                     out_ref_point_long=np.array(0.3), out_ref_point_lat=np.array(0.2))
    shifted_data['several_shifts'] = {'ref_data': ref_test_vector,
                                      'est_data': est_test_vector_several_shifts}

    return shifted_data


