import os
import traceback
from pathlib import Path
from time import sleep
from typing import Dict, List, Optional, Union

import tqdm

from aspe.extractors.F360.DataSets.F360MudpExtractedData import F360MudpExtractedData
from aspe.extractors.F360.Mudp.F360MUDPExtractor import F360MUDPExtractor
from aspe.parsers.MudpParser import f360_defaults_configs
from aspe.parsers.MudpParser.mudp_parser import MudpParser
from aspe.utilities.SupportingFunctions import (
    create_pickle_path,
    get_log_list_from_path,
    is_log_newer_than_pickle,
    load_from_pkl,
    save_to_pkl,
)


def parse_mudp(mudp_log_path: str,
               mudp_stream_def_path: Optional[str] = None,
               streams_to_parse: tuple = f360_defaults_configs.streams_to_read,
               source_to_parse: Optional[int] = None,
               unknown_size_per_stream: dict = f360_defaults_configs.unknown_size_per_stream,
               unknown_type_handling: bool = True,
               unknown_size_warnings: bool = False):
    """
    Parse mudp file using stream definition files.
    :param mudp_log_path: absolute path to .mudp log file
    :type mudp_log_path: str
    :param mudp_stream_def_path: absolute path to folder with stream definitions - optional- if not passed, parser will
    try to use MUDP_STREAM_DEFINITIONS_PATH environmental variable
    :type mudp_stream_def_path: str
    :return: dict containing parsed .mudp data 
    :type: dict
    :param streams_to_parse: tuple of streams which should be parsed
    :type: tuple
    :param source_to_parse: source number to parse, default set to None - parser does not check streams source number,
    if set to some value - only streams which source number is equal to source_to_parse will be processed
    :type source_to_parse: int
    :param unknown_size_per_stream: when inside stream definitions there is variable type which is unknown (enums for
    example) parser use this dict to check how to read and store variable, this dictionary should be in form
    {stream_number: number_of_bytes} - it assumes different unknown sizes for each channel,
    some default configuration is saved inside parser file, passed dict overwrites only these keys which are different
    than default
    :type: dict
    :param unknown_type_handling: if True unknown variables types which are stored in stream definitions (enums for
    example) are read using unknown_size_per_stream dict argument, if False - these variables will be not saved to
    output
    :type: bool
    :param unknown_size_warnings: if True when parser meet unknown variable type warning and appropriate info will be
    displayed
    :param: bool

    Additional notes:
    Example of .mudp logs can be found:
    https://aspera-emea.aptiv.com/browse/shares/136?path=%2FASPE_Examples%2FF360_RT_Range%2FRCTA
    """
    mudp_parser = MudpParser(mudp_stream_def_path=mudp_stream_def_path,
                             streams_to_read=streams_to_parse,
                             source_to_parse=source_to_parse,
                             unknown_size_per_stream=unknown_size_per_stream,
                             unknown_type_handling=unknown_type_handling,
                             unknown_size_warnings=unknown_size_warnings)
    parsed_data = mudp_parser.parse(mudp_log_path)
    return parsed_data


def extract_f360_from_mudp(mudp_log_path: str,
                           mudp_stream_def_path: Optional[str] = None,
                           streams_to_parse: tuple = f360_defaults_configs.streams_to_read,
                           source_to_parse: Optional[int] = None,
                           unknown_size_per_stream: dict = f360_defaults_configs.unknown_size_per_stream,
                           unknown_type_handling: bool = True,
                           unknown_size_warnings: bool = False,
                           save_to_file: bool = False,
                           objects: bool = True,
                           host: bool = True,
                           trailer: bool = False,
                           internal_objects: bool = False,
                           oal_objects: bool = False,
                           sensors: bool = True,
                           detections: bool = False,
                           gdsr: bool = False,
                           stat_env: bool = False,
                           execution_time_info: bool = False,
                           raw_signals: bool = False,
                           raise_exc_if_stream_missing: bool = False,
                           raise_exc_if_version_not_supported: bool = False,
                           force_extract: bool = False) -> F360MudpExtractedData:
    '''
    Extract F360 tracker core data from .mudp log file.
    :param mudp_log_path: absolute path to .mudp log file
    :type mudp_log_path: str
    :param mudp_stream_def_path: absolute path to folder with stream definitions - optional- if not passed, parser will
    try to use MUDP_STREAM_DEFINITIONS_PATH environmental variable
    :type mudp_stream_def_path: str
    :param streams_to_parse: tuple of streams which should be parsed
    :type: tuple
    :param source_to_parse: source number to parse, default set to None - parser does not check streams source number,
    if set to some value - only streams which source number is equal to source_to_parse will be processed
    :type source_to_parse: int
    :param unknown_size_per_stream: when inside stream definitions there is variable type which is unknown (enums for
    example) parser use this dict to check how to read and store variable, this dictionary should be in form
    {stream_number: number_of_bytes} - it assumes different unknown sizes for each channel,
    some default configuration is saved inside parser file, passed dict overwrites only these keys which are different
    than default
    :type: dict
    :param unknown_type_handling: if True unknown variables types which are stored in stream definitions (enums for
    example) are read using unknown_size_per_stream dict argument, if False - these variables will be not saved to
    output
    :type: bool
    :param unknown_size_warnings: if True when parser meet unknown variable type warning and appropriate info will be
    displayed
    :param: bool
    :param save_to_file: flag indicating if extracted output should be saved as .pickle file. If is True .pickle
    file named log_name_f360_mudp_extracted.pickle will be created within same folder.
    :type save_to_file: bool
    :param objects: flag indicating if reduced objects should be extracted
    :type objects: bool
    :param internal_objects: flag indicating if non-reduced objects should be extracted
    :type internal_objects: bool
    :param oal_objects: flag indicating if Output Adaptation Layer objects should be extracted
    :type oal_objects: bool
    :param sensors: flag indicating if sensors should be extracted
    :type sensors: bool
    :param host: flag indicating if host should be extracted
    :type host: bool
    :param trailer: flag indicating if trailer should be extracted
    :type trailer: bool
    :param detections: flag indicating if detections should be extracted
    :type detections: bool
    :param gdsr: flag indicating if gdsr objects should be extracted
    :type gdsr: bool
    :param stat_env: flag indicating if static environment should be extracted
    :type stat_env: bool
    :param execution_time_info: flag indicating if timing info should be extracted
    :type execution_time_info:
    :param raw_signals: flag indicating if raw signals should be extracted. Raw signals have same signatures as in
    stream definition - there is no translation applied. If yes extracted raw signals are placed in separate DataFrame
    within data set.
    :type raw_signals: bool
    :param raise_exc_if_stream_missing: flag indicating if exception should be raised if there is no stream needed for
    specific data set extraction. If False - only warning is shown.
    :type raise_exc_if_stream_missing: bool
    :param raise_exc_if_version_not_supported: flag indicating if exception should be raised if extractor can not handle
    given stream definition version. If False - only warning is shown.
    :type raise_exc_if_version_not_supported: bool
    :return: extracted data in ASPE data structure form
    :type: F360MUDPExtractedData
    Additional notes:
    Example of .mudp logs can be found: https://aspera-emea.aptiv.com/browse/shares/136?path=%2FASPE_Examples%2FF360_RT_Range%2FRCTA
    '''
    print(f'Started processing MUDP log file: {os.path.split(mudp_log_path)[-1]}')
    pickle_save_path = create_pickle_path(mudp_log_path)
    if Path(pickle_save_path).exists() and not force_extract and not is_log_newer_than_pickle(mudp_log_path,
                                                                                              pickle_save_path):
        print('Found pickle file. Loading instead of extracting.')
        extracted = load_from_pkl(pickle_save_path)
    else:
        print(f'Parsing data ...')
        parsed = parse_mudp(mudp_log_path, mudp_stream_def_path,
                            streams_to_parse=streams_to_parse,
                            source_to_parse=source_to_parse,
                            unknown_size_per_stream=unknown_size_per_stream,
                            unknown_type_handling=unknown_type_handling,
                            unknown_size_warnings=unknown_size_warnings)

        print(f'Extracting data ...')
        extractor = F360MUDPExtractor(
            f_extract_objects=objects,
            f_extract_internal_objects=internal_objects,
            f_extract_oal_objects=oal_objects,
            f_extract_sensors=sensors,
            f_extract_host=host,
            f_extract_trailer=trailer,
            f_extract_detections=detections,
            f_extract_execution_time_info=execution_time_info,
            f_extract_gdsr_objects=gdsr,
            f_extract_stat_env=stat_env,
            f_builders_extract_raw_signals=raw_signals,
            f_raise_exc_if_stream_missing=raise_exc_if_stream_missing,
            f_raise_exc_if_version_not_supported=raise_exc_if_version_not_supported)
        extracted = extractor.extract_data(parsed)
        print('Extraction done')
        if save_to_file:
            save_to_pkl(extracted, pickle_save_path)
            print(f'Saved output to {pickle_save_path}')

    return extracted


def extract_f360_from_mudp_folder(folder_path: str,
                                  mudp_stream_def_path: Optional[str] = None,
                                  streams_to_parse: tuple = f360_defaults_configs.streams_to_read,
                                  source_to_parse: Optional[int] = None,
                                  unknown_size_per_stream: dict = f360_defaults_configs.unknown_size_per_stream,
                                  unknown_size_handling: bool = True,
                                  unknown_size_warnings: bool = False,
                                  save_to_file: bool = True,
                                  objects: bool = True,
                                  host: bool = True,
                                  trailer: bool = False,
                                  internal_objects: bool = False,
                                  oal_objects: bool = False,
                                  sensors: bool = False,
                                  detections: bool = False,
                                  gdsr: bool = False,
                                  stat_env: bool = False,
                                  execution_time_info: bool = False,
                                  raw_signals: bool = False,
                                  raise_exc_if_stream_missing: bool = False,
                                  raise_exc_if_version_not_supported: bool = False,
                                  required_path_sub_strings: Union[str, List[str]] = None,
                                  restricted_path_sub_strings: Union[str, List[str]] = None,
                                  folder_search_level: int = 0,
                                  force_extract: bool = False) -> Dict[str, F360MudpExtractedData]:
    '''
    Find .mudp files in given folder and extract F360 tracker core data from it.
    :param gdsr: flag indicating if gdsr objects should be extracted
    :param folder_path: absolute path to folder with .mudp log files
    :type folder_path: str
    :param mudp_stream_def_path: absolute path to folder with stream definitions - optional- if not passed, parser will
    try to use MUDP_STREAM_DEFINITIONS_PATH environmental variable
    :type mudp_stream_def_path: str
    :param streams_to_parse: tuple of streams which should be parsed
    :type: tuple
    :param source_to_parse: source number to parse, default set to None - parser does not check streams source number,
    if set to some value - only streams which source number is equal to source_to_parse will be processed
    :type source_to_parse: int
    :param unknown_size_per_stream: when inside stream definitions there is variable type which is unknown (enums for
    example) parser use this dict to check how to read and store variable, this dictionary should be in form
    {stream_number: number_of_bytes} - it assumes different unknown sizes for each channel,
    some default configuration is saved inside parser file, passed dict overwrites only these keys which are different
    than default
    :type: dict
    :param unknown_size_handling: if True unknown variables types which are stored in stream definitions (enums for
    example) are read using unknown_size_per_stream dict argument, if False - these variables will be not saved to
    output
    :type: bool
    :param unknown_size_warnings: if True when parser meet unknown variable type warning and appropriate info will be
    displayed
    :param: bool
    :param save_to_file: flag indicating if extracted output should be saved as .pickle file. If is True .pickle
    file named log_name_f360_mudp_extracted.pickle will be created within same folder.
    :type save_to_file: bool
    :param objects: flag indicating if reduced objects should be extracted
    :type objects: bool
    :param internal_objects: flag indicating if non-reduced objects should be extracted
    :type internal_objects: bool
    :param oal_objects: flag indicating if Output Adaptation Layer objects should be extracted
    :type oal_objects: bool
    :param sensors: flag indicating if sensors should be extracted
    :type sensors: bool
    :param host: flag indicating if host should be extracted
    :type host: bool
    :param trailer: flag indicating if trailer should be extracted
    :type trailer: bool
    :param detections: flag indicating if detections should be extracted
    :type detections: bool
    :param stat_env: flag indicating if static environment should be extracted
    :type stat_env: bool
    :param execution_time_info: flag indicating if timing info should be extracted
    :type execution_time_info:
    :param raw_signals: flag indicating if raw signals should be extracted. Raw signals have same signatures as in
    stream definition - there is no translation applied. If yes extracted raw signals are placed in separate DataFrame
    within data set.
    :type raw_signals: bool
    :param raise_exc_if_stream_missing: flag indicating if exception should be raised if there is no stream needed for
    specific data set extraction. If False - only warning is shown.
    :type raise_exc_if_stream_missing: bool
    :param raise_exc_if_version_not_supported: flag indicating if exception should be raised if extractor can not handle
    given stream definition version. If False - only warning is shown.
    :type raise_exc_if_version_not_supported: bool
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
    :return: dict containing extracted data from multiple logs, keys - log names, values - extracted data
    :type: dict(str: F360MudpExtractedData)

    Additional notes:
    Example of .mudp logs can be found:
    https://aspera-emea.aptiv.com/browse/shares/136?path=%2FASPE_Examples%2FF360_RT_Range%2FRCTA
    '''
    output = {}
    log_list = get_log_list_from_path(folder_path, req_ext='.mudp', required_sub_strings=required_path_sub_strings,
                                      restricted_sub_strings=restricted_path_sub_strings, level=folder_search_level)
    pbar = tqdm.tqdm(total=len(log_list))
    for log_path in log_list:
        try:
            sleep(0.01)  # to don't put tqdm bar and print message in same line
            print(' ')

            log_name = os.path.split(log_path)[-1]
            output[log_name] = extract_f360_from_mudp(log_path,
                                                      mudp_stream_def_path=mudp_stream_def_path,
                                                      streams_to_parse=streams_to_parse,
                                                      source_to_parse=source_to_parse,
                                                      unknown_size_per_stream=unknown_size_per_stream,
                                                      unknown_type_handling=unknown_size_handling,
                                                      unknown_size_warnings=unknown_size_warnings,
                                                      save_to_file=save_to_file,
                                                      objects=objects,
                                                      host=host,
                                                      trailer=trailer,
                                                      internal_objects=internal_objects,
                                                      oal_objects=oal_objects,
                                                      gdsr=gdsr,
                                                      stat_env=stat_env,
                                                      sensors=sensors,
                                                      detections=detections,
                                                      execution_time_info=execution_time_info,
                                                      raw_signals=raw_signals,
                                                      raise_exc_if_stream_missing=raise_exc_if_stream_missing,
                                                      raise_exc_if_version_not_supported=raise_exc_if_version_not_supported,
                                                      force_extract=force_extract)
            pbar.update(1)
        except Exception as e:
            traceback.print_exc()
            print(f'Log {log_name} will not be extracted')
    pbar.close()
    return output


if __name__ == '__main__':
    mudp_stream_definitions_path = ""
    extracted = extract_f360_from_mudp(r"",
                                       mudp_stream_def_path=mudp_stream_definitions_path,
                                       raw_signals=True,
                                       sensors=True,
                                       internal_objects=True,
                                       detections=True,
                                       trailer=True,
                                       save_to_file=True)
