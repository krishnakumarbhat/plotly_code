import os
from time import sleep
from typing import Dict, List, Union

import tqdm

from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.extractors.ReferenceExtractor.RtDVLExtractor import RtDVLExtractor
from aspe.extractors.ReferenceExtractor.RtRange.rt_3000_msg_sets_config import rt_3000_message_sets_config_ch_4
from aspe.parsers.DvlParser import DVLParser
from aspe.utilities.SupportingFunctions import get_default_rt_3000_dbc_path, get_log_list_from_path, save_to_pkl


def parse_dvl(dvl_log_path: str, dbc_config: dict):
    """
    Parse data from given .dvl file
    :param dvl_log_path: path to .dvl file
    :type dvl_log_path: str
    :param dbc_config: configuration for used dbc files, input is dictionary in form: {channel_num: path_to_dbc}
    :type dbc_config: dict
    :return: parsed data
    :type: dict

    Additional notes:
    Example logs with RTRange data can be found: https://aspera-emea.aptiv.com/browse/shares/136?path=/RNA_SRR5/Opole_CW19
    """
    dvl_parser = DVLParser(dbc_config=dbc_config)
    output = dvl_parser.parse(dvl_log_path)
    return output


def extract_rt_range_3000_from_dvl(dvl_log_path: str,
                                   dbc_config: dict = {4: get_default_rt_3000_dbc_path()},
                                   message_sets_config: dict = rt_3000_message_sets_config_ch_4,
                                   raw_signals: bool = False,
                                   completed_messages_only: bool = True,
                                   save_to_file: bool = False) -> ExtractedData:
    """
    Extract RtRange3000 data from given .dvl log file.
    :param dvl_log_path: path to .dvl log file
    :type dvl_log_path: str
    :param dbc_config: configuration for used dbc files, input is dictionary in form: {channel_num: path_to_dbc}
    :type dbc_config: dict
    :param message_sets_config: configuration which contains information how to group CAN frames
    :type message_sets_config: dict
    :param raw_signals: flag indicating if raw signals should be extracted. Raw signals have same signatures as in
    stream definition - there is no translation applied. If yes extracted raw signals are placed in separate DataFrame
    within data set.
    :type raw_signals: bool
    :param completed_messages_only: only complete messages taken from dvl are saved to extracted data
    :type completed_messages_only: str
    :param save_to_file: flag indicating if extracted output should be saved as .pickle file. If is True .pickle
    file named log_name_f360_mudp_extracted.pickle will be created within same folder.
    :type save_to_file: bool
    :return: ExtractedData

    Additional notes:
    Example logs with RTRange data can be found: https://aspera-emea.aptiv.com/browse/shares/136?path=/RNA_SRR5/Opole_CW19
    """
    print(f'Started processing DVL log file: {os.path.split(dvl_log_path)[-1]}')
    print(f'Parsing data ...')
    parsed = parse_dvl(dvl_log_path, dbc_config=dbc_config)
    print(f'Extracting data ...')
    rt_extractor = RtDVLExtractor(msg_sets_config=message_sets_config, f_extract_raw_signals=raw_signals,
                                  f_completed_messages_only=completed_messages_only)
    extracted = rt_extractor.extract_data(parsed)
    print('Extraction done')
    if save_to_file:
        pickle_save_path = dvl_log_path.replace('.dvl', '_rt_range_3000_dvl_extracted.pickle')
        save_to_pkl(extracted, pickle_save_path)
    return extracted


def extract_rt_range_3000_from_dvl_folder(folder_path: str,
                                          dbc_config: dict = {4: get_default_rt_3000_dbc_path()},
                                          message_sets_config: dict = rt_3000_message_sets_config_ch_4,
                                          completed_messages_only: bool = True,
                                          raw_signals: bool = False,
                                          save_to_file: bool = True,
                                          required_path_sub_strings: Union[str, List[str]] = None,
                                          restricted_path_sub_strings: Union[str, List[str]] = None,
                                          folder_search_level: int = 0,
                                          ) -> Dict[str, ExtractedData]:
    """
    Extract RtRange3000 data from .dvl log files in given folder. Example logs with RTRange data can be found: https://aspera-emea.aptiv.com/browse/shares/136?path=/RNA_SRR5/Opole_CW19
    :param folder_path: path to .dvl log file
    :type folder_path: str
    :param dbc_config: configuration for used dbc files, input is dictionary in form: {channel_num: path_to_dbc}
    :type dbc_config: dict
    :param message_sets_config: configuration which contains information how to group CAN frames
    :type message_sets_config: dict
    :param completed_messages_only: only complete messages taken from dvl are saved to extracted data
    :type completed_messages_only: str
    :param raw_signals: flag indicating if raw signals should be extracted. Raw signals have same signatures as in
    stream definition - there is no translation applied. If yes extracted raw signals are placed in separate DataFrame
    within data set.
    :type raw_signals: bool
    :param save_to_file: flag indicating if extracted output should be saved as .pickle file. If is True .pickle
    file named log_name_f360_mudp_extracted.pickle will be created within same folder.
    :type save_to_file: bool
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
    :return: ExtractedData

    Additional notes:
    Example logs with RTRange data can be found: https://aspera-emea.aptiv.com/browse/shares/136?path=/RNA_SRR5/Opole_CW19
    """
    output = {}
    log_list = get_log_list_from_path(folder_path, req_ext='.dvl', required_sub_strings=required_path_sub_strings,
                                      restricted_sub_strings=restricted_path_sub_strings, level=folder_search_level)
    pbar = tqdm.tqdm(total=len(log_list))
    for log_path in log_list:
        sleep(0.01)  # to don't put tqdm bar and print message in same line
        print(' ')

        log_name = os.path.split(log_path)[-1]
        output[log_name] = extract_rt_range_3000_from_dvl(dvl_log_path=log_path,
                                                          dbc_config=dbc_config,
                                                          message_sets_config=message_sets_config,
                                                          raw_signals=raw_signals,
                                                          completed_messages_only=completed_messages_only,
                                                          save_to_file=save_to_file)

        pbar.update(1)

    pbar.close()
    return output


if __name__ == '__main__':
    folder_path = r'C:\logs\ASPE_example_data_2\F360_RT_Range\LSS'
    extracted = extract_rt_range_3000_from_dvl_folder(folder_path, raw_signals=True, folder_search_level=1)
