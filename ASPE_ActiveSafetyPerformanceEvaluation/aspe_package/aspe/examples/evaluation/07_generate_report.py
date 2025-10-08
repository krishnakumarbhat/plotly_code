from aspe.examples.evaluation.private.user_data import ExampleData
from aspe.report_generator.Plotter import Plotter
from aspe.report_generator.ReportGenerator import ReportGenerator

"""
Reporting example.
Update report_temp_figs_path within ExampleData (or juts update Plotter_config dict directly) with some folder for 
storing temporary figures created for report. This folder must exists on your disk.
To generate example data which is needed run first 04_F360_multi_log_evaluation.py.
"""

sw_1_load_path = ExampleData.multi_log_evaluation_results_2_12_path
sw_2_load_path = ExampleData.multi_log_evaluation_results_2_15_path
report_save_path = ExampleData.report_save_path

# Create plotter object
plotter_config = {'plot_style': 'default',
                  'default_plot_size': (10, 3.75),
                  'figures_save_path': ExampleData.report_temp_figs_path,
                  'host_size': (2, 5),
                  'trace_plots_lims': (-75, 75)}
plotter = Plotter(plotter_config)

# set main report configuration
report_gen_config = {'title': 'Active Safety Performance Evaluation report',
                     'subtitle': 'F360 radar tracker',
                     'tc_description': 'this is test scenario description'}

# prepare data
sw_names = ['sw_2_12', 'sw_2_15']  # if soft_names is empty list, default names are used: sw_1, sw_2, ... sw_N
sw_descriptions = ['this is sw_2_12 description', 'this is sw_2_15 description']
data_paths = [sw_1_load_path,
              sw_2_load_path]

# generate report and save it
report_input_data = ReportGenerator.prepare_input_data(data_paths, sw_names, sw_descriptions)
rep_gen = ReportGenerator(report_input_data, report_gen_config, plotter)
rep_gen.save_pdf(report_save_path)
