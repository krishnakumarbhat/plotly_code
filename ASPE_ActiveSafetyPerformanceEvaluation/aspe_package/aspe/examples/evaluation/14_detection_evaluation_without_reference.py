import time

import matplotlib.pyplot as plt
import numpy as np

from aspe.evaluation.RadarDetectionEvaluation.DetectionEvaluation import DetectionEvaluation
from aspe.evaluation.RadarDetectionEvaluation.Features.Nees3dFeatures import Ness3dFeatures
from aspe.evaluation.RadarDetectionEvaluation.NEES.optimizers import nearest_det_3d_nees_no_obj_cov
from aspe.evaluation.RadarDetectionEvaluation.PreProcesors.MudpPreProcessor import MudpPreProcessor

# Data config
from aspe.evaluation.RadarObjectsEvaluation.Flags import SignalInBoundsFlag
from aspe.examples.evaluation.private.user_data import ExampleData
from aspe.providers.F360MudpObjectAndSensorDP import F360MudpObjectAndSensorDP

data_path = ExampleData.rt_range_data_path + (r"\LSS\rRf360t4100309v205p50_2_15"
                                              r"\FTP201_TC1_160040_001_rRf360t4100309v205p50_2_15.mudp")
data_paths = [
    ExampleData.rt_range_data_path + r"\LSS\rRf360t4100309v205p50_2_15"
                                     r"\FTP201_TC1_160040_001_rRf360t4100309v205p50_2_15.mudp",
    ExampleData.rt_range_data_path + r"\RCTA\rRf360t4100309v205p50_2_15"
                                     r"\FTP402_TC1_90_151006_001_rRf360t4100309v205p50_2_15.mudp"]

mudp_stream_def_path = ExampleData.mudp_stream_def_path
data_provider = F360MudpObjectAndSensorDP(mudp_stream_def_path)

nees_features = Ness3dFeatures(optimizer=nearest_det_3d_nees_no_obj_cov)
# First evaluation
detection_evaluation = DetectionEvaluation(pre_processor=MudpPreProcessor(),
                                           pairs_features_list=[nees_features],
                                           association=SignalInBoundsFlag('p_value', min_value=0.00001))
estimated_data, reference_data = data_provider.get_single_log_data(data_path)
t1 = time.time()
output_single_sensor = detection_evaluation.evaluate_single_sensor(estimated_data, reference_data, 4)
t2 = time.time()
print(t2 - t1)
# Second evaluation
output_single_log = detection_evaluation.evaluate_single_data(estimated_data, reference_data)

# Third evaluation
output_multi_log = detection_evaluation.evaluate_multi_log(data_paths, data_provider)

# Postprocessing
output_df = output_single_sensor['pairs_df']

df_not_associated = output_df[np.logical_not(output_df['f_associated'])]
df_associated = output_df[output_df['f_associated']]
df_corner = df_associated[df_associated['dof'] == 3]
df_side = df_associated[df_associated['dof'] == 2]
df_inside = df_associated[df_associated['dof'] == 1]

plt.figure()
output_df['p_value'].hist(bins='auto', density=True)
plt.figure()
df_inside['deviation_range_rate'].hist(bins='auto', density=True)
plt.figure()
df_inside['p_value'].hist(bins='auto', density=True)
