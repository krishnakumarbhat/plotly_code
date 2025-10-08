import numpy as np
import warnings

from future.tests.base import assertWarns

from aspe.utilities.SupportingFunctions import get_unique_value_with_most_counts


class TestGetUniqueValueWithMostCounts:
    def test_for_one_unique_value(self):
        unique_value = 3
        array_shape = [1, 10]
        in_array = np.full(array_shape, unique_value)
        out = get_unique_value_with_most_counts(in_array)

        np.testing.assert_array_equal(out, unique_value)

    def test_for_one_unique_value_warning_negative(self):
        unique_value = 3
        array_shape = [1, 10]
        in_array = np.full(array_shape, unique_value)
        with warnings.catch_warnings(record=True) as warn_list:
            get_unique_value_with_most_counts(in_array)
            assert not warn_list  # Expected empty array of warnings

    def test_for_one_not_unique_value(self):
        unique_value = 3
        not_unique_value = 7
        array_shape = [1, 10]
        in_array = np.full(array_shape, unique_value)
        in_array[0, 0] = not_unique_value

        out = get_unique_value_with_most_counts(in_array)

        np.testing.assert_array_equal(out, unique_value)

    def test_for_one_not_unique_value_warning_positive(self):
        unique_value = 3
        not_unique_value = 7
        array_shape = [1, 10]
        in_array = np.full(array_shape, unique_value)
        in_array[0, 0] = not_unique_value

        expected_warning = "UserWarning: several unique (2) values for property  - value with the most counts will be used warn(message)"
        assertWarns(expected_warning, get_unique_value_with_most_counts(in_array))

    def test_for_many_not_unique_value(self):
        unique_value = 3
        unique_values_num = 20
        not_unique_values_num = 4

        unique_array_shape = [unique_values_num, 1]
        not_unique_values = np.random.randint(unique_value + 1, unique_value + 100, [not_unique_values_num, 1])
        in_array = np.full(unique_array_shape, unique_value)
        in_array[np.random.choice(unique_values_num, not_unique_values_num)] = not_unique_values

        out = get_unique_value_with_most_counts(in_array)

        np.testing.assert_array_equal(out, unique_value)

    def test_for_the_same_counts_symmetric(self):
        unique_value_1 = 3
        unique_value_2 = 5
        unique_values_num = 20
        array_shape = [unique_values_num, 1]

        in_array_1 = np.full(array_shape, unique_value_1)
        in_array_2 = np.full(array_shape, unique_value_2)
        in_array = [in_array_1, in_array_2]

        out = get_unique_value_with_most_counts(in_array)

        np.testing.assert_array_equal(out, unique_value_1)

    def test_for_the_same_counts_non_symmetric(self):
        in_array = [5, 2, 5, 5, 2, 2]

        out = get_unique_value_with_most_counts(in_array)

        np.testing.assert_array_equal(out, 2)

    def test_for_one_count_of_each_value(self):
        in_array = np.linspace(0, 50, 51, dtype=int)

        out = get_unique_value_with_most_counts(in_array)

        np.testing.assert_array_equal(out, 0)
