import subprocess
from typing import List


class F360ResimRunner:
    def __init__(self, resim_exe_path: str):
        """
        Simple class for f360 resim .exe running. It wraps windows shell call with arguments. Current version support
        only single argument - -xtrklog. TODO handle more arguments
        :param resim_exe_path: path to .exe file with f360 resim
        :param f_xtrk_out: flag to indicate if .xtrk file will be created
        """
        self._resim_exe_path = resim_exe_path

    def resim_single_log(self, dvl_log_path: str, ini_file_path: str, arguments='', trk_opts=''):
        """
        Resimulate single log
        :param dvl_log_path: path to log .dvl file
        :param ini_file_path: path to .ini file
        :return:
        """
        print(f'Processing {dvl_log_path} log ... \n')
        command = f'{self._resim_exe_path} {dvl_log_path} {arguments} -dvl -f360trkopt -ini {ini_file_path} {trk_opts} -endopt'
        subprocess.call(command)

    def resim_log_list(self, dvl_log_list: List[str], ini_file_path: str, arguments='', trk_opts=''):
        """
        Resimulate files stored in list of logs with given ini file
        :param dvl_log_list: list of log paths to be resimulated
        :param ini_file_path: path to ini file
        :return:
        """
        for dvl_log_path in dvl_log_list:
            self.resim_single_log(dvl_log_path, ini_file_path, arguments, trk_opts)


class TxtReader:
    """
    Helper function for reading text files. TODO: should be probably moved to some ASPE utilities folder
    """
    @staticmethod
    def get_multi_lines(txt_path):
        """
        Read .txt file with multiple lines and return list of strings.
        :param txt_path: path to .txt file
        :return: list of single line strings
        """
        with open(txt_path, 'r') as file:
            lines = file.read().splitlines()
        return lines

    @staticmethod
    def get_single_line(txt_path):
        """
        Read .txt file with single line and return this line as string.
        :param txt_path: path to .txt file
        :return: string with line
        """
        with open(txt_path, 'r') as file:
            line = file.read()
        return line


if __name__ == '__main__':
    """
    Example of usage. change resim_exe_path to your local .exe resim file.
    """
    log_paths = TxtReader.get_multi_lines(r"\\10.224.186.68\AD-Shared\F360\Tools\Resim\Private\JWr\ASPE\LSS_CURVES"
                                          r"\resim_runner_test.txt")
    ini_file = r"\\10.224.186.68\AD-Shared\F360\Tools\Resim\Private\JWr\ASPE\LSS_CURVES\F360Tracker_RNA_SUV_Scenic.ini"
    resim_exe_path = r"\\10.224.186.68\AD-Shared\F360\Tools\Resim\Standalone\F360Core_2_08_0\resim_f360.exe"
    runner = F360ResimRunner(resim_exe_path)
    runner.resim_log_list(log_paths, ini_file)