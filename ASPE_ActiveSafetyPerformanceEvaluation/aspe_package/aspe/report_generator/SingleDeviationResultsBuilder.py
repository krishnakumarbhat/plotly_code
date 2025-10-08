import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification import BCType
from aspe.report_generator.Plotter import Plotter
from aspe.report_generator.BaseBuilder import BaseBuilder
from aspe.report_generator.support_functions import load_pickle
from aspe.report_generator.support_functions import calc_pdf
from aspe.report_generator.support_functions import calc_cdf
from aspe.report_generator.support_functions import calc_quantils


class SingleDeviationResultsBuilder(BaseBuilder):

    def __init__(self, data, feature_signature, section_name, unit, template_path, plotter):
        super().__init__(template_path, plotter)
        self.data = data
        self.feature_signature = feature_signature
        self.section_name = section_name
        self.unit = unit

        self.init_template_args()
        self._prepare_data_frames()
        self.build_results()
        self.render_html()

    def _prepare_data_frames(self):
        self._prepare_aggregated_kpis()
        self._prepare_agg_feature_per_soft_data()
        self._prepare_feature_cdf_pdf_data()
        self._prepare_kpis_per_log_data()
        self._prepare_best_worst_tables()

    def init_template_args(self):
        self.template_args['agg_kpi_plot'] = ''
        self.template_args['agg_results_plots'] = []
        self.template_args['per_log_plots'] = []
        self.template_args['best_worst_cases_tabs'] = {}
        self.template_args['quantile_table'] = ''
        self.template_args['section_name'] = self.section_name
        self.env.globals.update(zip=zip)
        pd.set_option('precision', 3)

    def build_results(self):
        kpis_bar_plot_path = self.plotter.bar(self.kpis_results_agg, title=self.section_name + ' KPIs',
                                              ylabel='KPI ' + self.unit, figsize=(10, 7))

        distribution_box_plot = self.plotter.box_xy(self.pairs_data, x_column=self.feature_signature, y_column='sw_signature',
                                                    title=self.section_name, xlabel='Deviation ' + self.unit, ylabel='')

        pdf_plot_path = self.plotter.pdf(self.pdf_data, title=self.section_name + ' PDF', xlabel='Deviation' + self.unit,
                                         ylabel='Density [-]')

        cdf_path = self.plotter.cdf(self.cdf_data, self.quantils_table, title=self.section_name + ' CDF',
                                    xlim=[0, self.cdf_data.index[-1]], xlabel='Absolute deviation' + self.unit, ylabel='Quantile [-]')

        per_log_boxes = self.plotter.box_xy_hue(self.kpis_per_log, x_col='kpi_signature', y_col='kpi_value',
                                                hue_col='sw_signature', title=self.section_name + ' KPI per log',
                                                figsize=(10, 10), xlabel='', ylabel='KPI value ' + self.unit)

        self.template_args['agg_kpi_bar_plot'] = kpis_bar_plot_path
        self.template_args['agg_kpis_table'] = self.kpis_results_agg_tab
        self.template_args['agg_results_plots'].append(distribution_box_plot)
        self.template_args['agg_results_plots'].append(pdf_plot_path)
        self.template_args['agg_results_plots'].append(cdf_path)
        self.template_args['quantile_table'] = self.quantils_table.to_html(index_names=False)
        self.template_args['per_log_plots'].append(per_log_boxes)
        self.template_args['best_worst_cases_tabs'] = self.best_worst_cases_html

    def _prepare_aggregated_kpis(self):
        kpis_agg = self.data['kpis_pairs_features_aggregated']
        kpis_agg = kpis_agg.loc[kpis_agg.loc[:, 'feature_signature'] == self.feature_signature].drop(columns='feature_signature')
        if 'deg' in self.unit:
            rad_signals_mask = [False if 'latency' in cell else True for cell in kpis_agg['kpi_signature']]
            kpis_agg.loc[rad_signals_mask, 'kpi_value'] = np.rad2deg(kpis_agg.loc[rad_signals_mask, 'kpi_value'].values.astype(np.float32))
        out_agg_kpis = pd.DataFrame()
        for kpi_signature, group in kpis_agg.groupby(by='kpi_signature'):
            partial = group.pivot_table(values='kpi_value', columns='sw_signature', aggfunc='first')
            partial.index = [kpi_signature]
            out_agg_kpis = out_agg_kpis.append(partial)
        index_upper = [idx.upper() for idx in list(out_agg_kpis.index)]
        idx_w_unit = [idx.upper() + ' ' + self.unit for idx in list(out_agg_kpis.index)]
        idx_w_unit = [idx.replace(self.unit, '[s]') if 'LATENCY' in idx else idx for idx in idx_w_unit]

        out_agg_kpis4table = out_agg_kpis.copy()
        out_agg_kpis4table.index = idx_w_unit
        out_agg_kpis.index = index_upper

        latency_rows_names = [idx for idx in out_agg_kpis4table.index if 'LATENCY' in idx]
        latency_rows = out_agg_kpis4table.loc[latency_rows_names, :]
        out_agg_kpis4table = out_agg_kpis4table.drop(index=latency_rows_names)
        out_agg_kpis4table = out_agg_kpis4table.sort_index().append(latency_rows)
        
        latency_rows_names = [idx for idx in out_agg_kpis.index if 'LATENCY' in idx]
        out_agg_kpis = out_agg_kpis.drop(index=latency_rows_names)
        self.kpis_results_agg = out_agg_kpis.sort_index()
        self.kpis_results_agg_tab = out_agg_kpis4table.to_html(index_names=False)

    def _prepare_agg_feature_per_soft_data(self):
        pe_results_obj_pairs = self.data['pe_results_obj_pairs']
        TP_mask = pe_results_obj_pairs['binary_classification'] == BCType.TruePositive
        pe_results_obj_pairs_TP_only = pe_results_obj_pairs.loc[TP_mask]
        pairs_data = pe_results_obj_pairs_TP_only.loc[:, [self.feature_signature, 'sw_signature']]
        if 'deg' in self.unit:
            pairs_data.loc[:, self.feature_signature] = np.rad2deg(pairs_data.loc[:, self.feature_signature].values.astype(np.float32))
        self.pairs_data = pairs_data
        feature_per_sw_data = pd.DataFrame()
        for sw_sign, group in self.pairs_data.groupby(by='sw_signature'):
            column = group.drop(columns='sw_signature').rename(columns={self.feature_signature: sw_sign}).reset_index(drop=True)
            feature_per_sw_data = pd.concat([feature_per_sw_data, column], axis=1)
        self.feature_vals = feature_per_sw_data

    def _prepare_feature_cdf_pdf_data(self):
        N = 1000
        min_val, max_val = self.feature_vals.min().min(), self.feature_vals.max().max()
        x = np.linspace(min_val, max_val, N)
        abs_x = np.sort(np.abs(x))
        cdf_data, pdf_data, quantils_data = {}, {}, {}
        for sw_sign in self.feature_vals.columns:
            cdf_data[sw_sign] = calc_cdf(abs_x, self.feature_vals.loc[:, sw_sign].values)
            pdf_data[sw_sign] = calc_pdf(x, self.feature_vals.loc[:, sw_sign].values)
            quantils_data[sw_sign] = calc_quantils(self.feature_vals.loc[:, sw_sign].to_numpy())
        self.cdf_data = pd.DataFrame(cdf_data, index=abs_x)
        self.pdf_data = pd.DataFrame(pdf_data, index=x)
        quant_indexes = [s + self.unit for s in ['Quantile 0.5 ', 'Quantile 0.95 ', 'Quantile 0.99 ']]  # add unit
        self.quantils_table = pd.DataFrame(quantils_data, index=quant_indexes)

    def _prepare_kpis_per_log_data(self):
        kpis_per_log = self.data['kpis_pairs_features_per_log']
        kpis_per_log = kpis_per_log.loc[kpis_per_log.loc[:, 'feature_signature'] == self.feature_signature, :].reset_index(drop=True)
        if 'deg' in self.unit:
            kpis_per_log.loc[:, 'kpi_value'] = np.rad2deg(kpis_per_log.loc[:, 'kpi_value'].values.astype(np.float32))
        self.kpis_per_log = kpis_per_log

    def _prepare_best_worst_tables(self):
        feature_mask = self.data['kpis_pairs_features_per_log'].loc[:, 'feature_signature'] == self.feature_signature
        pairs_data = self.data['kpis_pairs_features_per_log'].loc[feature_mask, :].reset_index(drop=True)
        if 'deg' in self.unit:
            pairs_data.loc[:, 'kpi_value'] = np.rad2deg(pairs_data.loc[:, 'kpi_value'].values.astype(np.float32))
        best_worst_data = {}
        df = pd.DataFrame()
        for (log_index, sw_sign), group in pairs_data.groupby(by=['log_index', 'sw_signature']):
            temp_df = group.drop(columns='feature_signature').pivot_table(values='kpi_value', columns='kpi_signature').reset_index(drop=True)
            temp_df = pd.concat([pd.DataFrame({'log_index': [log_index], 'sw_signature': [sw_sign]}), temp_df], axis=1)
            df = df.append(temp_df)
        for sw_sign, group in df.groupby(by='sw_signature'):
            best_worst_data[sw_sign] = {'Logs with best RMSE': group.sort_values(by='RMSE').head(5).to_html(index=False),
                                        'Logs with worst RMSE': group.sort_values(by='RMSE', ascending=False).head(5).to_html(index=False)}
        self.best_worst_cases_html = best_worst_data


if __name__ == "__main__":
    data1_path = r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\pe_example\pe_example_aggRefData.pickle"
    data2_path = r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\pe_example\pe_example_aggRefData.pickle"
    data1 = load_pickle(data1_path)
    data2 = load_pickle(data2_path)
    input_data = {'soft_1': data1,
                  'soft_2': data2}
    column_name = 'deviation_pos_vcsx'
    section_name = 'Longitudinal position deviation in VCS'
    config = {'plot_style': 'default',
              'default_plot_size': (10, 3.5),
              'figures_save_path': r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\pe_example\report_module\temp_plots"}
    template_path = "SingleDeviationResults.html"
    plotter = Plotter(config)
    dev_results_builder = SingleDeviationResultsBuilder(input_data, column_name, section_name, '[m]', template_path, plotter)
    dev_results_builder.save_pdf(r'\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\pe_example\report_module\pdf_reports\single_dev_calc.pdf')
