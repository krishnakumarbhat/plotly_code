import re
from pathlib import Path
from typing import Dict

import pandas as pd
from tqdm import tqdm

from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluationConfig import Nexus50kEvaluationConfig
from aspe.utilities.nexus50k_events_finder.pe_output_aggregator import PEOutputAggregator
from aspe.utilities.nexus50k_events_finder.user_config import user_config
from aspe.utilities.SupportingFunctions import load_from_pkl


def aggregate_results(config: Nexus50kEvaluationConfig, event_type: str, resim_dir_name: str):
    pe_output_paths = [str(p) for p in Path(config.PERF_EVAL_OUT_DIR).glob(f'{event_type}_events_*{resim_dir_name}_pe_output.pickle')]
    aggregator = PEOutputAggregator()
    for p in tqdm(pe_output_paths):
        ev_index = int(re.search(f'{event_type}_events_(.+?)_', p).group(1))
        pe_out = load_from_pkl(p)

        aggregator.add(pe_out, resim_dir_name, ev_index)

    output = aggregator.aggregate(resim_dir_name)

    save_to_excel(dir_to_save=config.PERF_EVAL_OUT_DIR,
                  file_name=f'{event_type}_events_kpis.xlsx',
                  sheet_to_df=output)
    return output


def save_to_excel(dir_to_save, file_name, sheet_to_df: Dict[str, pd.DataFrame]):
    excel_path = Path(dir_to_save) / file_name
    mode = 'w'
    if_sheet_exists = None

    if excel_path.exists():
        mode = 'a'  # append
        if_sheet_exists = 'replace'

        excel_sheets = pd.read_excel(excel_path, sheet_name=None)
        for sheet_name, df in excel_sheets.items():

            if sheet_name in sheet_to_df:
                df_to_join = sheet_to_df[sheet_name]

                if sheet_name == 'weighted_mean_pairs_kpis':
                    keys_to_merge = ['feature_signature', 'kpi_signature']
                elif sheet_name == 'log_features_sum':
                    keys_to_merge = ['signature']
                elif sheet_name == 'pairs_kpis_per_event':
                    keys_to_merge = ['feature_signature', 'kpi_signature', 'event_index']
                elif sheet_name == 'log_features_per_event':
                    keys_to_merge = ['signature', 'event_index']
                else:
                    raise KeyError(f'Aggregation of sheet: {sheet_name} is not handled')

                sheet_to_df[sheet_name] = df.join(df_to_join.set_index(keys_to_merge), on=keys_to_merge)

            else:
                sheet_to_df[sheet_name] = df

    with pd.ExcelWriter(excel_path, engine='openpyxl', mode=mode, if_sheet_exists=if_sheet_exists) as writer:
        for sheet_name, df in sheet_to_df.items():
            df.to_excel(excel_writer=writer, sheet_name=sheet_name, index=False)


if __name__ == '__main__':
    event_type = 'close_trucks'

    resim_names = ['rRf360t7060309v205p50_dev_baseline',
                   ]

    for resim_name in resim_names:
        aggregate_results(config=user_config,
                          event_type=event_type,
                          resim_dir_name=resim_name)
