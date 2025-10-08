import pandas as pd
from aspe.report_generator.BaseBuilder import BaseBuilder


class BinaryClassificationBuilder(BaseBuilder):
    def __init__(self, input_data, template_path, plotter):
        super().__init__(template_path, plotter)
        self.input_data = input_data

        self._transform_data()
        self.init_template_args()
        self.build_results()
        self.render_html()

    def init_template_args(self):
        self.template_args['agg_results_plots'] = []
        self.template_args['agg_kpi_table'] = str
        self.template_args['agg_bin_results_bar_plot'] = str
        self.template_args['conf_matrixes'] = {}
        self.template_args['per_log_plots'] = []
        self.template_args['best_worst_cases_tabs'] = []
        pd.set_option('precision', 3)
        self.env.globals.update(zip=zip)

    def build_results(self):
        agg_kpis_bar_plot_path = self.plotter.bar(self.kpis_agg_results, 'Binary classification KPIs', ylabel='Value [-]',
                                                  ylim=[0, 1.05], figsize=(10, 6), legend_loc=4)

        agg_bin_class_plot_path = self.plotter.bar(self.log_features_results, 'Binary classification objects count',
                                                   ylabel='Value [-]', figsize=(10, 6))

        kpis_per_log = self.input_data['kpis_binary_class_per_log']
        per_log_box_plot_path = self.plotter.box_xy_hue(kpis_per_log, x_col='signature', y_col='value', hue_col='sw_signature',
                                                        title='KPIs per log', ylim=[0, 1.05], figsize=(10, 12), ylabel='KPI value [-]')

        self.template_args['agg_results_plots'].append(agg_kpis_bar_plot_path)
        self.template_args['agg_kpi_table'] = self.kpis_agg_results.to_html(index_names=False)
        self.template_args['agg_bin_results_bar_plot'] = agg_bin_class_plot_path
        self.template_args['per_log_plots'].append(per_log_box_plot_path)
        self.template_args['conf_matrices'] = self.conf_matrices
        self.template_args['best_worst_cases_tabs'] = self.best_worst_cases

    def _transform_data(self):
        self._prepare_aggregated_kpis()
        self._prepare_log_features()
        self._prepare_best_worst_cases_tabs()
        self._prepare_confusion_matrices()

    def _prepare_aggregated_kpis(self):
        kpis_agg = self.input_data['kpis_binary_class_aggregated']
        kpis_agg_results = pd.DataFrame()
        for kpi_signature, group in kpis_agg.groupby(by='signature'):
            partial = group.pivot_table(values='value', columns='sw_signature', aggfunc='first')
            partial.index = [kpi_signature]
            kpis_agg_results = kpis_agg_results.append(partial)
        self.kpis_agg_results = kpis_agg_results

    def _prepare_log_features(self):
        relevant_log_features = ['FN_count', 'FP_count', 'TP_count']
        log_features_agg = self.input_data['logs_features_aggregated']
        log_features_agg = log_features_agg.loc[log_features_agg.loc[:, 'signature'].isin(relevant_log_features), :]

        log_features_results = pd.DataFrame()
        for feature_signature, group in log_features_agg.groupby(by='signature'):
            partial = group.pivot_table(values='value', columns='sw_signature')
            partial.index = [feature_signature]
            log_features_results = log_features_results.append(partial)
        self.log_features_results = log_features_results

    def _prepare_best_worst_cases_tabs(self):
        n_best_worst = 5
        kpis_per_log = self.input_data['kpis_binary_class_per_log']
        log_features_per_log = self.input_data['logs_features_per_log']

        relevant_log_features = ['FN_count', 'FP_count', 'TP_count']
        rel_signatures = relevant_log_features + ['TPR', 'PPV', 'F1_score']
        temp_df = kpis_per_log.append(log_features_per_log)
        temp_df = temp_df.loc[temp_df.loc[:, 'signature'].isin(rel_signatures), :]
        best_worst_df = pd.DataFrame()
        for (log_idx, sw_sign), group in temp_df.groupby(by=['log_index', 'sw_signature']):
            trans = group.drop(columns=['log_index', 'sw_signature']).T
            trans.columns = trans.loc['signature', :]
            trans = trans.drop(index='signature').reset_index(drop=True)
            log_sw_df = pd.DataFrame({'log_index': [log_idx], 'sw_signature': [sw_sign]})
            record = pd.concat([log_sw_df, trans], sort=False, axis=1)
            best_worst_df = best_worst_df.append(record).reset_index(drop=True)
        best_worst_data = {}
        for sw_signature, group in best_worst_df.groupby(by='sw_signature'):
            group.drop(columns='sw_signature', inplace=True)
            best_f1 = group.sort_values(by=['F1_score'], ascending=False).head(n_best_worst)
            worst_f1 = group.sort_values(by=['F1_score'], ascending=True).head(n_best_worst)
            best_worst_data[sw_signature] = {'Logs with best F1 score values: ': best_f1.to_html(index=False),
                                             'Logs with worst F1 score values: ': worst_f1.to_html(index=False)}
        self.best_worst_cases = best_worst_data

    def _prepare_confusion_matrices(self):
        agg_log_features = self.input_data['logs_features_aggregated'].set_index('signature', drop=True)
        conf_mat_data = {}
        df_idxs = ['Estimated exists', 'Estimated not exists']
        for sw_signature, group in agg_log_features.groupby(by='sw_signature'):
            tp_count = str(group.loc['TP_count', 'value']) + ' TP'
            fn_count = str(group.loc['FN_count', 'value']) + ' FN'
            fp_count = str(group.loc['FP_count', 'value']) + ' FP'
            df = pd.DataFrame({'Reference exists': [tp_count, fn_count],
                               'Reference not exists': [fp_count, '-']}, index=df_idxs)
            conf_mat_data[sw_signature] = df.to_html(index_names=False)
        self.conf_matrices = conf_mat_data


if __name__ == "__main__":
    import pickle
    in_path = r"C:\wkspaces\ASPE0000_00_Common\.private\ASPE_reports\ASPE_reporting_input.pickle"
    with open(in_path, 'rb') as f:
        data = pickle.load(f)
    bcb = BinaryClassificationBuilder(data)
