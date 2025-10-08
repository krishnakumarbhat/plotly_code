import os
import sys
print(sys.path)

from aspe.report_generator.ReportGenerator import ReportGenerator
from aspe.report_generator.BaseBuilder import BaseBuilder
from aspe.report_generator.SingleDeviationResultsBuilder import SingleDeviationResultsBuilder
from aspe.report_generator.Plotter import Plotter

if __name__ == '__main__':
    figures_save_path = r"C:\Repos\ASPE0000_00_Common\aptivperformance_evaluation\aspe.evaluation\ReportGenerator\ASPE_reports"
    pdf_save_path = r'C:\Repos\ASPE0000_00_Common\aptivperformance_evaluation\aspe.evaluation\ReportGenerator\ASPE_reports'
    ''' Data loading section: - append all files with agregated_data'''
    data_paths = []
    data_paths.append(r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\LSS_CURVES\rRf360t3050303v202r1\rRf360t3050303v202r1_aggregated_data.pickle")
    data_paths.append(r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\LSS_CURVES\rRf360t3060304v202r1\rRf360t3060304v202r1_aggregated_data.pickle")

    '''
    title - string, main report title
    subtitle - string, report subtitle 
    soft_names - list of strings, aliases for compared software versions, if list is empty default names are resim extension which is 
    saved in .pickle files
    soft_descriptions - list of strings describing each of compared software versions
    test_case_description - string with logs scenario description, where log data was gathered or some addition information
    deviation_columns - list of strings, names of columns of reference objects data DataFrame which are taken to 
    deviations visualization
    deviation_section_names - list of strings, headers of deviations sections - same length as deviation_columns
    deviation_units - list of strings, units of deviations - same length as deviation_columns
    '''
    title = 'Active Safety Performance Evaluation report'
    subtitle = 'F360 radar tracker'
    soft_names = []  # if soft_names is empty list, software version name is resim extension
    soft_descriptions = ['logs resimulated without alignment angles',
                         'logs resimulated with alignment angles']
    test_case_description = 'test report'

    """
    Paths:
    templates_dir - string, directory of Jinja .html templates files
    css_path - string, path of css file
    figures_save_path - string, temp path for saving matplotlib figures which are used in report 
    pdf_save_path - string, folder path where report should be saved
    pdf_name - string, name of saved .pdf report
    """
    templates_dir = r'templates'
    css_path = r'CSS\default.css'

    pdf_name = 'ASPE_report.pdf'

    """
    Plotter settings:
    default_plot_size - tuple: (width, height) given in inches, default size of the plot 
    host_size - tuple: (width, height) given in inches, size of host in trace plots 
    trace_plots_lims - tuple (mix ,max) given in meters, trace plot xy axes limits
    mae_pdf_xlim_factor - int, for deviation calculation PDF plot xlim is computed: 
                        x_max = accuracy + mae * mae_pdf_xlim_factor , where mae - median of absolute deviation
                        and xlim = [-x_max, x_max]
    """
    default_plot_size = (10, 3.75)
    host_size = (2, 5)
    trace_plots_lims = (-75, 75)
    mae_pdf_xlim_factor = 5
    """
    Set chosen settings
    """
    BaseBuilder.templates_dir = templates_dir
    BaseBuilder.css_path = css_path
    report_gen_config = {'title': title,
                         'subtitle': subtitle,
                         'tc_description': test_case_description}
    plotter_config = {'plot_style': 'default',
                      'default_plot_size': default_plot_size,
                      'figures_save_path': figures_save_path,
                      'host_size': host_size,
                      'trace_plots_lims': trace_plots_lims}
    SingleDeviationResultsBuilder.MAE_PDF_XLIM_FACTOR = mae_pdf_xlim_factor
    """
    Parse input data and generate report
    """
    input_data = ReportGenerator.prepare_input_data(soft_names, data_paths, soft_descriptions)
    plotter = Plotter(plotter_config)
    report_generator = ReportGenerator(input_data, report_gen_config, plotter)
    report_generator.save_pdf(os.path.join(pdf_save_path, pdf_name))