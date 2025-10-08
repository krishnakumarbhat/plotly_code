import os
import pathlib
from typing import List


def build_pe_input_list(log_list: List[str], estimated_data_ext: str, reference_data_ext: str):
    output_paths = []
    for log_path in log_list:
        est_data_path, ref_data_path = get_single_log_pe_input_paths(log_path, estimated_data_ext, reference_data_ext)

        if os.path.exists(est_data_path) and os.path.exists(ref_data_path):
            output_paths.append((est_data_path, ref_data_path))

    return output_paths


def get_single_log_pe_input_paths(log_path: str, estimated_data_ext: str, reference_data_ext: str):
    log_path_without_ext = os.path.splitext(log_path)[0]
    estimated_data_path = log_path_without_ext + estimated_data_ext
    reference_data_path = log_path_without_ext + reference_data_ext
    return estimated_data_path, reference_data_path


def get_lines_from_txt(txt_path: str):
    """
    Read .txt file with multiple lines and return list of strings.
    :param txt_path: path to .txt file
    :return: list of single line strings
    """
    with open(txt_path, 'r') as file:
        lines = file.read().splitlines()
    return lines


def get_log_list_from_path(log_general_path,
                           req_ext='.dvl',
                           required_sub_strings=None,
                           restricted_sub_strings=None,
                           level=0):
    log_list = []
    for local_path, dirs, files in walk_level(log_general_path, level=level):
        for file_name in files:
            file_extension = os.path.splitext(file_name)[1]
            if file_extension.casefold() == req_ext.casefold():

                f_in_is_correct_fn = True
                f_out_is_correct_fn = True

                if required_sub_strings is not None:
                    for single_sub_string in required_sub_strings:
                        if single_sub_string not in file_name and single_sub_string not in local_path:
                            f_in_is_correct_fn = False
                            break

                if restricted_sub_strings is not None:
                    for single_sub_string in restricted_sub_strings:
                        if single_sub_string in file_name or single_sub_string in local_path:
                            f_out_is_correct_fn = False
                            break

                if f_in_is_correct_fn and f_out_is_correct_fn:
                    path = os.path.join(local_path, file_name)
                    log_list.append(path)

    return log_list


def walk_level(some_dir, level=1):
    some_dir = some_dir.rstrip(os.path.sep)
    assert os.path.isdir(some_dir)
    num_sep = some_dir.count(os.path.sep)
    for root, dirs, files in os.walk(some_dir):
        yield root, dirs, files
        num_sep_this = root.count(os.path.sep)
        if num_sep + level <= num_sep_this:
            del dirs[:]


def get_single_mdf_log_pe_input_paths(path, ref_dir='SRR_REFERENCE'):
    path = pathlib.Path(path)
    path_to_dir = path.parents[1]
    file_name = path.name
    ref_name = file_name.replace('fas', 'ref').replace('BN_FASETH', 'SRR_REFERENCE')
    if ref_name in os.listdir(os.path.join(path_to_dir, ref_dir)):
        return str(path), os.path.join(path_to_dir, ref_dir, ref_name)
    else:
        print(f'Cannot find {ref_name} in {os.path.join(path_to_dir, ref_dir)}')
        return None
