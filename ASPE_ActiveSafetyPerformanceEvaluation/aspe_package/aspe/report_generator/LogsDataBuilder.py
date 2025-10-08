import pandas as pd
from aspe.report_generator.BaseBuilder import BaseBuilder
from aspe.report_generator.Plotter import Plotter
from aspe.report_generator.support_functions import load_pickle


class LogsDataBuilder(BaseBuilder):
    def __init__(self, logs_data, template_name, plotter):
        """
        Class for building logs data report section. Contains information about logs length given in seconds.
        Log length is interpreted as time difference of first and last relevant reference sample.
        :param logs_data: dict: soft_version: DataFrame with host data - output from PE
        :param template_name: string, .html template file name
        :param plotter: object, report_generator.Plotter class instance
        """
        BaseBuilder.__init__(self, template_name, plotter)
        self.logs_data = logs_data

        self.init_template_args()
        self.build_results()
        self.render_html()

    def init_template_args(self):
        self.template_args['log_count'] = int
        self.template_args['est_rel_samples_plot'] = str
        self.template_args['est_rel_samples_plot'] = str
        self.template_args['logs_len_plot'] = str
        self.template_args['logs_len_summary'] = str

    def build_results(self):
        logs_len_df = pd.DataFrame()
        for key, val in self.logs_data.items():
            val.set_index('log_index', inplace=True)
            logs_len_df.loc[:, 'log_length'] = val['log_length']
        self.template_args['log_count'] = val.shape[0]
        self.template_args['logs_len_plot'] = self.plotter.histogram(logs_len_df, 'Logs length histogram', xlabel='log length [s]',
                                                                     ylabel='count [-]', bins=20, figsize=(10, 8))
        self.template_args['logs_len_summary'] = "{0:.2f}".format(float(logs_len_df.sum()))

if __name__ == "__main__":
    logs_data_1_path = r"C:\logs\ASPE\LSS_CURVES\rRf360t3050303v202r1\rRf360t3050303v202r1_agg_log_data.pickle"
    logs_data_2_path = r"C:\logs\ASPE\LSS_CURVES\rRf360t3060304v202r1\rRf360t3060304v202r1_agg_log_data.pickle"
    config = {'plot_style': 'default',
              'plot_size': (10, 4),
              'figures_save_path': r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\pe_example\report_module\temp_plots"}
    logs_data_1 = load_pickle(logs_data_1_path)
    logs_data_2 = load_pickle(logs_data_2_path)
    logs_data = {'soft_1': logs_data_1, 'soft_2': logs_data_2}
    template_path = "LogsData.html"
    plotter = Plotter(config)
    logs_data_builder = LogsDataBuilder(logs_data, template_path, plotter)
    logs_data_builder.save_pdf(r'\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\pe_example\report_module\pdf_reports\logs_data.pdf')
