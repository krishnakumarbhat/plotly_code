import re
from pathlib import Path

import numpy as np
from srr5_dev_tools import mgp_module, pkl_module
from srr5_dev_tools.common import NestedDict
from srr5_dev_tools.statistics import StatisticList
from tqdm import tqdm

eval_results_dir = Path(r'E:\1k\data\FASETH\ASP_output\eval_output')
# mdf_cache_dir = eval_results_dir.parents[1] / 'parse_result' / 'complete_data'
mdf_cache_dir = Path(r'D:\1k\data\FASETH\parse_result\complete_data')

relevant_logs = []
set_ids = set()
evaluations_list = list(eval_results_dir.glob('pe_output_*.pickle'))
for eval_result_path in tqdm(evaluations_list, 'Gathering logs'):
    # m = re.search(r'pe_output_(\d{8}_\d{6}).pickle', eval_result_path.name)
    m = re.search(r'pe_output_(20200710_152244).pickle', eval_result_path.name)
    if m is None:
        continue
    set_id = m.group(1)
    set_ids.add(set_id)
    # if '20200728_' in set_id or '20200727_' in set_id:
    #     continue
    relevant_logs.extend(mdf_cache_dir.glob(f'*_{set_id}_fas_*_SRR_Master-mPAD.mgp'))

summary_dist = 0
dt_stats = StatisticList()
vel_stats = StatisticList()
sets_summary = {}
for parsed_log_path in tqdm(relevant_logs, 'Calculating distance'):
    try:
        m = re.search(r'_(\d{8}_\d{6})_fas_', parsed_log_path.name)
        if m is None:
            continue
        set_id = m.group(1)
        mdf_file = mgp_module.load(parsed_log_path)
        data_structs = mdf_file['data']['RecogShortRangeRadarEgoVehicleSpeed']['EgoSpeed']
        time_array = []
        vel_array = []
        for time, struct in data_structs.items():
            time_array.append(time)
            vel_array.append(struct['longitudinal_speed_ego_vehicle']['value'])
        time_array = np.array(time_array)
        vel_array = np.array(vel_array)

        dt_array = time_array[1:] - time_array[:-1]
        valid_i = np.abs(dt_array) < 20
        vel_array = vel_array[:-1]
        dist_array = vel_array[valid_i] * dt_array[valid_i]

        detail_dist = dist_array.sum()
        summary_dist += detail_dist
        dt_stats.extend(dt_array)
        vel_stats.extend(vel_array)
        try:
            sets_summary[set_id]['sum'] += detail_dist
            sets_summary[set_id]['array'].extend(dist_array)
        except KeyError:
            sets_summary[set_id] = {
                'sum':   detail_dist,
                'array': list(dist_array),
            }
    except Exception as e:
        print(e)
        pass

summary = {
    'logs_amount':  len(relevant_logs),
    'dist_details': sets_summary,
    'dist':         summary_dist,
    'vel':          vel_stats,
    'dt':           dt_stats
}
pkl_module.save('stats_0.pkl', summary)
