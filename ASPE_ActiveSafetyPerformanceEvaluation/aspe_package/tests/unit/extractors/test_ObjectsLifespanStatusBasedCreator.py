# coding=utf-8
"""
Unittests for ObjectLifespanStatusBasedCreator functions
"""
import numpy as np
import pandas as pd
from pandas.testing import assert_frame_equal

from aspe.utilities.objects_lifespan_status_based_creator import ObjectLifespanStatusBasedCreator


class TestCreateFirstInstanceMask:

    def test_common_case(self):
        status = np.array([[0, 0],
                          [3, 0],
                          [4, 3],
                          [4, 4],
                          [5, 0],
                          [0, 0],
                          [3, 0],
                          [4, 0],
                          [0, 0]])
        status_new_value = 3
        status_invalid_value = 0
        output = pd.DataFrame({'first_row': [1, 6, 2], 'column': [0, 0, 1]})
        assert_frame_equal(ObjectLifespanStatusBasedCreator._create_first_obj_instance_mask(status, status_new_value,
                                                                                            status_invalid_value),
                           output, check_dtype=False)

    def test_init_at_beggining(self):
        status = np.array([[3, 4],
                          [4, 4],
                          [5, 4],
                          [0, 4],
                          [0, 5],
                          [0, 0]])
        status_new_value = 3
        status_invalid_value = 0
        output = pd.DataFrame({'first_row': [0, 0], 'column': [0, 1]})
        assert_frame_equal(ObjectLifespanStatusBasedCreator._create_first_obj_instance_mask(status, status_new_value,
                                                                                            status_invalid_value),
                           output, check_dtype=False)

    def test_single_instance_lifetime(self):
        status = np.array([[3, 0, 0, 3],
                          [0, 0, 3, 3],
                          [0, 0, 3, 3],
                          [0, 3, 0, 3]])
        status_new_value = 3
        status_invalid_value = 0
        output = pd.DataFrame({'first_row': [0, 3, 1, 2, 0, 1, 2, 3], 'column': [0, 1, 2, 2, 3, 3, 3, 3]})
        assert_frame_equal(ObjectLifespanStatusBasedCreator._create_first_obj_instance_mask(status, status_new_value,
                                                                                            status_invalid_value),
                           output, check_dtype=False)

    def test_drops_in_new_status(self):
        status = np.array([[4, 0, 0, 0, 4],
                           [4, 4, 0, 0, 4],
                           [0, 4, 4, 0, 3],
                           [0, 3, 4, 4, 4]])
        status_new_value = 3
        status_invalid_value = 0
        output = pd.DataFrame({'first_row': [0, 1, 3, 2, 3, 0, 2], 'column': [0, 1, 1, 2, 3, 4, 4]})
        result = ObjectLifespanStatusBasedCreator._create_first_obj_instance_mask(status, status_new_value,
                                                                                  status_invalid_value)
        assert_frame_equal(result, output, check_dtype=False)


class TestCreateLastInstanceMask:

    def test_common_case(self):
        status = np.array([[0, 0],
                          [3, 0],
                          [4, 3],
                          [4, 4],
                          [5, 0],
                          [0, 0],
                          [3, 0],
                          [4, 0],
                          [0, 0]])
        status_new_value = 3
        status_invalid_value = 0
        output = pd.DataFrame({'last_row': [4, 7, 3], 'column': [0, 0, 1]})
        assert_frame_equal(ObjectLifespanStatusBasedCreator._create_last_obj_instance_mask(status, status_new_value,
                                                                                           status_invalid_value),
                           output, check_dtype=False)

    def test_killed_at_end(self):
        status = np.array([[0, 3],
                          [0, 4],
                          [0, 4],
                          [3, 4],
                          [4, 4],
                          [4, 5]])
        status_new_value = 3
        status_invalid_value = 0
        output = pd.DataFrame({'last_row': [5, 5], 'column': [0, 1]})
        assert_frame_equal(ObjectLifespanStatusBasedCreator._create_last_obj_instance_mask(status, status_new_value,
                                                                                           status_invalid_value),
                           output, check_dtype=False)

    def test_single_instance_lifetime(self):
        status = np.array([[3, 0, 0, 3],
                          [0, 0, 3, 3],
                          [0, 0, 3, 3],
                          [0, 3, 0, 3]])
        status_new_value = 3
        status_invalid_value = 0
        output = pd.DataFrame({'last_row': [0, 3, 1, 2, 0, 1, 2, 3], 'column': [0, 1, 2, 2, 3, 3, 3, 3]})
        assert_frame_equal(ObjectLifespanStatusBasedCreator._create_last_obj_instance_mask(status, status_new_value,
                                                                                           status_invalid_value),
                           output, check_dtype=False)
