import numpy as np
import pandas as pd
from aspe.report_generator.BaseBuilder import BaseBuilder
from aspe.report_generator.Plotter import Plotter
from aspe.report_generator.support_functions import load_pickle
from aspe.report_generator.support_functions import calc_cdf


class ReferenceDataBuilder(BaseBuilder):
    def __init__(self, ref_data, template_name, plotter):
        """
        Class for building reference data report section. Contains information about reference object heading,
        speed and yaw rate.
        :param ref_data: dict: soft_version: DataFrame with reference data - output from PE
        :param template_name: string, .html template file name
        :param plotter: object, report_generator.Plotter class instance
        """
        BaseBuilder.__init__(self, template_name, plotter)
        """" only single ref data is needed for this module"""
        self.ref_data = ref_data
        self.init_template_args()
        self.build_results()
        self.render_html()

    def init_template_args(self):
        self.template_args['rel_count_bar_plot_path'] = ''
        self.template_args['velocity_plots'] = []
        self.template_args['heading_plots'] = []
        self.template_args['yaw_rate_plots'] = []

    def build_results(self):
        #""""Relative / non relative comparision"""
        #rel_nrel_df, rel_data_df = self.get_rel_nrel_df()
        #rel_count_bar_plot_path = self.plotter.bar(rel_nrel_df, 'Relevancy status objects count', ylabel='Count [-]',
        #                                           figsize=(10, 4))

        """"Velocity information"""
        vel_cdf, min_val, max_val = self.calc_cdf(np.abs(self.ref_data.loc[:, 'speed']))
        xlim = [0.9*min_val, 1.1*max_val]
        vel_pdf_plot_path = self.plotter.histogram(self.ref_data.loc[:, 'speed'], 'Reference speed histogram',
                                                   xlabel='Speed [m/s]', ylabel='Count [-]', xlim=xlim, figsize=(10, 3))
        vel_cdf_plot_path = self.plotter.cdf(vel_cdf, [], 'Reference speed empirical CDF', xlabel='Speed [m/s]',
                                             xlim=xlim, figsize=(10, 3))

        """"Heading information"""
        head_cdf, min_val, max_val = self.calc_cdf(np.abs(np.rad2deg(self.ref_data.loc[:, 'bounding_box_orientation'])))
        head_pdf_plot_path = self.plotter.histogram(np.rad2deg(self.ref_data.loc[:, 'bounding_box_orientation']),
                                                    'Reference orientation histogram', xlabel='Orientation [deg]',
                                                    ylabel='Count [-]', figsize=(10, 3))

        head_cdf_plot_path = self.plotter.cdf(head_cdf, [], 'Reference orientation empirical CDF', xlabel='Absolute orientation [deg]',
                                              figsize=(10, 3))

        """"Yaw rate information"""
        head_cdf, min_val, max_val = self.calc_cdf(np.abs(np.rad2deg(self.ref_data.loc[:, 'yaw_rate'].astype(np.float32))))
        yaw_pdf_plot_path = self.plotter.histogram(self.ref_data.loc[:, 'yaw_rate'], 'Reference yaw rate histogram',
                                                   xlabel='Yaw rate [deg/s]', ylabel='Count [-]', figsize=(10, 3))
        yaw_cdf_plot_path = self.plotter.cdf(head_cdf, [], 'Reference yaw rate empirical CDF',
                                             xlabel='Absolute  yaw rate [deg/s]', figsize=(10, 3))

        #self.template_args['rel_count_bar_plot_path'] = rel_count_bar_plot_path
        self.template_args['velocity_plots'].append(vel_pdf_plot_path)
        self.template_args['velocity_plots'].append(vel_cdf_plot_path)
        self.template_args['heading_plots'].append(head_pdf_plot_path)
        self.template_args['heading_plots'].append(head_cdf_plot_path)
        self.template_args['yaw_rate_plots'].append(yaw_pdf_plot_path)
        self.template_args['yaw_rate_plots'].append(yaw_cdf_plot_path)

    def get_rel_nrel_df(self):
        rel_dict = {}
        rel_dict['Status'] = ['REL', 'NREL']
        rel_data_df = self.ref_data[self.ref_data['relevancy_flag']]
        rel_count = len(rel_data_df.index)
        nrel_count = len(self.ref_data[~self.ref_data['relevancy_flag']].index)
        rel_dict['objects count'] = [rel_count, nrel_count]
        rel_nrel_df = pd.DataFrame(rel_dict)
        rel_nrel_df.set_index('Status', inplace=True)
        return rel_nrel_df, rel_data_df

    def calc_cdf(self, data):
        n = 1000
        column_data = data[~np.isnan(data)]  # delete nan from vector
        min_val = column_data.min()
        max_val = column_data.max()
        x = np.linspace(min_val, max_val, n)
        y_cdf = calc_cdf(x, column_data)

        cdf_df = pd.DataFrame({'CDF': y_cdf}, index=x)
        return cdf_df, min_val, max_val


if __name__ == "__main__":
    ref_data1_path = r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP" \
                     r"\full_loop\SW201_SUV_V1\LSS\pe_example\pe_example_aggRefData.pickle"
    ref_data2_path = r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP" \
                     r"\full_loop\SW201_SUV_V1\LSS\pe_example\pe_example_aggRefData.pickle"
    config = {'plot_style': 'default',
              'plot_size': (10, 3.1),
              'figures_save_path': r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP"
                                   r"\full_loop\SW201_SUV_V1\LSS\pe_example\report_module\temp_plots"}
    data1 = load_pickle(ref_data1_path)
    data2 = load_pickle(ref_data2_path)
    ref_input_data = {'soft_1': data1,
                      'soft_2': data2}

    template_path = "ReferenceData.html"
    plotter = Plotter(config)
    ref_data_builder = ReferenceDataBuilder(ref_input_data, template_path, plotter)
    ref_data_builder.save_pdf(r'\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP'
                              r'\full_loop\SW201_SUV_V1\LSS\pe_example\report_module\pdf_reports\ref_data.pdf')
