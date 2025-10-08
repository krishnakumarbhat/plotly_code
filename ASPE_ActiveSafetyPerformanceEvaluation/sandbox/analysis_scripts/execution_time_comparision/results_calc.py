from extract_mudp import local_mudp_load, save_to_pickle
from copy import deepcopy
import pandas as pd


path = r"C:\Users\mj2snl\Documents\logs\logs\clust_log_set"
pre_name = f'aggr_tag2_raw_params_dataframes'
res_dict = {}
n = 4
for i in range(n):
    name1 = pre_name + f'_{i}.pickle'

    file_path = path + "\\" + name1

    r = local_mudp_load(file_path)
    res_dict[i] = r

output = []
for x in range(3):
    df_list = [i[x] for i in res_dict.values()]
    df_concat = pd.concat(df_list, sort=False)
    df_groupedby = df_concat.groupby(df_concat.index)
    output.append(df_groupedby.median())

save_to_pickle(path + "\\" + pre_name + '_median.pickle', output)
