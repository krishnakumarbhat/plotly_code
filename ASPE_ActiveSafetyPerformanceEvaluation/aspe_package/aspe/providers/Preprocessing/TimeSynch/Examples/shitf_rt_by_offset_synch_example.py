from aspe.extractors.API.dvl import extract_rt_range_3000_from_dvl
from aspe.extractors.API.mudp import extract_f360_from_mudp
from aspe.providers.Preprocessing import ShiftSlaveExtractedDataTimestamp


def example_preprocessing_pipeline(mudp_log_path, dvl_log_path):
    mudp_stream_def_path = r"\\10.224.186.68\AD-Shared\ASPE\configurations\F360\MUDP_Stream_Definitions\stream_definitions"

    rt_extracted_data = extract_rt_range_3000_from_dvl(dvl_log_path)
    f360_extracted_data = extract_f360_from_mudp(mudp_log_path, mudp_stream_def_path=mudp_stream_def_path)

    # Verify
    time_synchronization = ShiftSlaveExtractedDataTimestamp(offset=-0.035)
    time_synched_f360_data, time_synched_rt_data = time_synchronization.synch(master_extracted_data=f360_extracted_data,
                                                                              slave_extracted_data=rt_extracted_data)

    time_synched_rt_data.objects.signals['unique_id'] = time_synched_rt_data.objects.signals['unique_obj_id']
    return time_synched_f360_data, time_synched_rt_data


if __name__ == '__main__':
    dvl_log_path = r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\extractors\ReferenceExtractor\RtDVLExtractor\smoke_tests\RNA\RT\rRf360t4010304v202r1p50\RNASUV_SRR5_K0402B_20181112_201_MO_TC1_80_L_001_rRf360t4010304v202r1p50.dvl"
    mudp_log_path = r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\extractors\F360\F360MUDPExtractor\smoke_test\RNA\RT\rRf360t4010304v202r1p50\RNASUV_SRR5_K0402B_20181112_201_MO_TC1_80_L_001_rRf360t4010304v202r1p50.mudp"
    time_synched_f360_data, time_synched_rt_data = example_preprocessing_pipeline(mudp_log_path, dvl_log_path)

    print()
