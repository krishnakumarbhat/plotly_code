import os
import pdfkit
from jinja2 import Environment, FileSystemLoader


class BaseBuilder:
    """Base report section builder class. All sections builders derive from it. """

    templates_dir = 'templates'
    css_path = r"CSS\default.css"

    def __init__(self, template_name, plotter):
        self.template_name = template_name
        self.plotter = plotter
        self.base_builder_path = os.path.split(os.path.abspath(__file__))[0]
        template_folder = os.path.join(self.base_builder_path, BaseBuilder.templates_dir)
        self.env = Environment(loader=FileSystemLoader(template_folder))
        self.html_out = str
        self.template_args = {}

    def render_html(self):
        template = self.env.get_template(self.template_name)
        self.html_out = template.render(self.template_args)

    def save_pdf(self, folder_path):
        wkhtmltopdf_path = os.path.join(self.base_builder_path, r'wkhtmltopdf\bin\wkhtmltopdf.exe')
        configuration = pdfkit.configuration(wkhtmltopdf=wkhtmltopdf_path)
        css_path = os.path.join(self.base_builder_path, self.css_path)
        pdfkit.from_string(self.html_out, folder_path, css=css_path, configuration=configuration)

    """Virtual methods"""
    def init_template_args(self):
        raise NotImplementedError

    def build_results(self):
        raise NotImplementedError
