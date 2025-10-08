import pytest
import numpy as np
from aspe.utilities.SupportingFunctions import map_array_values


def test_map_ints_to_ints():
    input = np.array([1, 2, 3, 2, 1])
    mapper = {1: 10, 2: 20, 3: 30}
    default = 0
    output = map_array_values(input, mapper, default)

    expected = np.array([10, 20, 30, 20, 10])
    assert output.dtype.kind == 'i'  # integer
    np.testing.assert_array_equal(output, expected)


def test_map_ints_to_ints_with_defaults():
    input = np.array([10, 1, 2, 3, 2, 1, 100])
    mapper = {1: 10, 2: 20, 3: 30}
    default = 0
    output = map_array_values(input, mapper, default)

    expected = np.array([0, 10, 20, 30, 20, 10, 0])
    assert output.dtype.kind == 'i'  # integer
    np.testing.assert_array_equal(output, expected)


def test_map_ints_to_str():
    input = np.array([1, 2, 3, 2, 1])
    mapper = {1: 'one', 2: 'two', 3: 'three'}
    default = 'default'
    output = map_array_values(input, mapper, default)

    expected = np.array(['one', 'two', 'three', 'two', 'one'])
    assert output.dtype.kind == 'U'  # unicode string
    np.testing.assert_array_equal(output, expected)


def test_map_ints_to_str_with_defaults():
    input = np.array([5, 1, 2, 3, 2, 1, 4])
    mapper = {1: 'one', 2: 'two', 3: 'three'}
    default = 'default'
    output = map_array_values(input, mapper, default)

    expected = np.array(['default', 'one', 'two', 'three', 'two', 'one', 'default'])
    assert output.dtype.kind == 'U'  # unicode string
    np.testing.assert_array_equal(output, expected)


def test_map_str_to_str():
    input = np.array(['one', 'two', 'three', 'two', 'one'])
    mapper = {'one': 'odd', 'two': 'even', 'three': 'odd'}
    default = 'not defined'
    output = map_array_values(input, mapper, default)

    expected = np.array(['odd', 'even', 'odd', 'even', 'odd'])
    assert output.dtype.kind == 'U'  # unicode string
    np.testing.assert_array_equal(output, expected)


def test_map_str_to_str_with_defaults():
    input = np.array(['owt', 'one', 'two', 'three', 'two', 'one', 'eno'])
    mapper = {'one': 'odd', 'two': 'even', 'three': 'odd'}
    default = 'not defined'
    output = map_array_values(input, mapper, default)

    expected = np.array(['not defined', 'odd', 'even', 'odd', 'even', 'odd', 'not defined'])
    assert output.dtype.kind == 'U'  # unicode string
    np.testing.assert_array_equal(output, expected)


def test_negative_mixed_input_types():
    '''
    For mixed numeric types with strings there will be non-explicit casting int to str, for all values, so mapping dict
    will not change anything - output will contain only default value - which is set to be string 'default'
    '''
    input = np.array([1, 2, 3, 'four'])
    mapper = {1: 'one', 2: 'two', 3: 'three'}
    default = 'default'
    output = map_array_values(input, mapper, default)

    expected = np.array(['default', 'default', 'default', 'default'])
    assert output.dtype.kind == 'U'  # unicode string
    np.testing.assert_array_equal(output, expected)


def test_object_input_types():
    '''
    For mixed numeric types with class instances input array dtype will be object.
    '''

    class Dummy:
        pass

    input = np.array([1, 2, 3, Dummy()])
    mapper = {1: 'one', 2: 'two', 3: 'three'}
    default = 'default'
    output = map_array_values(input, mapper, default)

    expected = np.array(['one', 'two', 'three', 'default'])
    assert output.dtype.kind == 'U'  # unicode string
    np.testing.assert_array_equal(output, expected)
