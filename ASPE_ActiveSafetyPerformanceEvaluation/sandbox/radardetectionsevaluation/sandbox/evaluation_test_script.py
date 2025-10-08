from copy import deepcopy

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy.stats import chi2
from radardetseval.data_provider.F360MudpRTRangeSensorDP import F360MudpRTRangeSensorDP
from sandbox.PEPipeline import pre_process_data, gating, nees_3d_dets_features, association, evaluate_single_sensor, \
    evaluate, evaluate_multi_log
from sandbox.private.user_data import ExampleData

# Data config
data_path = ExampleData.rt_range_data_path + r"\LSS\rRf360t4100309v205p50_2_15\FTP201_TC1_160040_001_rRf360t4100309v205p50_2_15.mudp"
data_paths = [ExampleData.rt_range_data_path + r"\LSS\rRf360t4100309v205p50_2_15\FTP201_TC1_160040_001_rRf360t4100309v205p50_2_15.mudp",
              ExampleData.rt_range_data_path + r"\RCTA\rRf360t4100309v205p50_2_15\FTP402_TC1_90_151006_001_rRf360t4100309v205p50_2_15.mudp"]
dvl_parser_config = r"configs\dvl_data_parser_config_RT3000.json"
mudp_parser_config = r"configs\mudp_data_parser_config.json"
mudp_stream_def_path = ExampleData.mudp_stream_def_path
data_provider = F360MudpRTRangeSensorDP(mudp_parser_config, dvl_parser_config, mudp_stream_def_path)

# First evaluation
estimated_data, reference_data = data_provider.get_single_log_data(data_path)
est_synch, ref_synch = pre_process_data(estimated_data, reference_data, 4)
output = evaluate_single_sensor(est_synch, ref_synch)

# Second evaluation
out_all = evaluate(estimated_data, reference_data)

# Third evaluation
multi_log_output = evaluate_multi_log(data_paths, data_provider)

# Postprocessing
output_df = multi_log_output['pairs_df']

df_not_associated = output_df[np.logical_not(output_df['f_associated'])]
df_associated = output_df[output_df['f_associated']]
df_corner = df_associated[df_associated['dof'] == 3]
df_side = df_associated[df_associated['dof'] == 2]
df_inside = df_associated[df_associated['dof'] == 1]

plt.figure()
df_inside['p_value'].hist(bins='auto', density=True)
plt.figure()
df_inside['deviation_azimuth_deg'].hist(bins='auto', density=True)
plt.figure()
df_inside['deviation_range_rate'].hist(bins='auto', density=True)

plt.figure()
df_side['p_value'].hist(bins='auto', density=True)

