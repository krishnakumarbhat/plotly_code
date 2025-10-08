from aspe.report_generator.ReportBuilder import ReportBuilder
from aspe.report_generator.example_data_gen import get_simple_exp_fig, get_simple_df_data, get_simple_long_text


if '__main__' == __name__:
    fig1 = get_simple_exp_fig()
    fig2 = get_simple_exp_fig()

    df1 = get_simple_df_data()
    df2 = get_simple_df_data()

    eg_text1 = get_simple_long_text()

    report_gen = ReportBuilder()

    report_gen.add_report_titles(titles=['Aptiv Active Safety KPI report',
                                 'F360 Core Tracker'])
    report_gen.add_software_names_in_main_page_template(software_names=['sw 3.00.00', 'sw 3.00.01'])
    report_gen.add_main_page_description(tests_desc='This is example report - provide here some general info about '
                                                    'report content')

    report_gen.add_section(section_name='Section PT 1')
    report_gen.add_text(text='This is text section number 1. You can describe figure '
                                                          'content which is below.')
    report_gen.add_plot(fig=fig1, fig_name="Figure name 1")

    report_gen.add_section(section_name='Section PT 2')
    report_gen.add_table(df=df2, df_name="Table from pandas.DataFrame")
    report_gen.add_text(text_header='Text header', text='This can be some table values description')

    report_gen.generate_pdf_report(output_filename=r"C:\logs\test.pdf")
