import os
import logging
import time
from datetime import datetime
from InteractivePlot.e_presentation_layer.front_end import html_template, minfy_css, minfy_js

class HtmlGenerator:
    """
    Class responsible for generating interactive HTML reports from Plotly visualizations.
    
    This component is part of the presentation layer and serves as the final step
    in the visualization pipeline. It takes plot data from the business layer
    and renders them into a self-contained HTML file with interactive features.
    
    Key features:
    - Organizes plots into tabbed sections
    - Creates a responsive layout for better viewing experience
    - Generates self-contained HTML with all JavaScript and CSS included
    - Automatically handles different plot types (regular plots and KPI plots)
    """
    
    def __init__(self, plots_hash, kpi_plots, html_name, output_dir=None, input_filename="", output_filename="", sensor_position=""):
        """
        Initialize the HTML generator with plot data and output parameters.
        
        Parameters:
            plots_hash (dict): Dictionary of plots organized by category/tab
            kpi_plots (dict): Dictionary of KPI-specific plots
            html_name (str): Name of the HTML file to be generated
            output_dir (str): Directory to save the HTML file (defaults to "html")
            input_filename (str): Name of the input file (for metadata display)
            output_filename (str): Name of the output file (for metadata display)
            sensor_position (str): Position of the sensor (e.g., "FL", "FR", "RL", "RR")
        """
        self.plots = plots_hash
        self.kpi_plots = kpi_plots
        self.html_name = html_name
        self.output_dir = output_dir or "html"  # Default to "html" if not provided
        self.input_filename = input_filename
        self.output_filename = output_filename
        self.sensor_position = sensor_position
        
        # Generate and save HTML in one go
        final_html = self.generate_html_content()
        self.save_html(self.html_name, final_html, self.output_dir)

    def generate_html_content(self):
        """
        Generate HTML content for the plots with interactive tabs.
        
        This method:
        1. Validates that plots exist to be rendered
        2. Combines regular plots with KPI plots (if any)
        3. Generates HTML tab navigation elements
        4. Creates the plot content organized in a responsive grid layout
        5. Populates the HTML template with generated content and metadata
        
        Returns:
            str: Complete HTML content as a string
        
        Raises:
            ValueError: If no plots are available to generate content
        """
        if not self.plots:
            raise ValueError("No plots available to generate HTML content.")

        # Combine regular plots with KPI plots
        all_plots = self.plots
        if self.kpi_plots:
            all_plots['KPI'] = self.kpi_plots

        # Generate tabs HTML for navigation
        tabs_html = ''.join(
            f'<div class="tab" onclick="showTab(\'{key.lower().replace(" ", "-")}\')">{key}</div>' 
            for key in all_plots.keys() if all_plots[key]
        )

        # Generate content HTML with plots arranged in a grid layout (2 columns)
        content_html = ''.join(
            f'<div id="{key.lower().replace(" ", "-")}" class="tab-content">'
            f'  <table style="width:100%;">'
            + ''.join(
                f'<tr>'
                + ''.join(
                    f'<td style="width:50%;">{plot.to_html(full_html=False, include_plotlyjs=False)}</td>'
                    for plot in (all_plots[key][key] if isinstance(all_plots[key], dict) and key in all_plots[key] else all_plots[key])[idx:idx+2]
                )
                + '</tr>'
                for idx in range(0, len(all_plots[key][key] if isinstance(all_plots[key], dict) and key in all_plots[key] else all_plots[key]), 2)
            )
            + '</table></div>'
            for key in all_plots.keys() if all_plots[key]
        )

        # Replace placeholders in the HTML template with actual content
        final_html = (html_template
                     .replace("{{TABS}}", tabs_html)
                     .replace("{{CONTENT}}", content_html)
                     .replace("{{INPUT_FILENAME}}", self.input_filename)
                     .replace("{{OUTPUT_FILENAME}}", self.output_filename)
                     .replace("{{SENSOR_POSITION}}", self.sensor_position))

        return final_html

    @staticmethod
    def save_html(filename, html_content, output_directory="html"):
        """
        Save the generated HTML content to a file.
        
        Parameters:
            filename (str): Name of the HTML file to save
            html_content (str): HTML content to write to the file
            output_directory (str): Directory to save the file (defaults to "html")
        
        This method will:
        1. Create the output directory if it doesn't exist
        2. Write the HTML content to the specified file
        3. Log a confirmation message with file size and line count
        """
        start_time = time.time()
        
        # Create output directory if it doesn't exist
        if not os.path.exists(output_directory):
            os.makedirs(output_directory)
            logging.info(f"Created output directory: {output_directory}")

        file_path = os.path.join(output_directory, filename)
        content_size_kb = len(html_content) / 1024
        
        # Write the HTML content to the file
        with open(file_path, 'w') as file:
            file.write(html_content)
        
        # Calculate the time taken
        end_time = time.time()
        time_taken = end_time - start_time
        
        # Log HTML report generation with time
        report_message = f"HTML report saved: {file_path} ({content_size_kb:.1f} KB, {len(html_content.splitlines())} lines)"
        time_message = f"Time taken to generate HTML report: {time_taken:.2f} seconds"
        
        # Log to both console and file
        logging.info(report_message)
        logging.info(time_message)
