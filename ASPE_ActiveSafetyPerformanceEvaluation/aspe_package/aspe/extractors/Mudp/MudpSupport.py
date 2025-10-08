# coding=utf-8
"""
Set of functions for mudp extraction support
"""

from pathlib import PureWindowsPath


def get_all_stream_def_names(parsed_data) -> set:
    """
    Walking through parsed_data and finding all stream definition names
    :return: list if stream definition names
    """
    str_def_name_list = []
    for _, str_def_name in parsed_data['header_data'].items():
        str_def_abspath = PureWindowsPath(str_def_name.configuration_file_path)
        str_def_path, str_def_file = str_def_abspath.parent, str_def_abspath.name
        str_def_name, str_def_extension = str_def_file.split('.')
        str_def_name_list.append(str_def_name)

    return set(str_def_name_list)
