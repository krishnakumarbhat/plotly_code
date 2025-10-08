import argparse


def get_input_args() -> argparse.Namespace:
    """
    Function allows to get input arguments when running script from command line.

    @return: namespace consisting of command line input arguments
    """
    parser = argparse.ArgumentParser(
        prog='Log to .keg converter',
        description="""
                    Log to .keg converter. 
                    Script allows to prepare .keg file which can be used as input for Component Resim.
                    Output files are based on different log types: 
                        - .dvl and .mudp
                        - .xtrk 
                    When .xtrk file is given as an input. Then data is parsed directly from log (without resimulation).
                    When .dvl/.mudp file path is given as input, script runs F360 Resim and saves output in .xtrk file.
                    Finally, data stored in .xtrk files are extracted and saved in .keg file. 
                    """,
        epilog='author: Pawel Knap (nj5tl5)')
    parser.add_argument('-p', '--log_path', required=True, type=str, help='Path to .dvl/.mudp or .xtrk log file.')
    parser.add_argument('-m', '--mudp_stream_def_path', required=False, default="",
                        help='Path to mudp stream definition folder in case of direct .mudp to.keg conversion')
    parser.add_argument('-r', '--resim_path', required=False, default='', type=str, help='Path to resim destination.')
    parser.add_argument('-o', '--output_suffix', required=False, default='converter_output',
                        help='Suffix added to output filename.')
    parser.add_argument('--bmw_stream', required=False, default=False,
                        help='RESIM OPTION. If set to True, adds BMW streams to resim command.')
    parser.add_argument('--init_from_log', required=False, default=False,
                        help='RESIM OPTION. If set to True, adds init from log option to resim command.')
    parser.add_argument('--ini_path', required=False, default=None, help='RESIM OPTION. Path to F360 Tracker ini file.')
    parser.add_argument('--sync_input', required=False, default=True,
                        help='RESIM OPTION. If set to True, adds input sync to resim command.')

    arguments = parser.parse_args()
    return arguments


if __name__ == '__main__':
    args = get_input_args()
    print(args.log_path)
    print(args.output_filename)
