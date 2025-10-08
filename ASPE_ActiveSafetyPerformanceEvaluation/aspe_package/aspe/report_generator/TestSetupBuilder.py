from aspe.report_generator.HostDataBuilder import HostDataBuilder
from aspe.report_generator.Plotter import Plotter
from aspe.report_generator.ReferenceDataBuilder import ReferenceDataBuilder
from aspe.report_generator.support_functions import load_pickle
from aspe.report_generator.BaseBuilder import BaseBuilder
from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification.IBinaryClassifier import BCType
import pandas as pd


class TestSetupBuilder(BaseBuilder):
    def __init__(self, input_data, tc_description, sw_descriptions, test_setup_tmpl_name, host_tmpl_name, ref_tmpl_name, log_tmpl_name,
                 plotter):
        BaseBuilder.__init__(self,  test_setup_tmpl_name, plotter)
        self.plotter = plotter
        self.host_data_builder = HostDataBuilder(input_data['extracted_reference_host'], host_tmpl_name, plotter)
        self.ref_data_builder = ReferenceDataBuilder(input_data['extracted_reference_objects'], ref_tmpl_name, plotter)
        #self.logs_data_builder = LogsDataBuilder(input_data['log_data'], log_tmpl_name, plotter)
        self.input_data = input_data

        self.init_template_args()
        self.template_args['test_case_description'] = tc_description
        self.template_args['sw_description'] = sw_descriptions
        self.build_results()
        self.render_html()

    def init_template_args(self):
        self.template_args['test_case_description'] = str
        self.template_args['host_data'] = str
        self.template_args['ref_data'] = str
        self.template_args['logs_data'] = str
        self.template_args['sw_description'] = {}
        self.template_args['trace_plot'] = []

    def build_results(self):
        self.template_args['trace_plot'] = self._build_traces()
        self.template_args['host_data'] = self.host_data_builder.html_out
        self.template_args['ref_data'] = self.ref_data_builder.html_out
        self.template_args['logs_data'] = ''

    def _build_traces(self):
        pe_ref_data = self.input_data['pe_results_obj_ref']
        pe_est_data = self.input_data['pe_results_obj_est']
        ref_position_data = self.input_data['extracted_reference_objects'].loc[:, ['position_x', 'position_y']]
        est_position_data = self.input_data['extracted_estimated_objects'].loc[:, ['position_x', 'position_y']]
        ref_data = pd.concat([pe_ref_data, ref_position_data], axis=1)
        est_data = pd.concat([pe_est_data, est_position_data], axis=1)
        trace_plots_paths = []
        for sw_signature, ref_sw_data in ref_data.groupby(by='sw_signature'):
            est_sw_data = est_data.loc[est_data.loc[:, 'sw_signature'] == sw_signature, :]
            traces = self._calc_traces_single_sw(ref_sw_data, est_sw_data)
            trace_plot_path = self.plotter.trace(traces, sw_signature,
                                                 figsize=(11, 11),
                                                 xlabel='position VCS Y [m]',
                                                 ylabel='position VCS X [m]')
            trace_plots_paths.append(trace_plot_path)
        return trace_plots_paths

    def _calc_traces_single_sw(self, ref_sw_data, est_sw_data):
        blue_trace = ref_sw_data[ref_sw_data['binary_classification'] == BCType.TruePositive][['position_x', 'position_y']]
        black_trace = ref_sw_data[ref_sw_data['binary_classification'] == BCType.FalseNegative][['position_x', 'position_y']]
        green_trace = est_sw_data[est_sw_data['binary_classification'] == BCType.TruePositive][['position_x', 'position_y']]
        red_trace = est_sw_data[est_sw_data['binary_classification'] == BCType.FalsePositive][['position_x', 'position_y']]
        traces = [blue_trace, black_trace, green_trace, red_trace]
        return traces


if __name__ == "__main__":
    host_data_path = r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\pe_example\pe_example_agg_host_data"
    host_data = {'soft_1': load_pickle(host_data_path)}

    ref_data1_path = r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\pe_example\pe_example_aggRefData.pickle"
    ref_data2_path = r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\pe_example\pe_example_aggRefData.pickle"
    data1 = load_pickle(ref_data1_path)
    data2 = load_pickle(ref_data2_path)
    ref_input_data = {'soft_1': data1,
                      'soft_2': data2}

    est_data1_path = r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\pe_example\pe_example_aggEstData.pickle"
    est_data2_path = r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\pe_example\pe_example_aggEstData.pickle"

    data1 = load_pickle(est_data1_path)
    data2 = load_pickle(est_data2_path)
    est_input_data = {'soft_1': data1,
                      'soft_2': data2}

    logs_data_path = r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\pe_example\pe_example_agg_log_data"
    logs_data = {'soft_1': load_pickle(logs_data_path)}

    config = {'plot_style': 'default',
              'default_plot_size': (10, 4),
              'host_size': (2, 5),
              'figures_save_path': r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\pe_example\report_module\temp_plots"}
    plotter = Plotter(config)
    test_setup_builder = TestSetupBuilder(host_data, ref_input_data, logs_data, est_input_data, plotter)
    test_setup_builder.save_pdf(
        r'\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\pe_example\report_module\pdf_reports\test_setup.pdf')
