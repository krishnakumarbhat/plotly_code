import os
import logging
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
            plots_hash (dict): Dictionary of plots organized by category
            kpi_plots (dict): Dictionary of KPI-specific plots
            html_name (str): Base name of the HTML file(s) to be generated (without extension)
            output_dir (str): Directory to save the HTML files (defaults to "html")
            input_filename (str): Name of the input file (for metadata display)
            output_filename (str): Name of the output file (for metadata display)
            sensor_position (str): Position of the sensor (e.g., "FL", "FR", "RL", "RR")
        """
        self.plots = plots_hash or {}
        self.kpi_plots = kpi_plots or {}
        self.html_name = os.path.splitext(html_name)[0]  # Remove extension if provided
        self.output_dir = output_dir or "html"
        self.input_filename = input_filename
        self.output_filename = output_filename
        self.sensor_position = sensor_position
        
        # Merge KPI plots into plots under 'KPI' key if any
        if self.kpi_plots:
            self.plots['KPI'] = self.kpi_plots
        
        # Generate and save separate HTML files for each plot category
        self.generate_and_save_html_files()

    def generate_html_content_for_key(self, key):
        """
        Generate HTML content for a single plot category key without tabs.
        
        Parameters:
            key (str): The plot category key
        
        Returns:
            str: Complete HTML content for that category
        """
        plots_to_use = []
        if key in self.plots:
            # Handle case where plots are nested dicts or list
            if isinstance(self.plots[key], dict) and key in self.plots[key]:
                plots_to_use = self.plots[key][key]
            else:
                plots_to_use = self.plots[key]
        
        if not plots_to_use:
            raise ValueError(f"No plots available for category '{key}'.")

        # Create plot content arranged in a 2-column grid
        content_html = f'<div class="tab-content"><table style="width:100%;">'
        for idx in range(0, len(plots_to_use), 2):
            content_html += '<tr>'
            for plot in plots_to_use[idx:idx+2]:
                content_html += f'<td style="width:50%;">{plot.to_html(full_html=False, include_plotlyjs=False)}</td>'
            content_html += '</tr>'
        content_html += '</table></div>'

        final_html = (html_template
                      .replace("{{TABS}}", "")  # No tabs
                      .replace("{{CONTENT}}", content_html)
                      .replace("{{INPUT_FILENAME}}", self.input_filename)
                      .replace("{{OUTPUT_FILENAME}}", self.output_filename)
                      .replace("{{SENSOR_POSITION}}", self.sensor_position))
        
        return final_html

    def generate_and_save_html_files(self):
        """
        Generate and save separate HTML files for each plot category.
        """
        if not self.plots:
            raise ValueError("No plots available to generate HTML content.")

        if not os.path.exists(self.output_dir):
            os.makedirs(self.output_dir)
            print(f"Created output directory: {self.output_dir}")

        for key in self.plots.keys():
            try:
                html_content = self.generate_html_content_for_key(key)
            except ValueError:
                continue
            
            # Filename pattern: basefilename_key.html, lowercase and spaces replaced by underscores
            safe_key = key.lower().replace(" ", "_")
            filename = f"{self.html_name}_{safe_key}.html"
            self.save_html(filename, html_content, self.output_dir)

    @staticmethod
    def save_html(filename, html_content, output_directory="html"):
        """
        Save the generated HTML content to a file inside 'seprate_html' folder.
        Also, create/update 'main.html' in output_directory with links to all HTML files inside 'seprate_html'.
        
        Parameters:
            filename (str): Name of the HTML file to save (e.g., 'report1.html')
            html_content (str): HTML content to write to the file
            output_directory (str): Directory to save files (default is "html")
        """
        os.makedirs(output_directory, exist_ok=True)

        separate_dir = os.path.join(output_directory, "seprate_html")
        os.makedirs(separate_dir, exist_ok=True)
        
        file_path = os.path.join(separate_dir, filename)
        content_size_kb = len(html_content) / 1024
        
        with open(file_path, 'w', encoding='utf-8') as file:
            file.write(html_content)
        
        print(f"HTML report saved: {file_path} ({content_size_kb:.1f} KB, {len(html_content.splitlines())} lines)")
        
        # Path to main.html in output_directory (outside 'seprate_html')
        main_html_path = os.path.join(output_directory, "main.html")
        
        # Read existing links from main.html if it exists
        existing_links = set()
        if os.path.exists(main_html_path):
            with open(main_html_path, 'r', encoding='utf-8') as main_file:
                main_content = main_file.read()
                # Extract href links inside <a> tags
                import re
                hrefs = re.findall(r'<a\s+href="([^"]+)">', main_content)
                existing_links.update(hrefs)
        else:
            main_content = ""
        
        # Relative path from main.html to the saved file
        relative_path = os.path.join("seprate_html", filename)
        
        if relative_path not in existing_links:
            if not main_content.strip():
                main_content = (
                    "<html>\n<head><title>Main HTML Index</title></head>\n<body>\n<ul>\n</ul>\n</body>\n</html>"
                )
            
            insert_index = main_content.rfind("</ul>")
            if insert_index == -1:
                main_content = main_content.replace("</body>", f"<ul>\n<li><a href=\"{relative_path}\">{filename}</a></li>\n</ul>\n</body>")
            else:
                # Insert the new link before </ul>
                new_link_html = f'  <li><a href="{relative_path}">{filename}</a></li>\n'
                main_content = main_content[:insert_index] + new_link_html + main_content[insert_index:]
            
            # Write updated main.html
            with open(main_html_path, 'w', encoding='utf-8') as main_file:
                main_file.write(main_content)
            
            logging.debug(f"Updated main.html with link to {filename}")
        else:
            print(f"Link to {filename} already exists in main.html")
