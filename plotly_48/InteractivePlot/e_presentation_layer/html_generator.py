import os
from InteractivePlot.e_presentation_layer.front_end import html_template, minfy_css, minfy_js

class HtmlGenerator:
    """Class responsible for generating plots based on fetched data."""
    
    def __init__(self, plots_hash, kpi_plots, html_name, output_dir=None, input_filename="", output_filename="", sensor_position=""):
        self.plots = plots_hash
        self.html_name = self._simplify_report_name(html_name, sensor_position)
        self.output_dir = output_dir or "html"  # Default to "html" if not provided
        self.input_filename = input_filename
        self.output_filename = output_filename
        self.sensor_position = sensor_position
        final_html = self.generate_html_content()
        self.save_html(self.html_name, final_html, self.output_dir)

    def _simplify_report_name(self, html_name, sensor_position):
        """Simplifies the report name to input+sensor.html instead of input+output+sensor.html"""
        if '+' in html_name:
            parts = html_name.split('+')
            if len(parts) > 1 and sensor_position:
                # Keep just input name + sensor position
                return f"{parts[0]}+{sensor_position}.html"
        return html_name

    def generate_html_content(self):
        """Generate HTML content for the plots."""
        if not self.plots:
            raise ValueError("No plots available to generate HTML content.")

        # Generate tabs HTML
        tabs_html = ''.join(
            f'<div class="tab" onclick="showTab(\'{key.lower().replace(" ", "-")}\')">{key}</div>' 
            for key in self.plots.keys() if self.plots[key]
        )
        # Generate content HTML
        content_html = ''.join(
            f'<div id="{key.lower().replace(" ", "-")}" class="tab-content">'
            f'  <table style="width:100%;">'
            + ''.join(
                f'<tr>'
                + ''.join(
                    f'<td style="width:50%;">{plot.to_html(full_html=False, include_plotlyjs=False)}</td>'
                    for plot in self.plots[key][idx:idx+2]
                )
                + '</tr>'
                for idx in range(0, len(self.plots[key]), 2)
            )
            + '</table></div>'
            for key in self.plots.keys() if self.plots[key]
        )

        # Replace placeholders in the HTML template
        final_html = (html_template
                     .replace("{{TABS}}", tabs_html)
                     .replace("{{CONTENT}}", content_html)
                     .replace("{{INPUT_FILENAME}}", self.input_filename)
                     .replace("{{OUTPUT_FILENAME}}", self.output_filename)
                     .replace("{{SENSOR_POSITION}}", self.sensor_position))

        return final_html


    @staticmethod
    def save_html(filename, html_content, output_directory="html"):
        """Save the generated HTML content to a file."""
        # Create output directory if it doesn't exist
        if not os.path.exists(output_directory):
            os.makedirs(output_directory)
            print(f"Created output directory: {output_directory}")

        file_path = os.path.join(output_directory, filename)
        content_size_kb = len(html_content) / 1024
        
        # Write the HTML content to the file
        with open(file_path, 'w') as file:
            file.write(html_content)
        
        print(f"HTML report saved: {file_path} ({content_size_kb:.1f} KB, {len(html_content.splitlines())} lines)")
