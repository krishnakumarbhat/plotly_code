import pickle

from aspe.evaluation.RadarObjectsEvaluation.Flags import DistanceToCSOriginBetweenThrFlag
from aspe.evaluation.RadarObjectsEvaluation.PEMultiLogEvaluation import PEMultiLogEvaluation
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.examples.evaluation.private.user_data import ExampleData
from aspe.providers import F360MudpRTRangeDataProvider
from aspe.providers.support_functions import get_log_list_from_path
from aspe.report_generator.Plotter import Plotter
from aspe.report_generator.ReportGenerator import ReportGenerator

mudp_stream_def_path = ExampleData.mudp_stream_def_path

data_path = ExampleData.rt_range_data_path
logger_path = ExampleData.multi_log_pipeline_logging_folder
save_path_02_12 = ExampleData.analysis_data_path + r"\multi_log_evaluation_output_02_12.pickle"
save_path_12_40 = ExampleData.analysis_data_path + r"\multi_log_evaluation_output_12_40.pickle"
save_path_40_100 = ExampleData.analysis_data_path + r"\multi_log_evaluation_output_40_100.pickle"

f360_data_provider = F360MudpRTRangeDataProvider(mudp_stream_def_path=mudp_stream_def_path)

log_list_2_12 = get_log_list_from_path(data_path, required_sub_strings=['LSS', '2_12.'], level=4)

pipeline_02_12 = PEMultiLogEvaluation(single_log_pipeline=PEPipeline(
    relevancy_reference_objects=DistanceToCSOriginBetweenThrFlag(thr_low=0.2, thr_high=12.0)),
                                      data_provider=f360_data_provider,
                                      logging_file_folder=logger_path)

pipeline_12_40 = PEMultiLogEvaluation(single_log_pipeline=PEPipeline(
    relevancy_reference_objects=DistanceToCSOriginBetweenThrFlag(thr_low=12.0, thr_high=40.0)),
                                      data_provider=f360_data_provider,
                                      logging_file_folder=logger_path)

pipeline_40_100 = PEMultiLogEvaluation(single_log_pipeline=PEPipeline(
    relevancy_reference_objects=DistanceToCSOriginBetweenThrFlag(thr_low=40.0, thr_high=100.0)),
                                       data_provider=f360_data_provider,
                                       logging_file_folder=logger_path)

pe_output_multi_log_02_12 = pipeline_02_12.process_data(log_list_2_12)
with open(save_path_02_12, 'wb') as handle:
    pickle.dump(pe_output_multi_log_02_12, handle, protocol=pickle.HIGHEST_PROTOCOL)
pe_output_multi_log_12_40 = pipeline_12_40.process_data(log_list_2_12)
with open(save_path_12_40, 'wb') as handle:
    pickle.dump(pe_output_multi_log_12_40, handle, protocol=pickle.HIGHEST_PROTOCOL)
pe_output_multi_log_40_100 = pipeline_40_100.process_data(log_list_2_12)
with open(save_path_40_100, 'wb') as handle:
    pickle.dump(pe_output_multi_log_40_100, handle, protocol=pickle.HIGHEST_PROTOCOL)

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

report_save_path = ExampleData.report_save_path

# prepare data
sw_names = ['dist_02_12', 'dist_12_40',
            'dist_40_100']  # if soft_names is empty list, default names are used: sw_1, sw_2, ... sw_N
sw_descriptions = ['0.2-12', '12-40', '40-100']
data_paths = [save_path_02_12, save_path_12_40, save_path_40_100]

# generate report and save it
report_input_data = ReportGenerator.prepare_input_data(data_paths, sw_names, sw_descriptions)
rep_gen = ReportGenerator(report_input_data, report_gen_config, plotter)
rep_gen.save_pdf(report_save_path)
