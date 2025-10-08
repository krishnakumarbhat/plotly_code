import argparse
import logging
import os
import sys

from aspe.utilities.log_to_keg_converter.args_parser import get_input_args
from aspe.utilities.log_to_keg_converter.ConvertMudpToKeg import ConvertMudpToKeg
from aspe.utilities.log_to_keg_converter.ConvertXtrkToKeg import ConvertXtrkToKeg
from aspe.utilities.log_to_keg_converter.resim import run_resim

"""
Log to .keg converter. 
Script allows to prepare .keg file which can be used as input for Component Resim. 
Output files are based on 
different log types: 
    - .dvl,
    - .mudp
    - .xtrk 

When .xtrk 
file is given as an input. 
Then data is parsed directly from log (without resimulation).

When .dvl or .mudp 
file is given as input and 
--resim_path argument has been passed, 
then  script runs F360 Resim. 
When --mudp_stream_def_path is also given 
data stored in resimulated .mudp file are converted to .keg file

When --mudp_stream_def_path is not defined than .xtrk data are directly converted to .keg file.

When .mudp file path is given as input,
-resim_path argument is not given and direct_mudp_conversion argument is defined,
then mudp data are converted directly to .keg file. 
(Resim argument is not needed).
"""

supported_resim_extension = ['.mudp', '.dvl']
supported_direct_conversion = '.mudp'
suported_xtrk_conversion = '.xtrk'

xtrk_suffix = '_from_Xtrk'
mudp_suffix = '_from_Mudp'


def main():
    set_logger()
    args = get_input_args()
    run(args)


def run(args):
    pathname, extension = get_pathname_and_extension(args.log_path)

    if extension == '.xtrk':
        run_xtrk_to_keg_conversion(pathname, args.log_path, args.output_suffix, xtrk_suffix)

    elif extension in supported_resim_extension:
        if args.resim_path:
            # run resim, generate files
            log_path_without_extension = run_resim(resim_exe_path=args.resim_path,
                                                   log_path=args.log_path,
                                                   bmw_stream=args.bmw_stream,
                                                   ini_file=args.ini_path,
                                                   init_from_log=args.init_from_log,
                                                   sync_input=args.sync_input,
                                                   save_xtrk=True,
                                                   add_suffix=True,
                                                   suffix='_temp',
                                                   add_my_dir=True,
                                                   my_dir='temp')

            if args.mudp_stream_def_path:
                run_mudp_to_keg_conversion(pathname, log_path_without_extension + extension, args.output_suffix,
                                           mudp_suffix,
                                           args.mudp_stream_def_path)
            else:
                run_xtrk_to_keg_conversion(pathname, log_path_without_extension + suported_xtrk_conversion,
                                           args.output_suffix, xtrk_suffix)
        else:
            if extension == supported_direct_conversion:
                run_mudp_to_keg_conversion(pathname, args.log_path, args.output_suffix, mudp_suffix,
                                           args.mudp_stream_def_path)
            else:
                err = RuntimeError(
                    f"Not supported file extension for direct .mudp conversion [SUPPORTED EXTENSIONS: "
                    f"{supported_direct_conversion}] or pass correct --resim_path argument")
                logging.error(err, stack_info=True)
                raise err
    else:
        err = RuntimeError(
            f"Not supported file extension,  pass {supported_resim_extension} or .xtrk file]")
        logging.error(err, stack_info=True)
        raise err


def run_xtrk_to_keg_conversion(keg_pathname, xtrk_pathname, output_suffix, xtrk_suffix):
    file_path = create_keg_file_path(keg_pathname, output_suffix + xtrk_suffix)
    obj = ConvertXtrkToKeg(xtrk_pathname, file_path)
    obj.save_to_keg_file()


def run_mudp_to_keg_conversion(keg_pathname, mudp_pathname, output_suffix, mudp_suffix, mudp_stream_def_path):
    file_path = create_keg_file_path(keg_pathname, output_suffix + mudp_suffix)
    obj = ConvertMudpToKeg(mudp_pathname, file_path, mudp_stream_def_path)
    obj.save_to_keg_file()


def get_pathname_and_extension(path: str) -> tuple:
    return os.path.splitext(path)


def create_keg_file_path(pathname: str, output_suffix: str) -> str:
    return pathname + '_' + output_suffix + '.keg'


def set_logger():
    logging.basicConfig(level=logging.INFO,
                        stream=sys.stdout,
                        format='%(levelname)s | %(message)s')
    logging.captureWarnings(True)


if __name__ == '__main__':
    main()
