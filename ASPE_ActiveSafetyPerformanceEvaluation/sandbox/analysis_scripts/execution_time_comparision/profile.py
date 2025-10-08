from warnings import warn

import pandas as pd
from tqdm import tqdm

from extract_mudp import local_mudp_load, save_to_pickle, walk_and_find_files_with_ext


def extract_min_max_mean(mudp_path, mudp_config_path, *args, **kwargs):
    extr_data = local_mudp_load(mudp_path, mudp_config_path)

    try:
        execution_time_signals = extr_data['data']['F360ExecutionTimeInfo']['signals']
    except:
        execution_time_signals = extr_data.data.execution_time_info.signals

    execution_time_signals = execution_time_signals.set_index('scan_index')

    clus_prof = execution_time_signals

    c_min, c_max, c_mean = clus_prof.min(), clus_prof.max(), clus_prof.mean()
    return c_min, c_max, c_mean


def profile_diff(mudp_path1, mudp_path2, mudp_config_path1, mudp_config_path2, norm=True, *args, **kwargs):
    """
    Provide dataframe with diff of execution time signals
    :param mudp_path1:
    :param mudp_path2:
    :param mudp_config_path1:
    :param mudp_config_path2:
    :return:
    """
    c_min1, c_max1, c_mean1 = extract_min_max_mean(mudp_path1, mudp_config_path1)
    c_min2, c_max2, c_mean2 = extract_min_max_mean(mudp_path2, mudp_config_path2)

    c_diff_min1, c_diff_max1, c_diff_mean1 = c_min2 - c_min1, c_max2 - c_max1, c_mean2 - c_mean1

    norm_min_diff, norm_max_diff, norm_mean_diff = c_diff_min1.divide(c_min1), c_diff_max1.divide(
        c_max1), c_diff_mean1.divide(c_mean1)

    if norm:
        return norm_min_diff, norm_max_diff, norm_mean_diff
    else:
        return c_diff_min1, c_diff_max1, c_diff_mean1


def handle_df_join(df, other_df, file_name):
    if df.empty:
        df = df.append(pd.DataFrame(data=other_df, columns=[file_name]))
    else:
        df = df.join(pd.DataFrame(data=other_df, columns=[file_name]))
    return df


def aggregate_diff_profile(folder_path, resim_tag_a, resim_tag_b, conf_path1=None, conf_path2=None, ext='.mudp',
                           norm=True):
    """
    Profile 2 files by getting one with resim_tag_a and replace this tag with resim_tag_b, if second file is found
    profiling if proceeding
    :param folder_path:
    :param resim_tag_a:
    :param resim_tag_b:
    :param ext:
    :return:
    """
    df_min, df_max, df_mean = pd.DataFrame(), pd.DataFrame(), pd.DataFrame()
    pkl_files = walk_and_find_files_with_ext(folder_path, ext)

    for st_file in tqdm(pkl_files):
        if resim_tag_a in st_file:
            nd_file = st_file.replace(resim_tag_a, resim_tag_b)
            try:
                norm_min_diff, norm_max_diff, norm_mean_diff = profile_diff(st_file, nd_file, conf_path1, conf_path2,
                                                                            norm=norm)

                df_min = handle_df_join(df_min, norm_min_diff, st_file)
                df_max = handle_df_join(df_max, norm_max_diff, st_file)
                df_mean = handle_df_join(df_mean, norm_mean_diff, st_file)

            except FileNotFoundError:
                warn(f'not matching files for log: {st_file}')

    return df_min.T, df_max.T, df_mean.T


def aggregate_tag_parameter(folder_path, resim_tag_a, resim_tag_b, conf_path1=None, conf_path2=None, ext='.pickle'):
    """
    Profile 2 files by getting one with resim_tag_a and replace this tag with resim_tag_b, if second file is found
    profiling if proceeding
    :param folder_path:
    :param resim_tag_a:
    :param resim_tag_b:
    :param ext:
    :return:
    """
    df_min, df_max, df_mean = pd.DataFrame(), pd.DataFrame(), pd.DataFrame()
    pkl_files = walk_and_find_files_with_ext(folder_path, ext)

    for st_file in tqdm(pkl_files):
        if resim_tag_a in st_file and resim_tag_b not in st_file:
            try:
                norm_min_diff, norm_max_diff, norm_mean_diff = extract_min_max_mean(st_file, conf_path1, conf_path2)

                df_min = handle_df_join(df_min, norm_min_diff, st_file)
                df_max = handle_df_join(df_max, norm_max_diff, st_file)
                df_mean = handle_df_join(df_mean, norm_mean_diff, st_file)

            except FileNotFoundError:
                warn(f'not matching files for log: {st_file}')

    return df_min.T, df_max.T, df_mean.T


if __name__ == '__main__':
    # # PROFILE SINGLE FILE FROM MUDP
    # mudp_config1 = r"C:\Users\mj2snl\Desktop\aspe_local\cea-258\parsers_config\mudp_data_parser_config.json"
    # mudp_config2 = r"C:\Users\mj2snl\Desktop\aspe_local\cea-258\parsers_config\mudp_data_parser_config.json"
    #
    # p1 = r"C:\Users\mj2snl\Documents\logs\logs\clust_log_set\rRf360t4090309v205p50\2p13pE_27c2132_20191127_110457_002_rRf360t4090309v205p50_MUDP_Extracted.pickle"
    # p2 = r"C:\Users\mj2snl\Documents\logs\logs\clust_log_set\rRf360t4090309v205p50m_cl_gr\2p13pE_27c2132_20191127_110457_002_rRf360t4090309v205p50m_cl_gr_MUDP_Extracted.pickle"
    #
    # norm_min_diff, norm_max_diff, norm_mean_diff = profile_diff(p1, p2, None, None)
    # ###

    # PROFILE FROM FOLDER
    folder_path = r'C:\Users\mj2snl\Documents\logs\logs\kombajn_tst'
    resim_tag_a = 'rRf360t4090309v205p50orig'
    resim_tag_b = 'rRf360t4090309v205p50m_cl_gr'

    mudp_config1 = r"C:\Users\mj2snl\Desktop\aspe_local\cea-258\parsers_config\mudp_data_parser_config.json"
    mudp_config2 = r"C:\Users\mj2snl\Desktop\aspe_local\cea-258\parsers_config\mudp_data_parser_config.json"
    #
    results = aggregate_diff_profile(folder_path, resim_tag_a, resim_tag_b, mudp_config1, mudp_config2, ext='.mudp', norm=False)
    save_to_pickle(folder_path + r'\aggr_raw_diffs_dataframes.pickle', results)

    results = aggregate_diff_profile(folder_path, resim_tag_a, resim_tag_b, mudp_config1, mudp_config2, ext='.mudp', norm=True)
    save_to_pickle(folder_path + r'\aggr_norm_diffs_dataframes.pickle', results)

    results = aggregate_tag_parameter(folder_path, resim_tag_a, resim_tag_b, mudp_config1, mudp_config2, ext='.mudp')
    save_to_pickle(folder_path + r'\aggr_tag1_raw_params_dataframes.pickle', results)

    results = aggregate_tag_parameter(folder_path, resim_tag_b, resim_tag_a, mudp_config1, mudp_config2, ext='.mudp')
    save_to_pickle(folder_path + r'\aggr_tag2_raw_params_dataframes.pickle', results)
    ###
