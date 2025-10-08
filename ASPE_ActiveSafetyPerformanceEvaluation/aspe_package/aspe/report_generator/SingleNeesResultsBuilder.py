import numpy as np
import pandas as pd
from scipy.stats import chi2

from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification import BCType
from aspe.report_generator.BaseBuilder import BaseBuilder
from aspe.report_generator.Plotter import Plotter
from aspe.report_generator.support_functions import calc_cdf, calc_pdf, calc_quantils


class SingleNeesResultsBuilder(BaseBuilder):

    def __init__(self, data: dict, plotter: Plotter, template_path: str, section_name: str, feature_signature: str,
                 dimensions: int):
        super().__init__(template_path, plotter)
        self.data = data
        self.section_name = section_name
        self.feature_signature = feature_signature
        self.dimensions = dimensions

        self.init_template_args()
        self._prepare_data_frames()
        self.build_results()
        self.render_html()

    def init_template_args(self):
        self.template_args['section_name'] = self.section_name
        self.template_args['agg_kpi_plot'] = ''
        self.template_args['best_worst_cases_tables'] = {}
        self.env.globals.update(zip=zip)
        pd.set_option('precision', 3)

    def _prepare_data_frames(self):
        self._prepare_aggregated_kpis()
        self._prepare_agg_feature_per_soft_data()
        self._prepare_feature_cdf_pdf_data()
        self._prepare_kpis_per_log_data()
        self._prepare_best_worst_tables()

    def build_results(self):
        mean_bar_plot_path = self.plotter.bar(self.mean_df,
                                              title=self.section_name + ' mean',
                                              markers={'expected': self.dimensions},
                                              ylabel='mean [$\sigma^2$]',
                                              figsize=(5, 7))

        p_value_bar_plot_path = self.plotter.bar(self.p_value_df,
                                                 title=self.section_name + '\nNEES mean Gamma test p-value',
                                                 markers={'5% critical value': 0.05},
                                                 ylabel='p-value [-]',
                                                 yscale='log',
                                                 figsize=(5, 7))

        p_value_pdf_plot_path = self.plotter.pdf(self.pdf_data,
                                                 reference=self.reference_distribution_pdf_series,
                                                 title=self.section_name + ' PDF',
                                                 xlabel='Normalized Estimated Error Squared [$\sigma^2$]',
                                                 ylabel='Density [-]',
                                                 figsize=(10, 5))

        p_value_cdf_plot_path = self.plotter.cdf(self.cdf_data,
                                                 self.quantils_table,
                                                 reference=self.reference_distribution_cdf_series,
                                                 title=self.section_name + ' CDF',
                                                 xlabel='Normalized Estimated Error Squared [$\sigma^2$]',
                                                 ylabel='Quantile [-]',
                                                 figsize=(10, 5))

        quantils_table_html = self.quantils_table.to_html(index_names=False, escape=False)

        mean_per_log_boxplot_path = self.plotter.box_xy_hue(self.mean_per_log,
                                                            x_col='kpi_signature',
                                                            y_col='kpi_value',
                                                            hue_col='sw_signature',
                                                            title=self.section_name + ' mean per log',
                                                            markers={'expected': self.dimensions},
                                                            xlabel='',
                                                            ylabel='mean [$\sigma^2$]',
                                                            xlim=[-1, 1],
                                                            figsize=(5, 8))

        p_value_per_log_boxplot_path = self.plotter.box_xy_hue(self.p_value_per_log,
                                                               x_col='kpi_signature',
                                                               y_col='kpi_value',
                                                               hue_col='sw_signature',
                                                               title=self.section_name + '\nNEES mean Gamma test p-value per log',
                                                               markers={'5% critical value': 0.05},
                                                               xlabel='',
                                                               ylabel='p-value [-]',
                                                               yscale='log',
                                                               xlim=[-1, 1],
                                                               figsize=(5, 8))

        self.template_args.update({
            'agg_mean_bar_plot_path': mean_bar_plot_path,
            'agg_p_value_bar_plot_path': p_value_bar_plot_path,
            'agg_kpis_table': self.kpis_results_agg_tab_html,
            'p_value_pdf_plot_path': p_value_pdf_plot_path,
            'p_value_cdf_plot_path': p_value_cdf_plot_path,
            'quantile_table': quantils_table_html,
            'mean_per_log_boxplot_path': mean_per_log_boxplot_path,
            'p_value_per_log_boxplot_path': p_value_per_log_boxplot_path,
            'best_worst_cases_tables': self.best_worst_cases_html,
        })

    def _prepare_aggregated_kpis(self):
        kpis_agg = self.data['kpis_pairs_features_aggregated']
        kpis_agg = kpis_agg.loc[kpis_agg.loc[:, 'feature_signature'] == self.feature_signature].drop(
            columns='feature_signature')
        out_agg_kpis = pd.DataFrame()
        for kpi_signature, group in kpis_agg.groupby(by='kpi_signature'):
            partial = group.pivot_table(values='kpi_value', columns='sw_signature', aggfunc='first')
            partial.index = [kpi_signature]
            out_agg_kpis = out_agg_kpis.append(partial)

        mean_df = out_agg_kpis.loc[['mean']]
        p_value_df = out_agg_kpis.loc[['NEES mean Gamma test p-value']]

        out_agg_kpis4table = pd.concat((mean_df, p_value_df))
        out_agg_kpis4table.index = [
            'mean [&sigma;<sup>2</sup>]',
            'NEES mean Gamma test p-value [-]',
        ]

        self.mean_df = mean_df
        self.p_value_df = p_value_df
        self.kpis_results_agg_tab_html = out_agg_kpis4table.to_html(index_names=False, escape=False)

    def _prepare_agg_feature_per_soft_data(self):
        pe_results_obj_pairs = self.data['pe_results_obj_pairs']
        TP_mask = pe_results_obj_pairs['binary_classification'] == BCType.TruePositive
        pe_results_obj_pairs_TP_only = pe_results_obj_pairs.loc[TP_mask]
        pairs_data = pe_results_obj_pairs_TP_only.loc[:, [self.feature_signature, 'sw_signature']]
        self.pairs_data = pairs_data
        feature_per_sw_data = pd.DataFrame()
        for sw_sign, group in self.pairs_data.groupby(by='sw_signature'):
            column = group.drop(columns='sw_signature').rename(columns={self.feature_signature: sw_sign}).reset_index(
                drop=True)
            feature_per_sw_data = pd.concat([feature_per_sw_data, column], axis=1)
        self.feature_vals = feature_per_sw_data

    def _prepare_feature_cdf_pdf_data(self):
        N = 1000
        min_val = 0
        max_val = 15 if self.dimensions == 1 else 20

        x = np.linspace(min_val, max_val, N)
        abs_x = np.sort(np.abs(x))
        cdf_data, pdf_data, quantils_data = {}, {}, {}
        for sw_sign in self.feature_vals.columns:
            cdf_data[sw_sign] = calc_cdf(abs_x, self.feature_vals.loc[:, sw_sign].values)
            pdf_data[sw_sign] = calc_pdf(x, self.feature_vals.loc[:, sw_sign].values)
            quantils_data[sw_sign] = calc_quantils(cdf_data[sw_sign])

        reference_distribution_pdf = chi2.pdf(x, self.dimensions)
        reference_distribution_cdf = chi2.cdf(x, self.dimensions)
        quantils_data['reference distribution'] = calc_quantils(reference_distribution_cdf)

        self.cdf_data = pd.DataFrame(cdf_data, index=abs_x)
        self.pdf_data = pd.DataFrame(pdf_data, index=x)
        quant_indexes = [f'Quantile {q} [&sigma;<sup>2</sup>]' for q in [0.5, 0.95, 0.99]]
        self.quantils_table = pd.DataFrame(quantils_data, index=quant_indexes)

        self.reference_distribution_cdf_series = pd.Series(data=reference_distribution_cdf, index=abs_x)
        self.reference_distribution_pdf_series = pd.Series(data=reference_distribution_pdf, index=x)

    def _prepare_kpis_per_log_data(self):
        kpis_per_log = self.data['kpis_pairs_features_per_log']
        kpis_per_log = kpis_per_log.loc[kpis_per_log.loc[:, 'feature_signature'] == self.feature_signature,
                       :].reset_index(drop=True)

        self.mean_per_log = kpis_per_log[kpis_per_log['kpi_signature'] == 'mean']
        self.p_value_per_log = kpis_per_log[kpis_per_log['kpi_signature'] == 'NEES mean Gamma test p-value']

    def _prepare_best_worst_tables(self):
        feature_mask = self.data['kpis_pairs_features_per_log'].loc[:, 'feature_signature'] == self.feature_signature
        pairs_data = self.data['kpis_pairs_features_per_log'].loc[feature_mask, :].reset_index(drop=True)
        best_worst_data = {}
        df = pd.DataFrame()
        for (log_index, sw_sign), group in pairs_data.groupby(by=['log_index', 'sw_signature']):
            temp_df = group.drop(columns='feature_signature').pivot_table(values='kpi_value',
                                                                          columns='kpi_signature').reset_index(
                drop=True)
            temp_df = pd.concat([pd.DataFrame({'log_index': [log_index], 'sw_signature': [sw_sign]}), temp_df], axis=1)
            df = df.append(temp_df)
        for sw_sign, group in df.groupby(by='sw_signature'):
            by = 'NEES mean Gamma test p-value'
            best_worst_data[sw_sign] = {
                'Logs with best p-value': group.sort_values(by=by, ascending=False).head(5).to_html(index=False),
                'Logs with worst p-value': group.sort_values(by=by).head(5).to_html(index=False)}
        self.best_worst_cases_html = best_worst_data
