from analysis_scripts.DEX80_missalignment_analysis.utils import load_pickle_file, save_pickle_file
from analysis_scripts.DEX80_missalignment_analysis.plotting_functions.deviations_plots import get_deviations_plots_in_range_fun, get_far_ranges_deviations_plots
from analysis_scripts.DEX80_missalignment_analysis.plotting_functions.binary_classification_plots import get_bin_class_plots_in_range_fun, get_far_ranges_bin_class_plots
from analysis_scripts.DEX80_missalignment_analysis.plotting_functions.plot_traces import plot_traces

SAVE_FIG_FOLDER = r'P:\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\Analysis\analysis_plots\RCTA_plots_same_angles'
CRITICAL_ERROR_POS_Y = 2.0
CRITICAL_ERROR_POS_X = 2.0
CRITICAL_ERROR_VEL_Y = 1.5
CRITICAL_ERROR_VEL_X = 1


if __name__ == "__main__":
    rcta_data_path = r"C:\projects\F360\DEX80_misalign_analysis_data_RCTA_case_2.pickle"
    rcta_data = load_pickle_file(rcta_data_path)

    ref_data = rcta_data['reference_obj_data']
    est_data = rcta_data['estimated_obj_data']

    range_low = 40
    range_high = 50

    get_far_ranges_deviations_plots(ref_data, 'deviation_pos_tcsy', range_low, range_high,
                                    'Position Y deviation in target coordinate system - RCTA', CRITICAL_ERROR_POS_Y)
    get_far_ranges_deviations_plots(ref_data, 'deviation_pos_tcsx', range_low, range_high,
                                    'Position X deviation in target coordinate system - RCTA', CRITICAL_ERROR_POS_X)

    get_far_ranges_deviations_plots(ref_data, 'deviation_vel_tcsy', range_low, range_high,
                                    'Velocity Y deviation in target coordinate system - RCTA', CRITICAL_ERROR_VEL_Y)
    get_far_ranges_deviations_plots(ref_data, 'deviation_vel_tcsx', range_low, range_high,
                                    'Velocity X deviation in target coordinate system - RCTA', CRITICAL_ERROR_VEL_X)

    get_far_ranges_bin_class_plots(ref_data, est_data, range_low, range_high)
    get_bin_class_plots_in_range_fun(ref_data, est_data)
    get_deviations_plots_in_range_fun(ref_data, 'deviation_pos_tcsy',
                                      'Position Y deviation in target coordinate system - RCTA', CRITICAL_ERROR_POS_Y)
    get_deviations_plots_in_range_fun(ref_data, 'deviation_pos_tcsx',
                                      'Position X deviation in target coordinate system - RCTA', CRITICAL_ERROR_POS_X)
    get_deviations_plots_in_range_fun(ref_data, 'deviation_vel_tcsy',
                                      'Velocity Y deviation in target coordinate system - RCTA', CRITICAL_ERROR_VEL_Y)
    out_ref_data_filtered = get_deviations_plots_in_range_fun(ref_data, 'deviation_vel_tcsx',
                                      'Velocity X deviation in target coordinate system - RCTA', CRITICAL_ERROR_VEL_X)
    plot_traces(out_ref_data_filtered, 'RCTA scenario reference object traces')