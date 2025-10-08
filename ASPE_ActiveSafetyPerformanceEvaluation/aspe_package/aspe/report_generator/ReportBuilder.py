import os
import shutil

import matplotlib.pyplot as plt
import pandas as pd

from aspe.report_generator.PdfRenderers.RenderHTMLToPDFXhtml2pdf import RenderHTMLToPDFXhtml2pdf
from pathlib import Path


class ReportBuilder:
    def __init__(self):
        self._data = {'logo': str(Path(os.path.dirname(os.path.abspath(__file__))) /'templates' / 'aptiv_logo.png'),
                      'report_objs': {},
                      'titles': ['Aptiv Active Safety KPI report'],
                      'software_names': [],
                      'tests_desc': '',
                      }
        self._n_plots = 0
        self._temp_data_folder = '_temp_plots'
        if not Path(self._temp_data_folder).exists():
            Path(self._temp_data_folder).mkdir(parents=True)

        self._last_section_name = None

    def _check_section_if_valid(self):
        if self._last_section_name is None:
            raise ValueError('current section_name is invalid, please specify a correct one before proceeding')

    def add_report_titles(self, titles: list):
        self._data['titles'] = titles

    def add_software_names_in_main_page_template(self, software_names: list):
        self._data['software_names'] = software_names

    def add_main_page_description(self, tests_desc: str):
        self._data['tests_desc'] = tests_desc

    def add_logo(self, logo_path: str):
        self._data['logo'] = logo_path

    def add_section(self, section_name: str):
        if section_name not in self._data['report_objs']:
            self._data['report_objs'][section_name] = []
            self._last_section_name = section_name
        else:
            raise ValueError(f'section {section_name} already exists, are you sure about this...?')

    def _add_element(self, element_srt_type: str, element_name: str, element_object: object,
                     f_in_new_page: bool = False):
        self._check_section_if_valid()
        element = [element_srt_type, element_name, element_object, f_in_new_page]
        if self._last_section_name not in self._data['report_objs']:
            self._data['report_objs'][self._last_section_name] = [element]
        else:
            self._data['report_objs'][self._last_section_name] += [element]

    def add_text(self, text, text_header='', f_in_new_page: bool = False):
        self._add_element('text', text_header, text, f_in_new_page)

    def add_plot(self, fig: plt.figure, fig_name, f_in_new_page: bool = False):
        self._check_section_if_valid()

        self._n_plots += 1
        fig_save_path = f'_temp_plots/fig{self._n_plots}.jpg'
        fig_path = f'../_temp_plots/fig{self._n_plots}.jpg'
        fig.savefig(fig_save_path)
        plt.close(fig)
        self._add_element('fig', fig_name, fig_path, f_in_new_page)

    def add_table(self, df: pd.DataFrame, df_name, f_in_new_page: bool = False):
        self._add_element('tab', df_name, df.to_html(classes="table"), f_in_new_page)

    def generate_pdf_report(self,
                            template_name='test_template.html',
                            output_filename="test.pdf",
                            css_name='style.css', ):

        render = RenderHTMLToPDFXhtml2pdf(self._data, template_name, output_filename, css_name)
        render.render_html_to_pdf()

        try:
            shutil.rmtree(self._temp_data_folder)
        except OSError as ex:
            print(ex)
