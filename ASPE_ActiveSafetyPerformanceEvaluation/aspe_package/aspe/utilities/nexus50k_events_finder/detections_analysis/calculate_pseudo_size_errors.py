import pandas as pd

from aspe.utilities.nexus50k_events_finder.detections_analysis.extract_dets_info_to_csv import calculate_psuedo_size_err

if __name__ == '__main__':
    dets_p = r"E:\logfiles\nexus_size_estimation_analysis\cta_rRf360t7020309v205p50_SW_7_02_detections.csv"
    auto_gt_p = r"E:\logfiles\nexus_size_estimation_analysis\cta_rRf360t7020309v205p50_SW_7_02_auto_gt.csv"

    dets = pd.read_csv(dets_p, index_col=0)
    auto_gt = pd.read_csv(auto_gt_p, index_col=0)

    auto_gt = calculate_psuedo_size_err(dets, auto_gt)
    gt_save_p = auto_gt_p
    auto_gt.to_csv(gt_save_p)