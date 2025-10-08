from analysis_scripts.DEX80_missalignment_analysis.resim_with_different_alignment_correction import resim_log_list_with_different_alignment_correction
from analysis_scripts.DEX80_missalignment_analysis.run_evaluation_on_dataset import run_evaluation_for_scenario
import os

if __name__ == "__main__":
    #---------------------------------------------RCTA------------------------------------------------------------------
    # RESIM
    base_ini_path = r"P:\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\F360Tracker_RNA_SUV_Espace.ini"
    resim_exe_path = r"C:\wkspaces_git\F360Core\F360TrackerPC_SRR_ESR_vs2015\output\Debug_Win32\resim_f360.exe"
    log_list_path = r"P:\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\Analysis\alignment_analysis_log_list_RCTA.txt"
    corr_angles_vec = [-3.0, -2.5, -2.0, -1.8, -1.6, -1.4, -1.2, -1.0, -0.8, -0.6, -0.5,
                       -0.4, -0.3, -0.2, -0.1, 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.8, 1.0,
                       1.2, 1.4, 1.6, 1.8, 2.0, 2.5, 3.0]
    resim_log_list_with_different_alignment_correction(resim_exe_path, base_ini_path, log_list_path, corr_angles_vec)

    # EVALUATION
    scenario_folder_path = r"P:\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\RCTA_FTP_402"
    agg_data_paths = run_evaluation_for_scenario(scenario_folder_path)

    with open(os.path.join(scenario_folder_path, 'case_2_aggregated_PE_data_paths.txt'), 'w') as file:
        file.writelines('\n'.join(agg_data_paths))


    #-----------------------------------------------LSS-----------------------------------------------------------------
    log_list_path = r"P:\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\Analysis\alignment_analysis_log_list_LSS.txt"
    resim_log_list_with_different_alignment_correction(resim_exe_path, base_ini_path, log_list_path, corr_angles_vec)

    # EVALUATION
    scenario_folder_path = r"P:\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\LSS_FTP_201"
    agg_data_paths = run_evaluation_for_scenario(scenario_folder_path)

    with open(os.path.join(scenario_folder_path, 'case_2_aggregated_PE_data_paths.txt'), 'w') as file:
        file.writelines('\n'.join(agg_data_paths))
