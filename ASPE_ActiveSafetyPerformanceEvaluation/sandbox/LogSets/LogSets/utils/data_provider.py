from LogSets.utils.logger import Logger
from AptivDataExtractors.F360.F360MUDPExtractor import F360Extractor
from AptivDataParser import MudpParser


class DataProvider:
    def generate_extracted_logs(self, paths, mudp_parser_config, log_info_path=None):
        log_idx, parsed_count, extracted_count = 0, 0, 0
        logger = Logger()
        for log_idx, path in enumerate(paths):
            try:
                parsed_log = self._get_parsed_mudp(path, mudp_parser_config)
            except:
                logger.log(log_idx, path, 'PARSING -> NOT OK', False)
                continue
            else:
                parsed_count += 1
                logger.log(log_idx, path, 'PARSING -> OK', True)

            try:
                extracted_log = self._get_extracted_mudp(parsed_log)
            except:
                logger.log(log_idx, path, 'EXTRACTING -> NOT OK', False)
                continue
            else:
                extracted_count += 1

            yield extracted_log

        parsing_success_rate = f'Parsed: {parsed_count}/{len(paths)} logs'
        extracting_success_rate = f'Extracted: {extracted_count}/{len(paths) - (len(paths)-parsed_count)} logs'
        print(parsing_success_rate)
        print(extracting_success_rate)
        logger.save_logged_info(log_info_path, parsing_success_rate, extracting_success_rate)

    @staticmethod
    def _get_parsed_mudp(mudp_path, mudp_parser_config):
        parser = MudpParser.MudpHandler(mudp_path, mudp_parser_config)
        parsed_data = parser.decode()
        return parsed_data

    @staticmethod
    def _get_extracted_mudp(parsed_data):
        f360_extractor_object = F360Extractor(parsed_data)
        f360_extracted_data = f360_extractor_object.extract_data()
        return f360_extracted_data.to_dict()


def example():
    from LogSets.main import LogSets
    #   Parse and extract logs using generator
    logs_paths = LogSets.find_re_simulated_logs_by_resim_version(
                            root_directory=r'\\10.224.186.68\AD-Shared\F360\Logs\Golden_Set_For_Refactoring\PSA',
                            resim_ver='rRf360t4010304v202r1')
    mudp_parser_config = r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS" \
                  r"\Extractors_refactoring\parsers_config\mudp_data_parser_config.json"
    log_info_path = r"\\10.224.186.68\AD-Shared\F360\Tools\LogSets\Utils\extractors_test_on_alpha_set_logged_info.txt"
    extracted_logs_generator = DataProvider().generate_extracted_logs(logs_paths, mudp_parser_config,
                                                                      log_info_path=log_info_path)

    for extracted_log in extracted_logs_generator:
        log = extracted_log
        print()

    print()


def example_single_log():
    from LogSets.utils.file_handling import save
    data_provider = DataProvider()
    mudp_parser_config = r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS" \
                  r"\Extractors_refactoring\parsers_config\mudp_data_parser_config.json"
    mudp_log_path = r'\\10.224.186.68\AD-Shared\F360\Logs\Marcel_Vaf_Logs_20190228\6p0_Marcel\6p0_Marcel' \
                         r'\rRf360t4010304v202r1\V6P0_Marcel_1_20190228_140719_002_rRf360t4010304v202r1p50.mudp'
    parsed_log = data_provider._get_parsed_mudp(mudp_log_path, mudp_parser_config)
    extracted_log = data_provider._get_extracted_mudp(parsed_log)
    save(extracted_log, r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\Utils\example_extracted_logs'
                       r'\example_extracted_log.pickle')


if __name__ == '__main__':
    # example()
    example_single_log()
