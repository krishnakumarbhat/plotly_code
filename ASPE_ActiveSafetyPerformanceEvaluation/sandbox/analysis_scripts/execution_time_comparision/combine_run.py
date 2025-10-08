import sys
sys.path.insert(0, r'C:\Users\mj2snl\wkspaces\ASPE0000_00_Common\sandbox\resim')
from F360ResimRunner import TxtReader, F360ResimRunner

import shutil
from extract_mudp import save_to_pickle
from profile import aggregate_diff_profile, aggregate_tag_parameter


log_paths = TxtReader.get_multi_lines(r"C:\Users\mj2snl\Documents\logs\logs\clust_log_set\log_list.txt")
ini_file = r"C:\Users\mj2snl\Documents\logs\logs\clust_log_set\F360Tracker.ini"

for i in range(3, 10):
    print('starts', i, 'iteration')
    resim_exe_path = r"C:\Users\mj2snl\Documents\logs\logs\Debug_Win32_m_cl_gr\resim_f360.exe"
    runner = F360ResimRunner(resim_exe_path)
    runner.resim_log_list(log_paths, ini_file, arguments='-osuffix m_cl_gr', trk_opts='-init_from_log -sync_input')

    resim_exe_path = r"C:\Users\mj2snl\Documents\logs\logs\Debug_Win32_orig\resim_f360.exe"
    runner = F360ResimRunner(resim_exe_path)
    runner.resim_log_list(log_paths, ini_file, arguments='-osuffix orig', trk_opts='-init_from_log -sync_input')


    folder_path = r'C:\Users\mj2snl\Documents\logs\logs\clust_log_set'
    resim_tag_a = 'rRf360t4090309v205p50orig'
    resim_tag_b = 'rRf360t4090309v205p50m_cl_gr'

    mudp_config1 = r"C:\Users\mj2snl\Desktop\aspe_local\cea-258\parsers_config\mudp_data_parser_config.json"
    mudp_config2 = r"C:\Users\mj2snl\Desktop\aspe_local\cea-258\parsers_config\mudp_data_parser_config.json"

    results = aggregate_diff_profile(folder_path, resim_tag_a, resim_tag_b, mudp_config1, mudp_config2, ext='.mudp', norm=False)
    save_to_pickle(folder_path + r'\aggr_raw_diffs_dataframes' + f'_{i}.pickle', results)

    results = aggregate_diff_profile(folder_path, resim_tag_a, resim_tag_b, mudp_config1, mudp_config2, ext='.mudp', norm=True)
    save_to_pickle(folder_path + r'\aggr_norm_diffs_dataframes' + f'_{i}.pickle', results)

    results = aggregate_tag_parameter(folder_path, resim_tag_a, resim_tag_b, mudp_config1, mudp_config2, ext='.mudp')
    save_to_pickle(folder_path + r'\aggr_tag1_raw_params_dataframes' + f'_{i}.pickle', results)

    results = aggregate_tag_parameter(folder_path, resim_tag_b, resim_tag_a, mudp_config1, mudp_config2, ext='.mudp')
    save_to_pickle(folder_path + r'\aggr_tag2_raw_params_dataframes' + f'_{i}.pickle', results)

    print('deleting folders...')
    try:
        shutil.rmtree(r'C:\Users\mj2snl\Documents\logs\logs\clust_log_set\rRf360t4090309v205p50m_cl_gr')
        shutil.rmtree(r'C:\Users\mj2snl\Documents\logs\logs\clust_log_set\rRf360t4090309v205p50orig')
    except:
        print('no delete :(')
