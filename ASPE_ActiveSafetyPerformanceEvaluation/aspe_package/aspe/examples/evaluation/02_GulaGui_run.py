from aspe.evaluation.Visualization.GulaGui import GulaGui
from aspe.examples.evaluation.private.user_data import ExampleData
from aspe.extractors.ReferenceExtractor.RtRange.rt_3000_msg_sets_config import rt_3000_message_sets_config_ch_4
from aspe.providers import F360MudpRTRangeDataProvider

"""
Run this script as 'Run File in Console'
This example shows simple visualization GUI which you can use for sanity check:
- if results from evaluation match what is observed in data
Use right and left keyboard arrows to iterate Scan index on clik on SI (Scan Index) slider to change it
Left and right arrow are from main view are disabled! 
"""

# Run this script as 'Run File in Console'
mudp_stream_def_path = ExampleData.mudp_stream_def_path
log = (ExampleData.rt_range_data_path +
       r"\LSS\rRf360t4100309v205p50_2_15\FTP201_TC8_162239_001_rRf360t4100309v205p50_2_15.mudp")
dbc_config = {4: 'configs\\RT_range_target_merged_working.dbc'}

data_provider = F360MudpRTRangeDataProvider(dbc_config, rt_3000_message_sets_config_ch_4, mudp_stream_def_path)
est_data, ref_data = data_provider.get_single_log_data(log)
gui = GulaGui(f_internal_objects=True)
gui.load_data(est_data, ref_data)
