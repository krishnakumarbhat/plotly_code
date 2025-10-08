from typing import Optional, Set

import numpy as np

from aspe.utilities.SupportingFunctions import get_from_dict_by_map, get_nested_dict_keys


def assert_nested_dict_equal(nested_dict_before: dict, nested_dict_after: dict) -> None:
    """
    Checks equivalence of two nested dictionaries.
    Raises AssertionError when dictionary keys or values differ.
    :param nested_dict_before: dict
    :param nested_dict_after: dict
    :return:
    """
    keys_before = set(get_nested_dict_keys(nested_dict_before))
    keys_after = set(get_nested_dict_keys(nested_dict_after))

    if keys_before != keys_after:
        diff = keys_before.symmetric_difference(keys_after)
        raise AssertionError(f'Signals {", ".join(map(".".join, diff))} not found in corresponding dicts')

    diff = []
    for path in keys_before:
        signal_values_before = get_from_dict_by_map(nested_dict_before, path)
        signal_values_after = get_from_dict_by_map(nested_dict_after, path)

        if not np.array_equal(signal_values_before, signal_values_after):
            diff.append(path[-1])

    if diff:
        raise AssertionError(f'Signals {", ".join(diff)} have different values')


def assert_mudp_parsed_data_equal(parsed_data_before: dict, parsed_data_after: dict, *,
                                  streams_to_compare: Optional[Set[int]] = None) -> None:
    """
    Checks equivalence of data from selected streams between two instances of parsed data.
    Raises AssertionError when streams are missing, signals are missing or signal values are not equal.
    :param parsed_data_before: parsed data dictionary
    :param parsed_data_after: parsed data dictionary
    :param streams_to_compare: set of streams to compare
    :return:
    """
    streams_before = parsed_data_before['parsed_data'].keys()
    streams_after = parsed_data_after['parsed_data'].keys()

    if streams_to_compare is None:
        streams_to_compare = streams_before | streams_after

    available_streams = streams_before & streams_after
    missing_streams = streams_to_compare - available_streams
    if missing_streams:
        raise AssertionError(f'Streams {", ".join(map(str, sorted(missing_streams)))} are missing in parsed data')

    for stream in streams_to_compare:
        stream_data_before = parsed_data_before['parsed_data'][stream]
        stream_data_after = parsed_data_after['parsed_data'][stream]

        assert_nested_dict_equal(stream_data_before, stream_data_after)


