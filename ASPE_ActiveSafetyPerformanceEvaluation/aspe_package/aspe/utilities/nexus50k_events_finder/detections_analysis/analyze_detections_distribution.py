import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

dets_p = r"E:\logfiles\nexus_50k_eval_data\auto_gt_and_detections\cta_rRf360t7020309v205p50_k09_g1E03b1E02qe05_detections.csv"
auto_gt_p = r"E:\logfiles\nexus_50k_eval_data\auto_gt_and_detections\cta_rRf360t7020309v205p50_k09_g1E03b1E02qe05_auto_gt.csv"
dets = pd.read_csv(dets_p, index_col=0)
auto_gt = pd.read_csv(auto_gt_p, index_col=0)

# dets.query('-7.5 < position_x & position_x < 2.5 & position_y < 0', inplace=True)
plt.figure()
heat_map_x = dets.tcs_norm_pos_y.to_numpy()
heat_map_y = dets.tcs_norm_pos_x.to_numpy()
heatmap, xedges, yedges = np.histogram2d(heat_map_x, heat_map_y, bins=100)
extent = [xedges[0], xedges[-1], yedges[0], yedges[-1]]
plt.imshow(heatmap.T, extent=extent, origin='lower')
plt.show()