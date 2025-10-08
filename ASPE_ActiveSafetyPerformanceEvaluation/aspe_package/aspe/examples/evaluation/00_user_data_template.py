"""
1) Create package in Examples package and name it 'private'
2) Copy this file and rename it in private/user_data
3) Change data_folder_dir variable in user_data.py file into path to your data (example data)
4) This file will be used by other examples as reference for where data is located
"""


class ExampleData:
    data_folder_dir = r'C:\logs\ASPE_example_data'  # update me!
    mudp_stream_def_path = r'C:\stream_definitions'  # update me!
    analysis_data_path = data_folder_dir + r"\Analysis_Data"
    rt_range_data_path = data_folder_dir + r"\F360_RT_Range"
    gdsr_dspace_data_path = data_folder_dir + r"\GDSR_dSpace_bin_logs"
    multi_log_pipeline_logging_folder = analysis_data_path + r"\Multi_Log_Pipeline_Execution_Logs"
    multi_log_evaluation_results_2_15_path = analysis_data_path + r"\multi_log_evaluation_output_2_15.pickle"
    multi_log_evaluation_results_2_12_path = analysis_data_path + r"\multi_log_evaluation_output_2_12.pickle"
    report_save_path = analysis_data_path + r'\ASPE_report_2_12_2_15_compare.pdf'
    report_temp_figs_path = analysis_data_path + r'\report_temp_figs_path'  # create this folder!
