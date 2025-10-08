import matplotlib.pyplot as plt
import numpy as np

from aspe.evaluation.RadarDetectionEvaluation.DetectionEvaluation import DetectionEvaluation
from aspe.examples.evaluation.private.user_data import ExampleData
from aspe.extractors.ReferenceExtractor.RtRange.rt_3000_msg_sets_config import rt_3000_message_sets_config_ch_4
from aspe.providers.F360MudpRTRangeSensorDP import F360MudpRTRangeSensorDP

data_path = ExampleData.rt_range_data_path + (r"\LSS\rRf360t4100309v205p50_2_15"
                                              r"\FTP201_TC1_160040_001_rRf360t4100309v205p50_2_15.mudp")
data_paths = [
    ExampleData.rt_range_data_path + r"\LSS\rRf360t4100309v205p50_2_15"
                                     r"\FTP201_TC1_160040_001_rRf360t4100309v205p50_2_15.mudp",
    ExampleData.rt_range_data_path + r"\RCTA\rRf360t4100309v205p50_2_15"
                                     r"\FTP402_TC1_90_151006_001_rRf360t4100309v205p50_2_15.mudp"]
mudp_stream_def_path = ExampleData.mudp_stream_def_path
dbc_config = {4: 'configs\\RT_range_target_merged_working.dbc'}

data_provider = F360MudpRTRangeSensorDP(dbc_config, rt_3000_message_sets_config_ch_4, mudp_stream_def_path)

# First evaluation
detection_evaluation = DetectionEvaluation()
estimated_data, reference_data = data_provider.get_single_log_data(data_path)
output_single_sensor = detection_evaluation.evaluate_single_sensor(estimated_data, reference_data, 4)

# Second evaluation
# output_single_log = detection_evaluation.evaluate_single_data(estimated_data, reference_data)

# Third evaluation
# output_multi_log = detection_evaluation.evaluate_multi_log(data_paths, data_provider)

# Postprocessing
output_df = output_single_sensor['pairs_df']

df_not_associated = output_df[np.logical_not(output_df['f_associated'])]
df_associated = output_df[output_df['f_associated']]
df_corner = df_associated[df_associated['dof'] == 3]
df_side = df_associated[df_associated['dof'] == 2]
df_inside = df_associated[df_associated['dof'] == 1]

plt.figure()
df_inside['p_value'].hist(bins='auto', density=True)
plt.figure()
df_inside['deviation_azimuth'].hist(bins='auto', density=True)
plt.figure()
df_inside['deviation_range_rate'].hist(bins='auto', density=True)

plt.figure()
df_side['p_value'].hist(bins='auto', density=True)
