import os
import h5py
import pandas as pd
import plotly.express as px
from front_end import html_template
from data_fetcher import DataFetcher

class PlotGenerator:
    """Class responsible for generating plots based on fetched data."""
    
    def __init__(self, hdf_name: str):
        self.hdf_name = hdf_name
        self.inputs, self.outputs = self.process_hdf_file(hdf_name)
        
        self.plots = self.create_plots()
        

        
    @staticmethod
    def save_html(output_directory: str, hdf_addr: str, html_content: str) -> None:
        """Save the generated HTML content to a file."""
        output_filename = os.path.join(output_directory, f'{hdf_addr}.html')
        with open(output_filename, 'w') as f:
            f.write(html_content)
            print(f"HTML file '{output_filename}' has been created.")
    
    def create_plots(self):
        """Create plots for each category of data."""
        plot_types = {
            'Scatter Plot': {},}

        for input_key, input_data in self.inputs.items():
            for output_key, output_data in self.outputs.items():
                # Convert HDF5 datasets to numpy arrays or lists
                input_values = input_data[:]
                output_values = output_data[:]

                scatter_fig = px.scatter(x=input_values, y=output_values,
                                        labels={'x': input_key, 'y': output_key},
                                        title=f'Scatter Plot between {input_key} and {output_key}')
                plot_types['Scatter Plot'][f'{input_key} vs {output_key}'] = scatter_fig

            
        return plot_types

    def generate_html_content(self):
        """Generate HTML content for the plots."""
        tabs_html = ''.join(f'<div class="tab" onclick="showTab(\'{key.lower().replace(" ", "-")}\')">{key}</div>' 
                            for key in self.plots.keys() if self.plots[key])

        content_html = ''.join(
            f'<div id="{key.lower().replace(" ", "-")}" class="tab-content">'
            f'  <table style="width:100%;">'
            + ''.join(
                f'<tr>'
                + ''.join(
                    f'<td style="width:50%;">{plot.to_html(full_html=False, include_plotlyjs=False)}</td>'
                    if idx % 2 == 0 else
                    f'<td style="width:50%;">{plot.to_html(full_html=False, include_plotlyjs=False)}</td></tr><tr>'
                    for idx, plot in enumerate(self.plots[key].values())
                )
                + '</tr>'
            )
            + '</table></div>'
            for key in self.plots.keys() if self.plots[key]
        )
        
        final_html = html_template.replace("{{TABS}}", tabs_html).replace("{{CONTENT}}", content_html)

        return final_html
