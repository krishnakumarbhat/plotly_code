import pytest

from aspe.providers import F360MudpRTRangeDataProvider
from aspe.evaluation.RadarObjectsEvaluation.PEMultiLogEvaluation import PEMultiLogEvaluation
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.extractors.ReferenceExtractor.RtRange.rt_3000_msg_sets_config import rt_3000_message_sets_config_ch_4


class TestSmokeMultiLogPipeline:
    """
    Smoke test class for multi log pipeline functionalities
    """
    @pytest.fixture
    def pe_pipeline(self):
        return PEPipeline()

    @pytest.fixture
    def data_provider(self, path_provider):
        dbc_config = {4: path_provider.get_rt_range_dbc_path()}
        mudp_stream_def_path = path_provider.get_mudp_stream_def_path()
        return F360MudpRTRangeDataProvider(dbc_config, rt_3000_message_sets_config_ch_4, mudp_stream_def_path)

    @pytest.fixture
    def logging_folder(self, path_provider):
        return path_provider.get_logging_folder_path()

    def test_smoke_init(self, pe_pipeline, data_provider, logging_folder):
        """
        Smoke test for ObjectsEvaluationMultiLogPipeline initialization
        :return:
        """
        PEMultiLogEvaluation(pe_pipeline, data_provider, logging_folder)

    @pytest.mark.parametrize('f_disable_exc_catch', (True, False))
    @pytest.mark.parametrize('log_list', [
        pytest.param([], id='empty_log_list', marks=pytest.mark.xfail),  # TODO: handle empty log list in process_data

        pytest.param([
            r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\performance_evaluation\multilog_pipeline\smoke_tests\
            FTP402_TC1_10_150435_001_rRf360t4060306v204p50.dvl",
        ], id='single_log_list'),

        pytest.param([
            r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\performance_evaluation\multilog_pipeline\smoke_tests\
            FTP402_TC1_10_150435_001_rRf360t4060306v204p50.dvl",
            r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\performance_evaluation\multilog_pipeline\smoke_tests\
            FTP402_TC1_10_150435_001_rRf360t4060306v204p50.dvl",
        ], id='log_list_with_repetition'),

        pytest.param([
            r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\performance_evaluation\multilog_pipeline\smoke_tests\
            FTP402_TC1_10_150435_001_rRf360t4060306v204p50.dvl",
            r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\performance_evaluation\multilog_pipeline\smoke_tests\
            FTP402_TC2_0_151201_001_rRf360t4060306v204p50.dvl",
            r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\performance_evaluation\multilog_pipeline\smoke_tests\
            FTP402_TC2_0_151250_001_rRf360t4060306v204p50.dvl",
            r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\performance_evaluation\multilog_pipeline\smoke_tests\
            FTP900_TC1_113155_001_rRf360t4060306v204p50.dvl",
        ], id='multi_log_list'),
    ])
    def test_smoke_process_data(self,
                                pe_pipeline,
                                data_provider,
                                logging_folder,
                                path_provider,
                                log_list,
                                f_disable_exc_catch):
        """
        Smoke test for process_data method
        :return:
        """
        for log_path in log_list:
            path_provider.get_log_path(log_path.replace('.dvl', '.mudp'))
        log_list = [path_provider.get_log_path(log_path) for log_path in log_list]
        multi_log_pipeline = PEMultiLogEvaluation(pe_pipeline, data_provider, logging_folder)
        multi_log_pipeline.process_data(log_list, f_disable_exc_catch=f_disable_exc_catch)
