import os
import traceback
from pathlib import Path
from time import sleep
from typing import Dict, List, Optional, Union

import tqdm
from mdf_parser_pkg import mdf_parser
from srr5_dev_tools import mgp_module

from aspe.extractors.F360.DataSets.F360Mf4BmwExtractedData import F360Mf4BmwExtractedData
from aspe.extractors.F360.MDF4_BMW_mid.F360Mdf4BmwExtractor import F360Mdf4BmwExtractor
from aspe.extractors.Interfaces import ExtractedData
from aspe.extractors.ReferenceExtractor.RtRangeMdfExtractor import RtRangeMdfExtractor
from aspe.utilities.SupportingFunctions import get_log_list_from_path, load_from_pkl, save_to_pkl


def extract_rt_range_3000_from_mf4(
        mf4_log_path: str,
        hunter_length: float,
        hunter_width: float,
        save_to_file: bool = True,
        save_mgp_parsed_data: bool = True,
        raw_signals: bool = False,
        force_parse: bool = False,
        hunter_target_instances_shift: Optional[int] = None,
        hunter_rear_axle_to_front_bumper_dist: Optional[float] = None,
        parse_cache_path: Path = None,
) -> ExtractedData:
    """
    Extract RtRange3000 data from MDF type file.
    @param mf4_log_path: absolute path to .mf4 log file
    @param hunter_width: width of the host vehicle
    @param hunter_length: length of the host vehicle
    @param save_to_file: flag indicating if extracted output should be saved as .pickle file. If is True .pickle
    file named log_name_rt_range_3000_mdf_extracted.pickle will be created within same folder.
    @param save_mgp_parsed_data: flag indicating if parsed data should be save as .mgp file. If True than .mgp file
    will be created inside parse_result folder.
    @param raw_signals: flag indicating if raw signals should be extracted. Raw signals have same signatures as in
    stream definition - there is no translation applied. If yes extracted raw signals are placed in separate DataFrame
    within data set.
    @param force_parse: if True - parsing is done no matter if .mgp file already exists
    @param hunter_target_instances_shift: hunter and target data are not in sync, for aligning it UTC timestamp is used,
    but in some cases this signal is not present - in that cases move target frames by given value
    @param hunter_rear_axle_to_front_bumper_dist: distance from host rear axle to front bumper - needed for coordinate
    system transformations
    @param parse_cache_path:
    @return ExtractedData
    """

    # only parse file if there is no previously parsed file
    mf4_log_path = Path(mf4_log_path)
    if parse_cache_path is None:
        parse_cache_path = mf4_log_path.parents[1] / 'parse_result' / mf4_log_path.parent.name
        parse_cache_path.mkdir(parents=True, exist_ok=True)

    mgp_path = parse_cache_path / f'{mf4_log_path.stem}_RT_Range.mgp'
    if not mgp_path.exists() or force_parse:  # check if glob found any mgp files related to log file
        print(f'Parsing data ...')
        if save_mgp_parsed_data:
            result_path = mgp_path
        else:
            result_path = None
        parsed_file = mdf_parser.parse_file(mf4_log_path,
                                            config='mid',
                                            translation_db='A370',
                                            bus_id=917515,
                                            result_path=result_path)
    else:
        print('Found .mgp file with parsed data. Loading it instead of parsing...')
        parsed_file = mgp_module.load(mgp_path)
    extractor = RtRangeMdfExtractor(f_extract_raw_signals=raw_signals,
                                    f_completed_messages_only=False,
                                    hunter_target_instances_shift=hunter_target_instances_shift,
                                    hunter_length=hunter_length,
                                    hunter_width=hunter_width,
                                    hunter_rear_axle_to_front_bumper_dist=hunter_rear_axle_to_front_bumper_dist)
    extracted = extractor.extract_data(parsed_file)
    if save_to_file:
        pickle_save_path = mf4_log_path.parent / f'{mf4_log_path.stem}_rt_range_3000_mdf_extracted.pickle'
        save_to_pkl(extracted, str(pickle_save_path))
    return extracted


def extract_f360_bmw_mid_from_mf4(
        mf4_log_path: str,
        sw_version,
        save_to_file: bool = False,
        save_mgp_parsed_data: bool = True,
        raw_signals: bool = False,
        force_parse: bool = False,
        force_extract: bool = False,
        host_rear_axle_to_front_bumper_dist: Optional[float] = None,
        parse_cache_path: Path = None,
        extract_cache_path: Path = None,
) -> F360Mf4BmwExtractedData:
    """
    Extract BMW F360 tracker data from .mf4 log file containing someip data.

    @param mf4_log_path: absolute path to .mf4 log file
    @param sw_version: string indicating what sw version was used for recording data, in A-step form i.e. A390
    @param save_to_file: flag indicating if extracted output should be saved as .pickle file. If is True .pickle
           file named log_name_f360_mf4_bmw_mid_extracted.pickle will be created within same folder.
    @param save_mgp_parsed_data: flag indicating if parsed data should be save as .mgp file. If True than .mgp file
           will be created inside parse_result folder.
    @param raw_signals: flag indicating if raw signals should be extracted. Raw signals have same signatures as in
           stream definition - there is no translation applied. If yes extracted raw signals are placed in separate
           DataFrame within data set.
    @param force_parse: if True - parsing is done no matter if .mgp file already exists
    @param force_extract: by default this flag is set to False - try to find .pickle file with already extracted data
           and load it instead of extracting, if True - always do extraction step
    @param host_rear_axle_to_front_bumper_dist: distance between rear axle and front bumper of host vehicle, parameter
           needed for coordinate systems transformations
    @param parse_cache_path: directory with cached parsed data
    @param extract_cache_path: directory with cached extracted data
    @return extracted data

    Additional notes:
    Example data could be found:
    https://aspera-emea.aptiv.com/browse/shares/136?path=%2FASPE_Examples%2FBMW_MDF_example_logs#
    Function uses mdf parser developed by VT&V team in TCK. For running it, srr5_dev_tools package is also needed.
    Whl installation files for mdf parser and srr5_dev_tools are located inside aspe.extractors/external_whls folder.
    MDF parser repository: https://hpc-gitlab.aptiv.com/BMW_SRR5/mdf_parser_pkg (access needed).
    Srr5 dev tools repository: https://hpc-gitlab.aptiv.com/BMW_SRR5/srr5_dev_tools (access needed).
    """

    mf4_log_path = Path(mf4_log_path)
    print(f'Started processing MDF log file: {mf4_log_path.name}')
    if extract_cache_path is None:
        extract_cache_path = mf4_log_path.parents[1] / 'parse_result' / mf4_log_path.parent.name
    extract_cache_path.mkdir(exist_ok=True, parents=True)
    pickle_save_path = extract_cache_path / f'{mf4_log_path.stem}_f360_mf4_bmw_mid_extracted.pickle'
    if pickle_save_path.exists() and not force_extract:
        print('Found already extracted data in .pickle file. Loading instead of extracting ...')
        extracted = load_from_pkl(str(pickle_save_path))
    else:
        parsed = parse_faseth_data(str(mf4_log_path), sw_version, 'SRR_Master-mPAD', force_parse, save_mgp_parsed_data,
                                   parse_cache_path)
        if 'PTPMessages' in parsed['data']:  # resim case - PTP is within SRR_Master-mPAD bus
            ptp = parsed['data']['PTPMessages']['PTPMessages']
            parsed['data']['PTPMessages'] = ptp  # reduce unnecessary nesting
        else:
            ptp_parsed = parse_faseth_data(str(mf4_log_path), sw_version, 'mPAD-SRR_Master', force_parse,
                                           save_mgp_parsed_data, parse_cache_path)
            if ptp_parsed is not None and 'PTPMessages' in ptp_parsed[
                'data']:  # logged data case - PTP within mPAD-SRR_Master bus
                parsed['data']['PTPMessages'] = ptp_parsed['data']['PTPMessages'][
                    'PTPMessages']  # add ptp messages to some ip output
        if isinstance(parsed, dict) and 'data' in parsed:
            print(f'Extracting data ...')
            extractor = F360Mdf4BmwExtractor(f_extract_raw_signals=raw_signals,
                                             rear_axle_to_front_bumper_distance=host_rear_axle_to_front_bumper_dist)
            extracted = extractor.extract_data(parsed)
            if save_to_file:
                save_to_pkl(extracted, str(pickle_save_path))
                print(f'Saved output to {pickle_save_path}')
        else:
            raise ValueError('MDF parser returned unexpected data type. Ensure inputs, only BN_FASETH bus supported')
    return extracted


def extract_f360_bmw_mid_from_mf4_folder(folder_path: str,
                                         sw_version: str,
                                         save_to_file: bool = True,
                                         save_mgp_parsed_data: bool = True,
                                         raw_signals: bool = False,
                                         force_parse: bool = False,
                                         force_extract: bool = False,
                                         host_rear_axle_to_front_bumper_dist: Optional[float] = float,
                                         required_path_sub_strings: Union[str, List[str]] = None,
                                         restricted_path_sub_strings: Union[str, List[str]] = None,
                                         folder_search_level: int = 0) -> Dict[str, F360Mf4BmwExtractedData]:
    """
    Find .mf4 files in given folder and extract BMW F360 tracker data from it.
    :param folder_path: absolute path to folder with .mf4 log files
    :type folder_path: str
    :param sw_version: string indicating what sw version was used for recording data, in A-step form i.e. A390
    :type sw_version: str
    :param save_to_file: flag indicating if extracted output should be saved as .pickle file. If is True .pickle
    file named log_name_f360_mf4_bmw_mid_extracted.pickle will be created within same folder.
    :type save_to_file: bool
    :param save_mgp_parsed_data: flag indicating if parsed data should be save as .mgp file. If True than .mgp file
    will be created inside parse_result folder.
    :type save_mgp_parsed_data: bool
    :param raw_signals: flag indicating if raw signals should be extracted. Raw signals have same signatures as in
    stream definition - there is no translation applied. If True extracted raw signals are placed in separate DataFrame
    within data set.
    :type raw_signals: bool
    :param force_parse: if True - parsing is done no matter if .mgp file already exists
    :type force_parse: bool
    :param force_extract: by default this flag is set to False - try to find .pickle file with already extracted data
    and load it instead of extracting, if True - always do extraction step
    :param required_path_sub_strings: list of strings which are required in log path/name to be extracted, for example
    resim extension(s)
    :type required_path_sub_strings: list[str]
    :param restricted_path_sub_strings: list of strings which are restricted in log path/name - logs containing any of
    these strings will be NOT extracted
    :type restricted_path_sub_strings: list[str]
    :param folder_search_level: number of sub-folders levels which should be searched, for example - level 0 is only
    given folder - sub-folders will be not searched, level 1 - given folder and sub-folders within will be searched, and
    so on...
    :type folder_search_level: int
    :param host_rear_axle_to_front_bumper_dist: distance between rear axle and front bumper of host vehicle, parameter
    needed for coordinate systems transformations
    :type host_rear_axle_to_front_bumper_dist: float
    :return: dict containing extracted data from multiple logs, keys - log names, values - extracted data
    :return: Dict[str: ExtractedData]

    Additional notes:
    Example data could be found:
    https://aspera-emea.aptiv.com/browse/shares/136?path=%2FASPE_Examples%2FBMW_MDF_example_logs#
    Function uses mdf parser developed by VT&V team in TCK. For running it, srr5_dev_tools package is also needed.
    Whl installation files for mdf parser and srr5_dev_tools are located inside aspe.extractors/external_whls folder.
    MDF parser repository: https://hpc-gitlab.aptiv.com/BMW_SRR5/mdf_parser_pkg (access needed).
    Srr5 dev tools repository: https://hpc-gitlab.aptiv.com/BMW_SRR5/srr5_dev_tools (access needed).
    """
    output = {}
    log_list = get_log_list_from_path(folder_path, req_ext='.MF4', required_sub_strings=required_path_sub_strings,
                                      restricted_sub_strings=restricted_path_sub_strings, level=folder_search_level)
    # logged data files have .MF4 extension, but after resim files have .mf4 extension :) here both are handled
    log_list += get_log_list_from_path(folder_path, req_ext='.mf4', required_sub_strings=required_path_sub_strings,
                                       restricted_sub_strings=restricted_path_sub_strings, level=folder_search_level)
    pbar = tqdm.tqdm(total=len(log_list))
    for log_path in log_list:
        try:
            sleep(0.01)  # to don't put tqdm bar and print message in same line
            print(' ')

            log_name = os.path.split(log_path)[-1]
            output[log_name] = extract_f360_bmw_mid_from_mf4(log_path, sw_version=sw_version,
                                                             save_to_file=save_to_file,
                                                             save_mgp_parsed_data=save_mgp_parsed_data,
                                                             raw_signals=raw_signals,
                                                             force_parse=force_parse,
                                                             force_extract=force_extract,
                                                             host_rear_axle_to_front_bumper_dist=host_rear_axle_to_front_bumper_dist)
            pbar.update(1)
        except Exception as e:
            traceback.print_exc()
            print(f'Log {log_name} will not be extracted')
    pbar.close()
    return output


def parse_faseth_data(
        mf4_log_path: str,
        sw_version: str,
        bus_name: str,
        force_parse: bool = False,
        save_mgp_parsed_data: bool = True,
        parse_cache_path: Path = None,
):
    mf4_log_path = Path(mf4_log_path)
    if parse_cache_path is None:
        parse_cache_path = mf4_log_path.parents[1] / 'parse_result' / mf4_log_path.parent.name
    parse_cache_path.mkdir(exist_ok=True, parents=True)
    mgp_path = parse_cache_path / f'{mf4_log_path.stem}_{bus_name}.mgp'
    if not mgp_path.exists() or force_parse:  # check if glob found any mgp files related to log file
        if save_mgp_parsed_data:
            result_path = mgp_path
        else:
            result_path = None
        parsed = mdf_parser.parse_file(mf4_log_path, config='mid', translation_db=sw_version,
                                       bus_id=bus_name, result_path=result_path)
    else:
        print(f'Found .mgp file with {bus_name}. Loading it instead of parsing...')
        parsed = mgp_module.load(mgp_path)
    return parsed
