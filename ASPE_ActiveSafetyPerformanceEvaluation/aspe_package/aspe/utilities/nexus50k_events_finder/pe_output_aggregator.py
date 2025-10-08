from typing import Optional

import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PEObjectsEvaluationOutput import (
    PEObjectsEvaluationOutputMultiLog,
)


class PEOutputAggregator:
    def __init__(self):
        self.pairs_kpis_agg = []
        self.bin_class_kpis_agg = []
        self.log_features_agg = []
        self.events_info_agg = []
        self.counter = 0

    def add(self, pe_out: PEObjectsEvaluationOutputMultiLog, event):

        def add_event_type_and_event_index_columns(df: pd.DataFrame, event_type: str, event_index: int):
            df['event_type'] = event_type
            df['event_index'] = event_index
            return df

        event_type, ev_index = event.name, event.index
        pairs_kpis = add_event_type_and_event_index_columns(pe_out.kpis_pairs_features_aggregated, event_type, ev_index)
        bin_class_kpis = add_event_type_and_event_index_columns(pe_out.kpis_binary_class_aggregated, event_type,
                                                                ev_index)
        log_features = add_event_type_and_event_index_columns(pe_out.logs_features_aggregated, event_type, ev_index)
        event_info = pd.DataFrame(event.to_dict(), index=[self.counter])

        self.pairs_kpis_agg.append(pairs_kpis)
        self.bin_class_kpis_agg.append(bin_class_kpis)
        self.log_features_agg.append(log_features)
        self.events_info_agg.append(event_info)

        self.counter += 1

    def aggregate(self, sw_signature: Optional[str] = None):
        if self.counter == 0:
            return

        pairs_kpis_agg = pd.concat(self.pairs_kpis_agg)
        bin_class_kpis_agg = pd.concat(self.bin_class_kpis_agg)
        log_features_agg = pd.concat(self.log_features_agg)
        events_info_agg = pd.concat(self.events_info_agg).rename(columns={'index': 'event_index'})

        # weighted mean pairs_features kpis
        grouped = pairs_kpis_agg.groupby(by=['feature_signature', 'kpi_signature', 'flag_signature'])
        if sw_signature is None:  # this branch needed for aggregation few resims versions results to single .xls file
            kpi_col_name = 'kpi_value'
        else:
            kpi_col_name = sw_signature
        weighted_mean_pairs_kpis = grouped.apply(
            lambda df: (df.kpi_value * df.n_samples).sum() / df.n_samples.sum()).reset_index(name=kpi_col_name)

        # now add sum of KPIs samples per flag filter
        grouped_by_flag = pairs_kpis_agg.groupby(by='flag_signature')
        for flag, group in grouped_by_flag:
            flag_samples = group.loc[~group.set_index('event_index').index.duplicated(keep='first'), 'n_samples'].sum()
            weighted_mean_pairs_kpis = weighted_mean_pairs_kpis.append({'feature_signature': 'samples_count',
                                                                        'kpi_signature': 'count',
                                                                        'flag_signature': flag,
                                                                        kpi_col_name: flag_samples}, ignore_index=True)

        pairs_kpis_per_event = \
            pairs_kpis_agg.loc[:, ['feature_signature', 'kpi_signature', 'event_index', 'kpi_value']].rename(
                columns={'kpi_value': kpi_col_name})
        log_features_per_event = log_features_agg.loc[:, ['signature', 'event_index', 'value']].rename(
            columns={'value': kpi_col_name})
        log_features_sum = log_features_agg.groupby(by='signature').apply(lambda df: df.value.sum()).reset_index(
            name=kpi_col_name)

        return {
            'weighted_mean_pairs_kpis': weighted_mean_pairs_kpis,
            'log_features_sum': log_features_sum,
            'pairs_kpis_per_event': pairs_kpis_per_event.sort_values(by='event_index').reset_index(drop=True),
            'log_features_per_event': log_features_per_event.sort_values(by='event_index').reset_index(drop=True),
            'events_info': events_info_agg,
        }
