from aspe.extractors.API.dvl import extract_rt_range_3000_from_dvl
from aspe.extractors.API.mudp import extract_f360_from_mudp
from aspe.providers.Preprocessing import NearestTimestampSynch, ShiftSlaveExtractedDataTimestamp


def preprocessing_pipeline(mudp_log_path, dvl_log_path):
    mudp_stream_def_path = r"\\10.224.186.68\AD-Shared\ASPE\configurations\F360\MUDP_Stream_Definitions\stream_definitions"

    rt_extracted_data = extract_rt_range_3000_from_dvl(dvl_log_path)
    f360_extracted_data = extract_f360_from_mudp(mudp_log_path, mudp_stream_def_path=mudp_stream_def_path)

    # Verify
    time_synchronize = ShiftSlaveExtractedDataTimestamp(offset=-0.035)
    time_synch_f360_data, time_synch_rt_data = time_synchronize.synch(master_extracted_data=f360_extracted_data,
                                                                      slave_extracted_data=rt_extracted_data,
                                                                      inplace=True)

    rt_f360_si_synch = NearestTimestampSynch()
    scan_index_synch_f360_data, scan_index_synch_rt_data = rt_f360_si_synch.synch(time_synch_f360_data,
                                                                                  time_synch_rt_data,
                                                                                  time_synch_f360_data.tracker_info,
                                                                                  time_synch_rt_data.host)
    return scan_index_synch_f360_data, scan_index_synch_rt_data


if __name__ == '__main__':
    dvl_log_path = r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\extractors\ReferenceExtractor\RtDVLExtractor\smoke_tests\RNA\RT\rRf360t4060306v204p50\FTP402_TC1_10_150435_001_rRf360t4060306v204p50.dvl"
    mudp_log_path = r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\extractors\F360\F360MUDPExtractor\smoke_test\RNA\RT\rRf360t4060306v204p50\FTP402_TC1_10_150435_001_rRf360t4060306v204p50.mudp"

    preprocessing_pipeline(mudp_log_path, dvl_log_path)
