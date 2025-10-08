import os
import pandas as pd
from aspe.report_generator.Plotter import Plotter
from aspe.report_generator.BaseBuilder import BaseBuilder
from aspe.report_generator.support_functions import load_pickle
from aspe.report_generator.kpi_calculation import calculate_binary_classification_kpis
from aspe.report_generator.kpi_calculation import calculate_deviation_calculation_kpis
import numpy as np
from typing import List


class ReportGeneratorMostRelevantInfo(BaseBuilder):
    """
    Class for simple form report generation. User provides data from N test cases as N separate .pickle files with
    PE output data. Output report consists of N pages - single page per test case.
    """
    def __init__(self, input_data: dict, plotter: object):
        """
        :param input_data: PE output data parsed by ReportGenerator.prepare_input_data function
        :param plotter: object of Plotter class
        """
        BaseBuilder.__init__(self, 'SimpleReport.html', plotter)
        self.input_data = input_data
        self.css_path = r"CSS\simple_report.css"
        self.init_template_args()
        self.kpis_data = {}
        self.build_results()
        self.render_html()

    def init_template_args(self):
        """
        initialize dictionary which contains Jinja template arguments. These arguments are used to fill .html template
        file
        :return:
        """
        self.template_args['test_cases'] = []

    def build_results(self):
        """
        Get html code for all test cases pages.
        :return:
        """
        for scenario_name, test_case_data in self.input_data.items():
            self.build_single_scenario_results(scenario_name, test_case_data)

    def build_single_scenario_results(self, test_case_name: str, test_case_data: dict):
        """
        Get plots paths for single test_case results.
        :param test_case_name: test_case name
        :param test_case_data: dict with PE output data for single test_case
        :return:
        """
        test_case_args = {}
        self.kpis_data[test_case_name] = {'KPI': [], 'value': []}
        test_case_args['scenario_name'] = test_case_name
        test_case_args['test_description'] = test_case_data['test_case_description'].replace(r'\n', '<br>')
        test_case_args['bin_class_results'] = \
            self.get_bin_class_results_plot_path(test_case_name, test_case_data,
                                                 f'Binary classification')
        test_case_args['dev_pos_results'] = \
            self.get_xy_deviations_kpis_plot_path(test_case_name, test_case_data, 'deviation_pos_vcsx',
                                                  'deviation_pos_vcsy', f'RMSE of position')
        test_case_args['dev_vel_results'] = \
            self.get_xy_deviations_kpis_plot_path(test_case_name, test_case_data, 'deviation_vel_vcsx',
                                                  'deviation_vel_vcsy', f'RMSE of velocity')
        test_case_args['trace_plot'] = self.get_simple_trace_plots_path(test_case_name, test_case_data)
        scenario_kpis = self.kpis_data[test_case_name]
        kpis_table = pd.DataFrame(scenario_kpis).round(2).to_html(index=False)
        test_case_args['kpis_table'] = kpis_table
        self.template_args['test_cases'].append(test_case_args)

    def get_bin_class_results_plot_path(self, test_case_name: str, test_case_data: dict, title: str):
        """
        Get binary classification results plot path.
        :param test_case_name: test case name
        :param test_case_data: single test case data
        :param title: plot title
        :return:
        """
        needed_kpis = ['TPR']
        kpi_data = {'KPI': needed_kpis}
        ref_data = test_case_data['ref_data']['binary_classification']
        est_data = test_case_data['est_data']['binary_classification']
        bin_calc_kpis = calculate_binary_classification_kpis(ref_data, est_data)
        kpis_list = [bin_calc_kpis[key] for key in needed_kpis]  # filter only needed kpis
        kpi_data[test_case_name] = kpis_list
        kpis_df = pd.DataFrame(kpi_data).set_index('KPI')
        self.kpis_data[test_case_name]['KPI'] += needed_kpis
        self.kpis_data[test_case_name]['value'] += kpis_list
        bin_class_plot_path = self.plotter.bar(kpis_df, f'{title} - {test_case_name}', xlabel='', remove_legend=True,
                                               ylim=[0, 1])
        return bin_class_plot_path

    def get_xy_deviations_kpis_plot_path(self, test_case_name: str, test_case_data: dict,
                                         dev_column_x: str, dev_column_y: str, title: str):
        """
        Get single plot for deviation results. Output plot contains information about x coordinate and y coordinate
        RMS deviation. For example RMSE of position x and position y.
        :param test_case_name: test case name
        :param test_case_data: single test case data
        :param dev_column_x: column name which is taken as first series to plot
        :param dev_column_y: column name which is taken as second series to plot
        :param title: plot title
        :return:
        """
        dev_data_x = test_case_data['ref_data'][dev_column_x]
        dev_data_y = test_case_data['ref_data'][dev_column_y]
        rmse_x = calculate_deviation_calculation_kpis(dev_data_x)['RMSE']
        rmse_y = calculate_deviation_calculation_kpis(dev_data_y)['RMSE']
        max_val = max(rmse_x, rmse_y)
        ylim = [0, (np.floor(max_val / 0.5) + 1) * 0.5]
        kpi_data = {'KPI': ['x', 'y'],
                    test_case_name: [rmse_x, rmse_y]}
        self.kpis_data[test_case_name]['KPI'] += [f'{title}_x', f'{title}_y']
        self.kpis_data[test_case_name]['value'] += [rmse_x, rmse_y]
        kpis_df = pd.DataFrame(kpi_data).set_index('KPI')
        xy_dev_plot_path = self.plotter.bar(kpis_df, f'{title} - {test_case_name}', xlabel='', ylim=ylim,
                                            remove_legend=True)
        return xy_dev_plot_path

    def get_simple_trace_plots_path(self, test_case_name: str, test_case_data: dict):
        """
        Get path with reference data plot trace.
        :param test_case_name: test case name
        :param test_case_data: single test case data
        :return:
        """
        ref_data = test_case_data['ref_data']
        title = f'Reference object trace paths - {test_case_name}'
        trace_plot_path = self.plotter.simple_trace(ref_data, title, figsize=(4, 4))
        return trace_plot_path

    @staticmethod
    def prepare_input_data(soft_names: List[str], data_paths: List[str], soft_description: List[str]):
        """
        Load data from multiple .pickle file and aggregate it to one dictionary structure.
        :param soft_names: used aliases for software versions. If empty list - use resim extension
        :param data_paths: list of paths to .pickle files. Single .pickle file represents single software version data
        :param soft_description: list of strings of software versions description
        :return: dict, aggregated data
        """
        scenarios_data = {}
        for i, (path, soft_name) in enumerate(zip(data_paths, soft_names)):
            data = load_pickle(path)
            scenario_dict = {}
            scenario_dict['ref_data'] = data['reference_obj_data']
            scenario_dict['est_data'] = data['estimated_obj_data']
            scenario_dict['log_data'] = data['log_data']
            scenario_dict['host_data'] = data['host_data']
            scenario_dict['test_case_description'] = soft_description[i]
            scenarios_data[soft_name] = scenario_dict
        return scenarios_data


def example():
    data_paths = []
    # 'P' is equivalent of \\10.224.186.68\AD-Shared\ASPE
    data_paths.append(r"P:\examples_data\most_relevant_kpi_report\pe_output_LSS_rRf360t4010304v202r1.pickle")
    data_paths.append(r"P:\examples_data\most_relevant_kpi_report\pe_output_RCTA_rRf360t4010304v202r1.pickle")
    data_paths.append(r"P:\examples_data\most_relevant_kpi_report\pe_output_OSE_rRf360t4010304v202r1.pickle")

    test_case_names = ['LSS', 'RCTA', 'OSE']
    test_case_descriptions = [r'Line Support System testing scenarios.'
                              r'\nTarget overtakes moving host with relative speed in range of 40 - 70 km/h.'
                              r'\nBoth - left and right side were tested.'
                              r'\nEvaluated logs - 25',

                              r'Rear Cross Traffic Alert testing scenarios.\nTarget object drives through rear side of '
                              r'stationary host with speed in range of 10 - 90 km/h at different pointing angle.'
                              r'\nEvaluated logs - 24',

                              r'Occupant Safe Exit testing scenarios.'
                              r'\nTarget overtakes stationary host with speed in range of 10 - 50 km/h.'
                              r'\nBoth - left and right side were tested.'
                              r'\nEvaluated logs - 24'
                              ]

    """
    Config section
    """
    templates_dir = r'templates'
    css_path = r'CSS\default.css'
    figures_save_path = r"C:\wkspaces\ASPE0000_00_Common\.private"  # replace it with some your local path
    pdf_save_path = r'P:\reports'
    pdf_name = 'Radar_Tracker_Most_Relevant_KPIs_Report_example.pdf'

    """
    Plotter settings:
    default_plot_size - tuple: (width, height) given in inches, default size of the plot 
    host_size - tuple: (width, height) given in inches, size of host in trace plots 
    trace_plots_lims - tuple (mix ,max) given in meters, trace plot xy axes limits
    """
    default_plot_size = (4, 6)
    host_size = (2, 5)
    trace_plots_lims = (-75, 75)
    """
    Set chosen settings
    """
    BaseBuilder.templates_dir = templates_dir
    BaseBuilder.css_path = css_path

    plotter_config = {'plot_style': 'default',
                      'default_plot_size': default_plot_size,
                      'figures_save_path': figures_save_path,
                      'host_size': host_size,
                      'trace_plots_lims': trace_plots_lims}
    """
    Parse input data and generate report
    """
    input_data = ReportGeneratorMostRelevantInfo.prepare_input_data(test_case_names, data_paths, test_case_descriptions)
    plotter = Plotter(plotter_config)
    report_generator = ReportGeneratorMostRelevantInfo(input_data, plotter)
    report_generator.save_pdf(os.path.join(pdf_save_path, pdf_name))


if __name__ == '__main__':
    example()
