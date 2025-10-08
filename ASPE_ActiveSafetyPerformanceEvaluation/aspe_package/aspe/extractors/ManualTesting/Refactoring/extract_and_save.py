"""
1. Create 'private' directory in 'aspe.extractors\ManualTesting\Refactoring' if not exists.
2. Provide a list of paths to MUDP logs to perform a comparision on (mudp_log_paths).
3. Configure MUDP parser if needed (mudp_parser_config_path and mudp_stream_def_path).
4. Change suffix to either '_before' or '_after'.
5. Execute this script in Python console (click "Run 'extract_and_save'" or press Ctrl+Shift+F10 in PyCharm).

6. Switch workspace to another branch or changeset.
7. Change suffix to either '_after' or '_before'.
8. Execute this script in Python console (click "Run 'extract_and_save'" or press Ctrl+Shift+F10 in PyCharm).
"""
import warnings

from aspe.extractors.F360.Mudp.F360MUDPExtractor import F360MUDPExtractor
from aspe.extractors.ManualTesting.Refactoring import utils
from aspe.parsers.MudpParser import MudpHandler
from aspe.utilities.SupportingFunctions import save_todict_pkl


def extract_and_save(mudp_log_path: str, mudp_parser_config_path: str, mudp_stream_def_path: str,
                     suffix: str = '') -> None:
    """
    Parses MUDP log, extracts F360 datasets and saves extracted data to a pickle.
    :param mudp_log_path: path to the original log file
    :param mudp_parser_config_path: path to parser configuration JSON file
    :param mudp_stream_def_path: path to stream definition directory
    :param suffix: extracted data version suffix, e.g. '_before' or '_after'
    :return:
    """

    # Parse
    print('Parsing...')
    mudp_parser = MudpHandler(mudp_log_path, mudp_parser_config_path, mudp_stream_def_path)
    parsed_data = mudp_parser.decode()

    # Extract
    print('Extracting...')
    f360_mudp_extractor = F360MUDPExtractor(
        f_extract_objects=True,
        f_extract_internal_objects=True,
        f_extract_sensors=True,
        f_extract_host=True,
        f_extract_trailer=True,
        f_extract_detections=True,
        f_extract_execution_time_info=True,
        f_builders_extract_raw_signals=True,
    )
    extracted_data = f360_mudp_extractor.extract_data(parsed_data)

    # Save
    print('Saving...')
    log_name = utils.log_path_to_log_name(mudp_log_path)
    pickle_path = f'private/{log_name}{suffix}.pickle'  # TODO: make directory path configurable
    save_todict_pkl(extracted_data, pickle_path)

    print('Completed')


if __name__ == '__main__':
    mudp_parser_config_path = r'\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\aspe.extractors_refactoring\parsers_config\mudp_data_parser_config.json'
    mudp_stream_def_path = r'\\10.224.186.68\AD-Shared\ASPE\configurations\F360\MUDP_Stream_Definitions\stream_definitions'

    mudp_log_paths = [
        r"\\10.224.186.68\AD-Shared\F360\Logs\Golden_Set_For_Refactoring\PSA\rRf360t4000304v202r1p50_AIT_1057_B\PSA_20180613_152933_003_rRf360t4000304v202r1p50_AIT_1057_B.mudp",
        r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\performance_evaluation\multilog_pipeline\smoke_tests\FTP402_TC1_10_150435_001_rRf360t4060306v204p50.mudp",
        r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\performance_evaluation\multilog_pipeline\smoke_tests\FTP402_TC2_0_151201_001_rRf360t4060306v204p50.mudp",
        r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\performance_evaluation\multilog_pipeline\smoke_tests\FTP402_TC2_0_151250_001_rRf360t4060306v204p50.mudp",
        r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\performance_evaluation\multilog_pipeline\smoke_tests\FTP900_TC1_113155_001_rRf360t4060306v204p50.mudp",
        r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors\F360\F360MUDPExtractor\smoke_test\6p2_6\rRf360t4010304v202r1p50\6p2_6_Sensor_1_20190321_141531_047_rRf360t4010304v202r1p50.mudp",
        # r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors\F360\F360MUDPExtractor\smoke_test\PSA\PSA_20180613_152933_003.mudp",
        r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors\F360\F360MUDPExtractor\smoke_test\PSA\rRf360t4000304v202r1p50\PSA_20180613_152933_003_rRf360t4000304v202r1p50_AIT_1057_B.mudp",
        r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors\F360\F360MUDPExtractor\smoke_test\PSA\rRf360t4000304v202r1p50\PSA_20180613_152933_006_rRf360t4000304v202r1p50_AIT_1057_B.mudp",
        r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors\F360\F360MUDPExtractor\smoke_test\PSA\rRf360t4000304v202r1p50\PSA_20180613_152933_010_rRf360t4000304v202r1p50_AIT_1057_B.mudp",
        r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors\F360\F360MUDPExtractor\smoke_test\PSA\rRf360t4060306v205p50_2_12\PSA_20180613_152933_003_rRf360t4060306v205p50.mudp",
        r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors\F360\F360MUDPExtractor\smoke_test\PSA\rRf360t4060306v205p50_2_12\PSA_20180613_152933_006_rRf360t4060306v205p50.mudp",
        r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors\F360\F360MUDPExtractor\smoke_test\PSA\rRf360t4060306v205p50_2_12\PSA_20180613_152933_010_rRf360t4060306v205p50.mudp",
        r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors\F360\F360MUDPExtractor\smoke_test\RNA\RT\rRf360t4010304v202r1p50\RNASUV_SRR5_K0402B_20181112_201_MO_TC1_80_BEHIND_002_rRf360t4010304v202r1p50.mudp",
        r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors\F360\F360MUDPExtractor\smoke_test\RNA\RT\rRf360t4010304v202r1p50\RNASUV_SRR5_K0402B_20181112_201_MO_TC1_80_L_001_rRf360t4010304v202r1p50.mudp",
        r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors\F360\F360MUDPExtractor\smoke_test\RNA\RT\rRf360t4010304v202r1p50\RNASUV_SRR5_K0402B_20181112_201_MO_TC1_100_R_001_rRf360t4010304v202r1p50.mudp",
        r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors\F360\F360MUDPExtractor\smoke_test\RNA\RT\rRf360t4060306v204p50\FTP402_TC1_10_150435_001_rRf360t4060306v204p50.mudp",
        r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors\F360\F360MUDPExtractor\smoke_test\V6P0\rRf360t4010304v202r1p50\V6P0_Marcel_3_20190228_151642_007_rRf360t4010304v202r1p50.mudp",
    ]
    suffix = '_after'  # '_before' or '_after'

    n = len(mudp_log_paths)
    for i, mudp_log_path in enumerate(mudp_log_paths, 1):
        print(f"\n\033[94m({i}/{n}) {mudp_log_path}\033[0m")

        with warnings.catch_warnings():
            warnings.simplefilter('ignore')
            extract_and_save(mudp_log_path, mudp_parser_config_path, mudp_stream_def_path, suffix)

    print('Finished')
