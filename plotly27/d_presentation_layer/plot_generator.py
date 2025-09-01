import os
from c_business_layer.front_end import html_template, minfy_css ,minfy_js

class PlotGenerator:
    """Class responsible for generating plots based on fetched data."""
    
    def __init__(self, plots_hash):
        self.plots = plots_hash

    def generate_html_content(self):
        """Generate HTML content for the plots."""
        if not self.plots:
            raise ValueError("No plots available to generate HTML content.")

        tabs_html = ''.join(
            f'<div class="tab" onclick="showTab(\'{key.lower().replace(" ", "-")}\')">{key}</div>' 
            for key in self.plots.keys() if self.plots[key]
        )

        content_html = ''.join(
            f'<div id="{key.lower().replace(" ", "-")}" class="tab-content">'
            f'  <table style="width:100%;">'
            + ''.join(
                f'<tr>'
                + ''.join(
                    f'<td style="width:50%;">{plot.to_html(full_html=False, include_plotlyjs=False)}</td>'
                    if idx % 2 == 0 else ''
                    for idx, plot in enumerate(self.plots[key])
                )
                + '</tr>'
            )
            + '</table></div>'
            for key in self.plots.keys() if self.plots[key]
        )
        
        final_html = html_template.replace("{{TABS}}", tabs_html).replace("{{CONTENT}}", content_html)

        return final_html
    
    @staticmethod
    def save_html(output_directory, filename, html_content):
        """Save the generated HTML content to a file."""
        if not os.path.exists(output_directory):
            os.makedirs(output_directory)

        file_path = os.path.join(output_directory, filename)
        
        with open(file_path, 'w') as file:
            file.write(html_content)
        
        print(f"HTML saved successfully at: {file_path}")
