from jinja2 import Environment, FileSystemLoader
from xhtml2pdf import pisa
from pathlib import Path
import os


class RenderHTMLToPDFXhtml2pdf:
    def __init__(self,
                 template_data: dict,
                 template_name='test_template.html',
                 output_filename="test.pdf",
                 css_name='style.css',
                 ):

        self.template_data = template_data
        self.template_path = str(Path(os.path.dirname(os.path.abspath(__file__))).parent / 'templates')
        self.template_name = template_name
        self.output_filename = output_filename
        self.css_path = str(Path(os.path.dirname(os.path.abspath(__file__))).parent / 'CSS' / css_name)

        self._data = None
        self._css = None
        self._source_html = None

    def _load_css(self):
        with open(self.css_path) as f:
            self._css = f.read()

    def _convert_html_to_pdf(self):
        # open output file for writing (truncated binary)
        result_file = open(self.output_filename, "w+b")

        # convert HTML to PDF
        pisa_status = pisa.CreatePDF(
            self._source_html,  # the HTML to convert
            dest=result_file,
            default_css=self._css)

        # close output file
        result_file.close()  # close output file

        # return False on success and True on errors
        return pisa_status.err

    def render_html_to_pdf(self):
        file_loader = FileSystemLoader(self.template_path)
        env = Environment(loader=file_loader)
        self._source_html = env.get_template(self.template_name).render(data=self.template_data)

        self._load_css()
        self._convert_html_to_pdf()
