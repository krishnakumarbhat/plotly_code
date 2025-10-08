from mdf_parser_pkg.mdf_parser import parse_file
from srr5_dev_tools.mgp_module import load as load_mgp

from aspe.extractors.F360.MDF4_BMW_mid.F360Mdf4BmwExtractor import F360Mdf4BmwExtractor

if __name__ == '__main__':
    f_load_from_mgp = True  # choose whether you want to parse mf4 file directly or load parsed data from .mgp file

    if f_load_from_mgp:
        data_path = (r"\\10.224.186.68\AD-Shared\ASPE\Logs\F360\mf4_example_files\parse_result"
                     r"\20200128T155737_20200128T155757_543078_LB36408_BN_FASETH_SRR_Master-mPAD.mgp")
        parsed_data = load_mgp(data_path)
    else:
        data_path = (r"\\10.224.186.68\AD-Shared\ASPE\Logs\F360\mf4_example_files"
                     r"\20200128T155757_20200128T155817_543078_LB36408_BN_FASETH.MF4")
        parsed_data = parse_file(data_path, 'mid')

    extractor = F360Mdf4BmwExtractor()
    extracted_data = extractor.extract_data(parsed_data)