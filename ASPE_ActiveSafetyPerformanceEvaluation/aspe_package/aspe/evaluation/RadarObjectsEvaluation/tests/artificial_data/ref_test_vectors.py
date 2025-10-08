import numpy as np
from aspe.extractors.Interfaces.Enums.Object import MovementStatus

from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.conversions import create_ref, join_two_ext_data

PI = np.pi
PI_2 = np.pi/2


def get_ref_test_vectors():
    ref_test_vectors = dict()

    ref_test_vectors['single_moving_same_dir_front'] = create_ref([
            [0, 1000.00, 0, 20.0,  20.0,10.0, 0.0,  0.0,  5.0, 2.0, 0.0, 0.0, 0.0, MovementStatus.MOVING],
            [1, 1000.05, 0, 20.5,  20.0,10.0, 0.0,  0.0,  5.0, 2.0, 0.0, 0.0, 0.0, MovementStatus.MOVING],
            [2, 1000.10, 0, 21.0,  20.0,10.0, 0.0,  0.0,  5.0, 2.0, 0.0, 0.0, 0.0, MovementStatus.MOVING],
        ])

    ref_test_vectors['single_moving_crossing_turning'] = create_ref([
            [0, 1000.00, 1, 20.0, -20.0, 0.0,40.0, PI_2,  5.0, 2.0, 1.0, 1.0, 0.5, MovementStatus.MOVING],
            [1, 1000.05, 1, 19.9, -18.1,-2.0,39.0,  1.6,  5.0, 2.0, 1.0, 1.0, 0.5, MovementStatus.MOVING],
            [2, 1000.10, 1, 19.7, -16.3,-4.0,38.0, 1.63,  5.0, 2.0, 1.0, 1.0, 0.5, MovementStatus.MOVING],
        ])

    ref_test_vectors['single_stationary_front'] = create_ref([
            [0, 1000.00, 2, 40.0, -20.0, 0.0, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.STATIONARY],
            [1, 1000.05, 2, 40.0, -20.0, 0.0, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.STATIONARY],
            [2, 1000.10, 2, 40.0, -20.0, 0.0, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.STATIONARY],
        ])

    ref_test_vectors['single_stationary_front2'] = create_ref([
        [0, 1000.00, 1, 40.0, -20.0, 0.0, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.STATIONARY],
        [1, 1000.05, 1, 40.0, -20.0, 0.0, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.STATIONARY],
        [2, 1000.10, 1, 40.0, -20.0, 0.0, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.STATIONARY],
        [3, 1000.05, 1, 40.0, -20.0, 0.0, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.STATIONARY],
        [4, 1000.10, 1, 40.0, -20.0, 0.0, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.STATIONARY],
        [5, 1000.05, 1, 40.0, -20.0, 0.0, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.STATIONARY],
        [6, 1000.10, 1, 40.0, -20.0, 0.0, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.STATIONARY],
        [7, 1000.05, 1, 40.0, -20.0, 0.0, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.STATIONARY],
        [8, 1000.10, 1, 40.0, -20.0, 0.0, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.STATIONARY],
    ])

    ref_test_vectors['single_moving_front'] = create_ref([
        [0, 1000.00, 1, 39.2, -20.0, 0.2, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.MOVING],
        [1, 1000.05, 1, 39.4, -20.0, 0.2, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.MOVING],
        [2, 1000.10, 1, 39.6, -20.0, 0.2, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.MOVING],
        [3, 1000.05, 1, 39.8, -20.0, 0.2, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.MOVING],
        [4, 1000.10, 1, 40.0, -20.0, 0.2, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.MOVING],
        [5, 1000.05, 1, 40.2, -20.0, 0.2, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.MOVING],
        [6, 1000.10, 1, 40.4, -20.0, 0.2, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.MOVING],
        [7, 1000.05, 1, 40.6, -20.0, 0.2, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.MOVING],
        [8, 1000.10, 1, 40.8, -20.0, 0.2, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.MOVING],
    ])

    ref_test_vectors['single_moving_front2'] = create_ref([
        [0, 1000.00, 2, 39.2, -20.2, 0.2, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.MOVING],
        [1, 1000.05, 2, 39.4, -20.2, 0.2, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.MOVING],
        [2, 1000.10, 2, 39.6, -20.2, 0.2, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.MOVING],
        [3, 1000.05, 2, 39.8, -20.2, 0.2, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.MOVING],
        [4, 1000.10, 2, 40.0, -20.2, 0.2, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.MOVING],
        [5, 1000.05, 2, 40.2, -20.2, 0.2, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.MOVING],
        [6, 1000.10, 2, 40.4, -20.2, 0.2, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.MOVING],
        [7, 1000.05, 2, 40.6, -20.2, 0.2, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.MOVING],
        [8, 1000.10, 2, 40.8, -20.2, 0.2, 0.0,  0.0,  5.0, 2.0, 1.0, 0.0, 0.0, MovementStatus.MOVING],
    ])

    ref_test_vectors['single_stopped_back'] = create_ref([
            [2, 1000.10, 3,-50.0, -10.0, 0.0, 0.0,  -PI, 10.0, 2.0, 0.0, 0.0, 0.0, MovementStatus.STATIONARY],
            [3, 1000.15, 3,-51.0, -10.0,-20.0,0.0,  -PI, 10.0, 2.0, 0.0, 0.0, 0.0, MovementStatus.MOVING],
            [4, 1000.20, 3,-51.0, -10.0, 0.0, 0.0,  -PI, 10.0, 2.0, 0.0, 0.0, 0.0, MovementStatus.STOPPED],
        ])

    ref_test_vectors['single_moving_out_of_fov'] = create_ref([
            [2, 1000.05, 4,300.0, -10.0, 0.0,10.0, PI_2,  5.0, 2.0, 1.0, 1.0, 0.0, MovementStatus.MOVING],
            [3, 1000.10, 4,300.0,  -9.5, 0.0,10.0, PI_2,  5.0, 2.0, 1.0, 1.0, 0.0, MovementStatus.MOVING],
        ])

    ref_test_vectors['single_stationary_out_of_fov'] = create_ref([
            [1, 1000.00, 5,300.0,  10.0, 0.0, 0.0, -PI_2, 1.0, 1.0, 0.0, 1.0, 0.0, MovementStatus.STATIONARY],
            [2, 1000.10, 5,300.0,  10.0, 0.0, 0.0, -PI_2, 1.0, 1.0, 0.0, 1.0, 0.0, MovementStatus.STATIONARY],
        ])

    ref_test_vectors['single_entering_fov'] = create_ref([
            [2, 1000.00, 6,120.0, -5.0,-50.0, 0.0, -PI, 4.0, 2.0, 0.0, 1.0, 0.0, MovementStatus.MOVING],
            [3, 1000.05, 6,110.0, -5.0,-50.0, 0.0, -PI, 4.0, 2.0, 0.0, 1.0, 0.0, MovementStatus.MOVING],
            [4, 1000.10, 6,100.0, -5.0,-50.0, 0.0, -PI, 4.0, 2.0, 0.0, 1.0, 0.0, MovementStatus.MOVING],
            [5, 1000.15, 6, 90.0, -5.0,-50.0, 0.0, -PI, 4.0, 2.0, 0.0, 1.0, 0.0, MovementStatus.MOVING],
            [6, 1000.20, 6, 80.0, -5.0,-50.0, 0.0, -PI, 4.0, 2.0, 0.0, 1.0, 0.0, MovementStatus.MOVING],
            [7, 1000.25, 6, 70.0, -5.0,-50.0, 0.0, -PI, 4.0, 2.0, 0.0, 1.0, 0.0, MovementStatus.MOVING]
        ])

    ref_test_vectors['single_long_leaving'] = create_ref([
            [1, 1000.00, 7, 0.0, 5.0, 10.0, 0.0, 0.0, 5.0, 2.0, 0.0, 0.0, 0.0, MovementStatus.MOVING],
            [2, 1000.05, 7, 0.5, 5.0, 10.0, 0.0, 0.0, 5.0, 2.0, 0.0, 0.0, 0.0, MovementStatus.MOVING],
            [3, 1000.10, 7, 1.0, 5.0, 10.0, 0.0, 0.0, 5.0, 2.0, 0.0, 0.0, 0.0, MovementStatus.MOVING],
            [4, 1000.15, 7, 1.5, 5.0, 10.0, 0.0, 0.0, 5.0, 2.0, 0.0, 0.0, 0.0, MovementStatus.MOVING],
            [5, 1000.20, 7, 2.0, 5.0, 10.0, 0.0, 0.0, 5.0, 2.0, 0.0, 0.0, 0.0, MovementStatus.MOVING],
            [6, 1000.25, 7, 2.5, 5.0, 10.0, 0.0, 0.0, 5.0, 2.0, 0.0, 0.0, 0.0, MovementStatus.MOVING],
            [7, 1000.30, 7, 3.0, 5.0, 10.0, 0.0, 0.0, 5.0, 2.0, 0.0, 0.0, 0.0, MovementStatus.MOVING],
            [8, 1000.35, 7, 3.5, 5.0, 10.0, 0.0, 0.0, 5.0, 2.0, 0.0, 0.0, 0.0, MovementStatus.MOVING],
            [9, 1000.40, 7, 4.0, 5.0, 10.0, 0.0, 0.0, 5.0, 2.0, 0.0, 0.0, 0.0, MovementStatus.MOVING],
            [10,1000.45, 7, 4.5, 5.0, 10.0, 0.0, 0.0, 5.0, 2.0, 0.0, 0.0, 0.0, MovementStatus.MOVING],
            [11,1000.50, 7, 5.0, 5.0, 10.0, 0.0, 0.0, 5.0, 2.0, 0.0, 0.0, 0.0, MovementStatus.MOVING],
        ])

    ref_test_vectors['two_moving_in_fov'] = join_two_ext_data(ref_test_vectors['single_moving_same_dir_front'],
                                                              ref_test_vectors['single_moving_crossing_turning'])

    ref_test_vectors['two_one_in_one_entering_fov'] = join_two_ext_data(ref_test_vectors['single_moving_same_dir_front'],
                                                                        ref_test_vectors['single_entering_fov'])

    ref_test_vectors['two_moving_stationary'] = join_two_ext_data(ref_test_vectors['single_moving_same_dir_front'],
                                                                  ref_test_vectors['single_stationary_front'])

    ref_test_vectors['two_one_short_one_long_leaving'] = join_two_ext_data(ref_test_vectors['single_moving_same_dir_front'],
                                                                           ref_test_vectors['single_long_leaving'])

    ref_test_vectors['three_moving_stationary_stopped'] = join_two_ext_data(ref_test_vectors['two_moving_stationary'],
                                                                            ref_test_vectors['single_stopped_back'])

    ref_test_vectors['two_moving_in_out_fov'] = join_two_ext_data(ref_test_vectors['single_moving_same_dir_front'],
                                                                  ref_test_vectors['single_moving_out_of_fov'])

    ref_test_vectors['two_out_of_fov'] = join_two_ext_data(ref_test_vectors['single_moving_out_of_fov'],
                                                           ref_test_vectors['single_stationary_out_of_fov'])

    ref_test_vectors['two_out_of_fov_one_entering'] = join_two_ext_data(ref_test_vectors['two_out_of_fov'],
                                                                        ref_test_vectors['single_entering_fov'])

    ref_test_vectors['two_stationary_stopped'] = join_two_ext_data(ref_test_vectors['single_stationary_front'],
                                                                   ref_test_vectors['single_stopped_back'])

    ref_test_vectors['four_in_fov'] = join_two_ext_data(ref_test_vectors['two_moving_in_fov'],
                                                        ref_test_vectors['two_stationary_stopped'])

    ref_test_vectors['seven_different'] = join_two_ext_data(ref_test_vectors['four_in_fov'],
                                                            ref_test_vectors['two_out_of_fov_one_entering'])

    return ref_test_vectors
