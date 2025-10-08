import glob
import logging
import os
import subprocess


def run_resim(resim_exe_path: str, log_path: str, bmw_stream=False, ini_file=None, init_from_log=True, sync_input=False,
              save_xtrk=True, add_suffix=False, suffix='', add_my_dir=False, my_dir='') -> str:
    """
    Runs F360 Resim in subprocess.

    @param init_from_log: flag, adds init_from_log option to Resim command
    @param resim_exe_path: path to F360 Resim .exe file.
    @param log_path: path to .dvl file which is going to be resimulated
    @param bmw_stream: flag, adds bmw_stream option to Resim command
    @param ini_file: path to F360 Tracker ini file
    @param sync_input: flag, adds syn_input option to Resim command
    @param save_xtrk: flag, adds xtrklog option to Resim command
    @param add_suffix: flag, adds suffix to Resim command
    @param suffix: string, that consists of suffix that will be added
    @param add_my_dir:flag, adds my_dir to Resim command
    @param my_dir: string, that consists of my dir name
    """
    logging.info('Running F360 Core Resim.')
    command = generate_resim_command(resim_exe_path, log_path, bmw_stream, ini_file, init_from_log, sync_input,
                                     save_xtrk, add_suffix, suffix, add_my_dir, my_dir)
    logging.info('Resim command: %s', command)
    process_output = subprocess.run(command)
    if process_output.returncode != 0:
        err = RuntimeError('Resim execution error.')
        logging.error(err, stack_info=True)
        raise err

    new_log_path = os.path.join(os.getcwd(), glob.glob(my_dir + '/*' + suffix + '.dvl')[0])
    new_log_path = os.path.splitext(new_log_path)[0]
    return new_log_path


def generate_resim_command(resim_exe_path: str, log_path: str, bmw_stream: bool, ini_file: str, init_from_log: bool,
                           sync_input: bool, save_xtrk: bool, add_suffix: bool, suffix: str, add_my_dir: bool,
                           my_dir: str) -> str:
    command = f'{resim_exe_path} {log_path} '

    if bmw_stream:
        command += ' -stream BMW'

    if add_suffix:
        command += ' -osuffix ' + suffix

    if add_my_dir:
        command += ' -mydir ' + my_dir

    if init_from_log:
        init_command = '-init_from_log'
    elif ini_file is not None:
        init_command = f'-ini {ini_file}'
    else:
        init_command = ''

    sync_command = '-sync_input' if sync_input else ''

    xtrk_command = '-xtrklog' if save_xtrk else ''

    command += f' -f360trkopt {init_command} {sync_command} {xtrk_command} -endopt'

    return command


if __name__ == '__main__':
    run_resim(
        log_path=r"C:\Users\nj5tl5\Desktop\log\DFT-1906\resim_test\20220519T073433_20220519T073453_548354_9H75684_SRR_DEBUG.mudp",
        resim_exe_path=r"C:\Users\nj5tl5\builds\OT_build\output\Debug\resim_f360.exe",
        bmw_stream=True)
