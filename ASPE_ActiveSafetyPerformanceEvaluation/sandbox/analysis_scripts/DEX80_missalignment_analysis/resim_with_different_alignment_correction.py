from sandbox.resim.F360ResimRunner import F360ResimRunner
from sandbox.resim.F360ResimRunner import TxtReader
from analysis_scripts.DEX80_missalignment_analysis.change_ini_file import create_new_ini_file_with_align_corr_angle


def resim_log_list_with_different_alignment_correction(resim_exe_path, base_ini, log_list_path, corr_angles_vec):
    resim_runner = F360ResimRunner(resim_exe_path)
    log_list = TxtReader.get_multi_lines(log_list_path)
    for corr_angle in corr_angles_vec:
        changed_ini_path = create_new_ini_file_with_align_corr_angle(base_ini, corr_angle)
        corr_angle_str = str(corr_angle).replace('.', '_').replace('-', 'neg_')
        arguments = f'-osuffix _case_2_align_cor_{corr_angle_str}'
        resim_runner.resim_log_list(log_list, changed_ini_path, arguments)


if __name__ == "__main__":
    base_ini_path = r"P:\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\F360Tracker_RNA_SUV_Espace.ini"
    resim_exe_path = r"C:\wkspaces_git\F360Core\F360TrackerPC_SRR_ESR_vs2015\output\Debug_Win32\resim_f360.exe"
    log_list_path = r"P:\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\Analysis\alignment_analysis_log_list_LSS.txt"
    corr_angles_vec = [-3.0, -2.5, -2.0, -1.8, -1.6, -1.4, -1.2, -1.0, -0.8, -0.6, -0.5,
                       -0.4, -0.3, -0.2,  -0.1, 0.0,  0.1, 0.2,  0.3,  0.4,  0.5,  0.6,  0.8,  1.0,
                       1.2,  1.4,  1.6,  1.8,  2.0,  2.5,  3.0]
    resim_log_list_with_different_alignment_correction(resim_exe_path, base_ini_path, log_list_path, corr_angles_vec)