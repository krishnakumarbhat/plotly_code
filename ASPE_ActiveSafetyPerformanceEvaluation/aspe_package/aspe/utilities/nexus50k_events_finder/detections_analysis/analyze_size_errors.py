from pathlib import Path

import pandas as pd

from aspe.utilities.nexus50k_events_finder.detections_analysis.size_error_analysis.calculate_features import (
    calculate_aspect_angle,
    calculate_range,
    calculate_speed,
)
from aspe.utilities.nexus50k_events_finder.detections_analysis.size_error_analysis.data_filters import (
    filter_only_moving,
)
from aspe.utilities.nexus50k_events_finder.detections_analysis.size_error_analysis.PlotConfig import CrossTrafficConfig
from aspe.utilities.nexus50k_events_finder.detections_analysis.size_error_analysis.SizeErrorPlotter import (
    SizeErrorPlotter,
)
from aspe.utilities.nexus50k_events_finder.detections_analysis.size_error_analysis.Statistic import Mean, Quantile


def load_auto_gt_data(csv_dir: Path, csv_base_name: str, resim_suffix: str):
    auto_gt_csv_filename = f"{csv_base_name}_{resim_suffix}_auto_gt.csv"
    auto_gt_p = csv_dir.joinpath(auto_gt_csv_filename)
    auto_gt = pd.read_csv(auto_gt_p, index_col=0)
    return auto_gt


if __name__ == '__main__':
    csv_dir = Path(r'E:\logfiles\nexus_50k_eval_data\auto_gt_and_detections')

    # specify statistics to compute & plot for each resim
    lines_to_plot = (Mean(), Quantile(0.05), Quantile(0.95))
    resims = [
        {'resim_suffix': "rRf360t7020309v205p50_SW_7_02",
         'signature': 'SW_7_02'},
        {'resim_suffix': "rRf360t7020309v205p50_k09_g1E03b1E02qe05",
         'signature': 'k09'},

        # {'resim_suffix': "rRf360t7020309v205p50_k10_g1E03b1E02qe05",
        #  'plots': [Mean()]},
    ]

    # choose CrossTrafficConfig / OvertakingConfig / StopAndGoConfig
    plot_config = CrossTrafficConfig(number_of_resims=len(resims))
    size_error_plotter = SizeErrorPlotter(plot_config=plot_config)

    for item in resims:
        auto_gt = load_auto_gt_data(csv_dir, plot_config.csv_base_name, item['resim_suffix'])

        # filter_only_cars(auto_gt)
        filter_only_moving(auto_gt)
        calculate_aspect_angle(auto_gt, use_degrees=True)
        calculate_speed(auto_gt)
        calculate_range(auto_gt)

        size_error_plotter.plot(auto_gt, lines_to_plot, item['signature'])

    size_error_plotter.legend()
    size_error_plotter.grid()
    size_error_plotter.show()
