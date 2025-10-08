import matplotlib.pyplot as plt

from aspe.evaluation.Visualization.extract_scan_index import ScanIndexExtraction
from aspe.evaluation.Visualization.scan_index_visualization import visualize_scan_index
from aspe.evaluation.Visualization.utilities import add_azimuth_vcs
from aspe.examples.evaluation.private.user_data import ExampleData
from aspe.extractors.ReferenceExtractor.RtRange.rt_3000_msg_sets_config import rt_3000_message_sets_config_ch_4
from aspe.providers import F360MudpRTRangeDataProvider

"""
Run this script as 'Run File in Console'
This example shows simple visualization for particular scan index which you can use for sanity check:
- if results from evaluation match what is observed in data
"""

# Data
mudp_stream_def_path = ExampleData.mudp_stream_def_path
log = (ExampleData.rt_range_data_path +
       r"\LSS\rRf360t4100309v205p50_2_15\FTP201_TC8_162239_001_rRf360t4100309v205p50_2_15.mudp")
scan_index = 70
dbc_config = {4: 'configs\\RT_range_target_merged_working.dbc'}

data_provider = F360MudpRTRangeDataProvider(dbc_config, rt_3000_message_sets_config_ch_4, mudp_stream_def_path,
                                            shift_offset=0.3)
est_data, ref_data = data_provider.get_single_log_data(log)
add_azimuth_vcs(est_data)
single_data_set = est_data.objects

# Scan index visualization
fig, ax = plt.subplots()
visualize_scan_index(ax, est_data, ref_data, scan_index)
ax.axis('equal')

# Additionally: Scan index extraction examples
scan_index_data_set = ScanIndexExtraction.extract_from_single_data_set(single_data_set, scan_index)

scan_index_est_data = ScanIndexExtraction.extract_from_extracted_data(est_data, scan_index)
scan_index_ref_data = ScanIndexExtraction.extract_from_extracted_data(ref_data, scan_index)

min_scan_index_est = ScanIndexExtraction.get_min_scan_index(est_data)
min_scan_index_ref = ScanIndexExtraction.get_min_scan_index(ref_data)

max_scan_index_est = ScanIndexExtraction.get_max_scan_index(est_data)
max_scan_index_ref = ScanIndexExtraction.get_max_scan_index(ref_data)
