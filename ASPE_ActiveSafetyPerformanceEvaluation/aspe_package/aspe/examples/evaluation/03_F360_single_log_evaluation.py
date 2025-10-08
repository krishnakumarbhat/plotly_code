import os
from time import time

from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.examples.evaluation.private.user_data import ExampleData
from aspe.extractors.ReferenceExtractor.RtRange.rt_3000_msg_sets_config import rt_3000_message_sets_config_ch_4
from aspe.providers.F360MudpRTRangeDataProvider import F360MudpRTRangeDataProvider

"""
Run this script as 'Run File in Console'
Example of single log evaluation:
pe - performance evaluation pipeline is configured by defaults strategies
"""

log = os.path.join(ExampleData.rt_range_data_path,
                   r"RCTA\rRf360t4060306v204p50_2_12\FTP402_TC4_TT_155253_001_rRf360t4060306v204p50_2_12.mudp")
mudp_stream_def_path = ExampleData.mudp_stream_def_path
current_dir = os.path.dirname(os.path.abspath(__file__))
dbc_config = {4: os.path.join(current_dir, r'configs\RT_range_target_merged_working.dbc')}

t1 = time()
data_provider = F360MudpRTRangeDataProvider(dbc_config, rt_3000_message_sets_config_ch_4, mudp_stream_def_path)
estimated_data, reference_data = data_provider.get_single_log_data(log)

t2 = time()
pe = PEPipeline()
output = pe.evaluate(estimated_data, reference_data)
t3 = time()
print('Providing Data: ', t2 - t1)
print('Evaluation: ', t3 - t2)
