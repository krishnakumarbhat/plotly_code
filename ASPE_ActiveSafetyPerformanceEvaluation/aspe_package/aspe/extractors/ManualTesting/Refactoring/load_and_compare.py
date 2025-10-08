"""
9. Provide a list of paths to MUDP logs to perform a comparision on (mudp_log_paths).
10. Make sure 'private' directory exists in 'aspe.extractors\ManualTesting\Refactoring'.
11. Make sure both '_before.pickle' and '_after.pickle' files exist in 'private' directory for each MUDP log.
12. Execute this script as pytest suite (click "Run 'pytest in load_and_compare'" or press Ctrl+Shift+F10 in PyCharm).
"""
import pytest

from aspe.extractors.ManualTesting.Refactoring.utils import assert_extracted_data_dict_equal, log_path_to_log_name
from aspe.utilities.SupportingFunctions import load_from_pkl

mudp_log_paths = [
    r"\\10.224.186.68\AD-Shared\F360\Logs\Golden_Set_For_Refactoring\PSA\rRf360t4000304v202r1p50_AIT_1057_B\PSA_20180613_152933_003_rRf360t4000304v202r1p50_AIT_1057_B.mudp",
    r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\performance_evaluation\multilog_pipeline\smoke_tests\FTP402_TC1_10_150435_001_rRf360t4060306v204p50.mudp",
    r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\performance_evaluation\multilog_pipeline\smoke_tests\FTP402_TC2_0_151201_001_rRf360t4060306v204p50.mudp",
    r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\performance_evaluation\multilog_pipeline\smoke_tests\FTP402_TC2_0_151250_001_rRf360t4060306v204p50.mudp",
    r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\performance_evaluation\multilog_pipeline\smoke_tests\FTP900_TC1_113155_001_rRf360t4060306v204p50.mudp",
    r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors\F360\F360MUDPExtractor\smoke_test\6p2_6\rRf360t4010304v202r1p50\6p2_6_Sensor_1_20190321_141531_047_rRf360t4010304v202r1p50.mudp",
    #r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors\F360\F360MUDPExtractor\smoke_test\PSA\PSA_20180613_152933_003.mudp",
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


@pytest.mark.parametrize('mudp_log_path', mudp_log_paths)
def test_load_and_compare(mudp_log_path: str) -> None:
    """
    Loads two versions of extracted data from pickle files and checks their equivalence.
    :param mudp_log_path: path to the original log file
    :return:
    """
    log_name = log_path_to_log_name(mudp_log_path)

    pickle_path_before = f'private/{log_name}_before.pickle'
    pickle_path_after = f'private/{log_name}_after.pickle'

    extracted_data_before = load_from_pkl(pickle_path_before)
    extracted_data_after = load_from_pkl(pickle_path_after)

    assert_extracted_data_dict_equal(extracted_data_before, extracted_data_after)
