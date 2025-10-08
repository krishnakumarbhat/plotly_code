import os
import pandas as pd
import aspe.report_generator as rep_gen
from aspe.report_generator.BinaryClassificationBuilder import BinaryClassificationBuilder
from aspe.report_generator.DeviationResultsBuilder import DeviationResultsBuilder
from aspe.report_generator.NeesResultsBuilder import NeesResultsBuilder
from aspe.report_generator.Plotter import Plotter
from aspe.report_generator.TestSetupBuilder import TestSetupBuilder
from aspe.report_generator.BaseBuilder import BaseBuilder
from aspe.report_generator.support_functions import load_pickle
from aspe.extractors.Interfaces.IDataSet import IDataSet
from aspe.utilities.SupportingFunctions import map_array_values


class ReportGenerator(BaseBuilder):

    def __init__(self, input_data, report_gen_config, plotter):
        """
        Main wrapper class for report generation.
        :param input_data: PE output data parsed by ReportGenerator.prepare_input_data function
        :param report_gen_config: dictionary of some overall settings
        :param plotter: object of Plotter class
        :param template_names: sections .html templates file names
        """
        BaseBuilder.__init__(self, 'Report.html', plotter)
        self.data = input_data
        self.config = report_gen_config
        self.test_setup_builder = None
        self.dev_results_builder = None
        self.nees_results_builder = None
        self.bin_results_builder = None
        self.init_template_args()
        self.create_builders()
        self.build_results()
        self.render_html()

    def create_builders(self):
        self.test_setup_builder = TestSetupBuilder(self.data, self.config['tc_description'], self.data['sw_descriptions'],
                                                   'TestSetup.html', 'HostData.html', 'ReferenceData.html', 'LogsData.html', self.plotter)
        self.dev_results_builder = DeviationResultsBuilder(self.data, 'SingleDeviationResults.html', self.plotter)
        self.nees_results_builder = NeesResultsBuilder(self.data, 'SingleNeesResults.html', self.plotter)
        self.bin_results_builder = BinaryClassificationBuilder(self.data, 'BinaryClassification.html', self.plotter)

    def init_template_args(self):
        """
        initialize dictionary which contains Jinja template arguments. These arguments are used to fill .html template
        file
        :return:
        """
        self.template_args['title'] = self.config['title']
        module_path, _ = os.path.split(os.path.abspath(rep_gen.__file__))
        self.template_args['logo'] = module_path + r'\templates\aptiv_logo.png'
        self.template_args['subtitle'] = self.config['subtitle']

    def build_results(self):
        """
        Get html code of all sections and save it to template_args
        :return:
        """
        self.template_args['test_setup_builder'] = self.test_setup_builder.html_out
        self.template_args['deviation_builder'] = self.dev_results_builder.html_out
        self.template_args['nees_builder'] = self.nees_results_builder.html_out
        self.template_args['binary_builder'] = self.bin_results_builder.html_out
        self.template_args['logs_paths'] = self.build_log_paths_df()

    def build_log_paths_df(self):
        log_paths_raw = self.data['logs_paths']
        out_dfs = {}
        for sw_signature, single_sw_df in log_paths_raw.groupby(by='sw_signature'):
            logs_paths = [os.path.split(path)[-1] for path in single_sw_df.loc[:, 'log_path'].values]
            single_sw_df.loc[:, 'log_path'] = logs_paths
            single_sw_df = single_sw_df.drop(columns='sw_signature').rename(columns={'log_path': 'log_name'}).astype({'log_index': int})
            out_dfs[sw_signature] = single_sw_df.to_html(index=False)
        return out_dfs

    @staticmethod
    def prepare_input_data(data_paths, soft_names=None, soft_descriptions=None):
        """
        Load data from multiple .pickle file and aggregate it to one dictionary structure.
        :param soft_names: used aliases for software versions. If emplty list - use default
        :param data_paths: list of paths to .pickle files. Single .pickle file represents single software version data
        :param soft_description: list of strings of software versions description
        :return: dict, aggregated data
        """
        if soft_names is None:
            soft_names = [f'sw_{n}' for n in range(1, len(data_paths)+1)]  # 0 - N-1 -> 1 - N
        if soft_descriptions is None:
            soft_descriptions = ['description not given' for _ in soft_names]
        sw_descriptions = {a: b for a, b in zip(soft_names, soft_descriptions)}

        atts_to_aggregate = ['logs_features_per_log', 'logs_features_aggregated',
                             'kpis_binary_class_per_log', 'kpis_binary_class_aggregated',
                             'kpis_pairs_features_per_log', 'kpis_pairs_features_aggregated',
                             'extracted_reference_host', 'extracted_estimated_objects', 'extracted_reference_objects',
                             'pe_results_obj_pairs', 'pe_results_obj_est', 'pe_results_obj_ref',
                             'kpis_pairs_lifetime_aggregated',
                             'logs_paths']

        dfs2concat = {att: [] for att in atts_to_aggregate}
        output_data = {'sw_descriptions': sw_descriptions}  # init output structure

        for i, path in enumerate(data_paths):
            data = load_pickle(path)
            soft_name = soft_names[i]
            for att_signature in atts_to_aggregate:
                attribute = getattr(data, att_signature)
                if isinstance(attribute, IDataSet):
                    df = attribute.signals
                else:
                    df = attribute
                df['sw_signature'] = soft_name
                dfs2concat[att_signature].append(df)

        for att_signature, df_list in dfs2concat.items():
            output_data[att_signature] = pd.concat(df_list).reset_index(drop=True)

        # concat latency results to pairs KPIs
        latency_kpis = output_data['kpis_pairs_lifetime_aggregated']
        latency_features_signatures = latency_kpis['feature_signature'].to_numpy()
        kpi_names = latency_kpis['kpi_signature'].to_numpy()
        signatures_mapper = {'latency_position_x': 'dev_position_x',
                             'latency_position_y': 'dev_position_y',
                             'latency_velocity_otg_x': 'dev_velocity_x',
                             'latency_velocity_otg_y': 'dev_velocity_y',
                             'latency_velocity_rel_x': 'dev_velocity_rel_x',
                             'latency_velocity_rel_y': 'dev_velocity_rel_y',
                             'latency_acceleration_otg_x': 'dev_acceleration_otg_x',
                             'latency_acceleration_otg_y': 'dev_acceleration_otg_y',
                             'latency_acceleration_rel_x': 'dev_acceleration_rel_x',
                             'latency_acceleration_rel_y': 'dev_acceleration_rel_y',
                             'latency_bounding_box_orientation': 'dev_orientation',
                             'latency_yaw_rate': 'dev_yaw_rate',
                             }
        kpi_names_mapper = {'bias': 'latency - mean',
                            'STD': 'latency - std',}
        latency_kpis.loc[:, 'feature_signature'] = map_array_values(latency_features_signatures, signatures_mapper, '')
        latency_kpis.loc[:, 'kpi_signature'] = map_array_values(kpi_names, kpi_names_mapper, '')

        pairs_kpis = output_data['kpis_pairs_features_aggregated']
        output_data['kpis_pairs_features_aggregated'] = pairs_kpis.append(latency_kpis).reset_index(drop=True)
        return output_data


def example():
    # Create plotter object
    plotter_config = {'plot_style': 'default',
                      'default_plot_size': (10, 3.75),
                      'figures_save_path': r"C:\wkspaces\ASPE0000_00_Common\.private\report_figs_temp",
                      'host_size': (2, 5),
                      'trace_plots_lims': (-75, 75)}
    plotter = Plotter(plotter_config)

    # set main report configuration
    report_gen_config = {'title': 'Active Safety Performance Evaluation report',
                         'subtitle': 'F360 radar tracker',
                         'tc_description': 'this is test scenario description'}

    # prepare data
    sw_names = ['sw_001', 'sw_002']  # if soft_names is empty list, default names are used: sw_1, sw_2, ... sw_N
    sw_descriptions = ['this is sw_001 description', 'this is sw_002 description']
    data_paths = [r"C:\wkspaces\ASPE0000_00_Common\.private\ASPE_reports\ASPE_mutlilog_out_data.pickle",
                  r"C:\wkspaces\ASPE0000_00_Common\.private\ASPE_reports\ASPE_mutlilog_out_data.pickle"]

    #generate report and save it
    report_input_data = ReportGenerator.prepare_input_data(data_paths, sw_names, sw_descriptions)
    rep_gen = ReportGenerator(report_input_data, report_gen_config, plotter)
    rep_gen.save_pdf(r"C:\wkspaces\ASPE0000_00_Common\.private\ASPE_reports\ASPE_report.pdf")


if __name__ == '__main__':
    example()