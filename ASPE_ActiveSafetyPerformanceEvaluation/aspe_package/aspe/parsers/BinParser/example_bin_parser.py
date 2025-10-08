import os

import scipy.io

from aspe.parsers.BinParser.bin_parser import BinParser


def example_read_single_bin(path, file_name, save_path):
    """

    :param path:
    :param file_name:
    :param save_path:
    :return:
    """
    decoded_bin_file = BinParser()
    parsed = decoded_bin_file.parse(path + '\\' + file_name)
    mat_name = file_name.replace('.bin', '.mat')
    scipy.io.savemat(save_path + '\\' + mat_name, parsed, oned_as='column', long_field_names=True)


def example_read_all_bins(path, save_path):
    """

    :param path:
    :param save_path:
    :return:
    """
    bins = []
    for root, directory, files in os.walk(path):
        for file in files:
            if '.bin' in file:
                bins.append(os.path.join(root, file))
    for bin_file in bins:
        decoded_bin_file = BinParser()
        parsed = decoded_bin_file.parse(bin_file)
        out_file_name = bin_file.split('\\')[-1]
        out_file_name = out_file_name.strip('.bin')
        scipy.io.savemat(save_path + '\\' + out_file_name + '.mat', parsed, oned_as='column', long_field_names=True)


if __name__ == '__main__':
    path = r"\\10.224.186.68\AD-Shared\ASPE\Logs\GDSR\test_log_bin_parser\REAR_LEFT_bin"
    file_name = r"\RNASUV_SRR5_DP790YM_20191120_420_MOS_KPI_LSS_035_TrackerClutterRegion.bin"
    path_bins = r"\\10.224.186.68\AD-Shared\ASPE\Logs\GDSR\test_log_bin_parser\REAR_LEFT_bin"
    save_path = r"\\10.224.186.68\AD-Shared\ASPE\Logs\GDSR\test_log_bin_parser\parser_output"

    example_read_single_bin(path, file_name, save_path)
    example_read_all_bins(path_bins, save_path)
