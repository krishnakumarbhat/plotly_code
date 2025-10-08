import pickle

import numpy as np

from aspe.examples.evaluation.private.user_data import ExampleData
from aspe.extractors.ReferenceExtractor.RtRange.rt_3000_msg_sets_config import rt_3000_message_sets_config_ch_4
from aspe.providers.F360MudpRTRangeDataProvider import F360MudpRTRangeDataProvider

"""
Run this script as 'Run File in Console'
Example shows how evaluated data can be used - here for outliers analysis
04_F360_multi_log_evaluation example should be run before
"""

# Configuration
from aspe.evaluation.Visualization.GulaGui import GulaGui

mudp_stream_def_path = ExampleData.mudp_stream_def_path

data_path = ExampleData.multi_log_evaluation_results_2_15_path
dbc_config = {4: 'configs\\RT_range_target_merged_working.dbc'}

speed_error_thr = 2
orientation_error_thr = np.deg2rad(10)

# Load data
with open(data_path, 'rb') as handle:
    pe_output_multi_log = pickle.load(handle)

# Filter data
high_speed_error = np.abs(pe_output_multi_log.pe_results_obj_pairs.signals.dev_speed > speed_error_thr)
paired_objects_high_speed_error = pe_output_multi_log.pe_results_obj_pairs.signals.loc[high_speed_error, :]

high_orientation_error = np.abs(
    pe_output_multi_log.pe_results_obj_pairs.signals.dev_orientation > orientation_error_thr)
paired_objects_high_orientation_error = pe_output_multi_log.pe_results_obj_pairs.signals.loc[high_orientation_error, :]

# Find most relevant log
log_idx_with_most_error = paired_objects_high_orientation_error.log_index.mode().values
log_path = pe_output_multi_log.logs_paths.log_path[log_idx_with_most_error].values[0]
first_scan_index = paired_objects_high_orientation_error.scan_index[
    paired_objects_high_orientation_error.log_index == log_idx_with_most_error[0]].values[0]

# Visualize it
data_provider = F360MudpRTRangeDataProvider(dbc_config, rt_3000_message_sets_config_ch_4, mudp_stream_def_path)
est_data, ref_data = data_provider.get_single_log_data(log_path)
gui = GulaGui()
gui.load_data(est_data, ref_data)
gui.set_scan_index(first_scan_index)
