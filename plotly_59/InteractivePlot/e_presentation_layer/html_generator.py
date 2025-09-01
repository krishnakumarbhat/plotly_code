import os
import re
from InteractivePlot.e_presentation_layer.front_end import (
    html_template,
)
import aiofiles
import asyncio


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
    - Creates a separate folder for each input file
    - Generates a main HTML file for each input file
    """

    def __init__(
        self,
        plots_hash,
        kpi_plots,
        html_name,
        output_dir=None,
        input_filename="",
        output_filename="",
        sensor_position="",
    ):
        """
        Initialize the HTML generator with plot data and output parameters.

        Parameters:
            plots_hash (dict): Dictionary of plots organized by category
            kpi_plots (dict): Dictionary of KPI-specific plots
            html_name (str): Base name of the HTML file(s) to be generated (without extension)
            output_dir (str): Root directory to save the HTML files (defaults to "html")
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

        # Get base filename without extension for folder creation
        self.base_filename = (
            os.path.splitext(os.path.basename(input_filename))[0]
            if input_filename
            else "default"
        )

        # Create folder structure
        self.file_specific_dir = os.path.join(self.output_dir, self.base_filename)
        self.separate_html_dir = os.path.join(self.file_specific_dir, "separateHTML")

        # Merge KPI plots into plots under 'KPI' key if any
        if self.kpi_plots:
            self.plots["KPI"] = self.kpi_plots

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
        content_html = '<div class="tab-content"><table style="width:100%;">'
        for idx in range(0, len(plots_to_use), 2):
            content_html += "<tr>"
            for plot in plots_to_use[idx : idx + 2]:
                content_html += f'<td style="width:50%;">{plot.to_html(full_html=False, include_plotlyjs=False)}</td>'
            content_html += "</tr>"
        content_html += "</table></div>"

        final_html = (
            html_template.replace("{{TABS}}", "")  # No tabs
            .replace("{{CONTENT}}", content_html)
            .replace("{{INPUT_FILENAME}}", self.input_filename)
            .replace("{{OUTPUT_FILENAME}}", self.output_filename)
            .replace("{{SENSOR_POSITION}}", self.sensor_position)
        )

        return final_html

    def generate_and_save_html_files(self):
        """
        Generate and save separate HTML files for each plot category.
        Creates a folder structure with:
        - A folder named after the input file
        - A main HTML file within that folder
        - A separateHTML subfolder containing individual plot HTML files
        """
        if not self.plots:
            raise ValueError("No plots available to generate HTML content.")

        # Create main folder for this file
        os.makedirs(self.file_specific_dir, exist_ok=True)
        print(f"Created file-specific directory: {self.file_specific_dir}")

        # Create separateHTML subfolder
        os.makedirs(self.separate_html_dir, exist_ok=True)
        print(f"Created separateHTML directory: {self.separate_html_dir}")

        # Generate main HTML file with links to all separate HTML files
        main_html_links = []

        for key in self.plots.keys():
            try:
                html_content = self.generate_html_content_for_key(key)

                # Filename pattern: basefilename_key.html, lowercase and spaces replaced by underscores
                safe_key = key.lower().replace(" ", "_")
                filename = f"{self.html_name}_{safe_key}.html"

                # Save separate HTML file
                file_path = os.path.join(self.separate_html_dir, filename)
                with open(file_path, "w", encoding="utf-8") as file:
                    file.write(html_content)

                content_size_kb = len(html_content) / 1024
                print(
                    f"HTML report saved: {file_path} ({content_size_kb:.1f} KB, {len(html_content.splitlines())} lines)"
                )

                # Add link to the main HTML file
                relative_path = os.path.join("separateHTML", filename)
                main_html_links.append((filename, relative_path))

            except ValueError as e:
                print(f"Warning: {str(e)}")
                continue

        # Create main HTML file
        self.create_main_html_file(main_html_links)

    def create_main_html_file(self, links):
        """
        Create a main HTML file with links to all separate HTML files.
        If the main HTML file already exists, append new links instead of replacing.

        Parameters:
            links (list): List of tuples (display_name, relative_path) for each HTML file
        """
        main_html_path = os.path.join(
            self.file_specific_dir, f"{self.base_filename}.html"
        )

        # Check if main HTML file already exists
        existing_links = set()
        main_content = ""
        if os.path.exists(main_html_path):
            with open(main_html_path, "r", encoding="utf-8") as main_file:
                main_content = main_file.read()
                # Extract href links inside <a> tags
                hrefs = re.findall(r'<a\s+href="([^"]+)">', main_content)
                existing_links.update(hrefs)
                print(f"Found existing main HTML file with {len(existing_links)} links")

        # If file doesn't exist, create new HTML content
        if not main_content.strip():
            main_content = (
                "<html>\n"
                "<head>\n"
                f"<title>{self.base_filename} - HTML Report Index</title>\n"
                "<style>\n"
                "body { background: #f7f9fb; font-family: 'Segoe UI', Arial, sans-serif; margin: 0; padding: 2em; color: #222; }\n"
                "h1 { color: #2a357a; margin-bottom: 1.5em; text-align: center; letter-spacing: 2px; }\n"
                "ul { list-style-type: disc; background: #fff; border-radius: 10px; box-shadow: 0 2px 12px rgba(40,60,80,0.07); max-width: 600px; margin: 2em auto; padding: 2em 1em; }\n"
                "li { border-bottom: 1px solid #eee; padding: 1.2em; text-align: center; transition: background 0.2s; }\n"
                "li:last-child { border-bottom: none; }\n"
                "a { text-decoration: none; color: #2a357a; font-weight: 500; font-size: 1.1em; transition: color 0.2s; border-radius: 5px; padding: 0.5em 1em; display: inline-block; }\n"
                "a:hover { color: #fff; background: #2a357a; }\n"
                ".input-info { text-align: center; margin-bottom: 1em; color: #666; }\n"
                "</style>\n"
                "</head>\n"
                "<body>\n"
                f"<h1>{self.base_filename} - HTML Report Index</h1>\n"
                f"<div class='input-info'>Input file: {self.input_filename}</div>\n"
                "<ul>\n"
                "</ul>\n"
                "</body>\n"
                "</html>"
            )

        # Add new links if they don't already exist
        changes_made = False
        insert_index = main_content.rfind("</ul>")

        if insert_index == -1:
            # No </ul> tag found, add the list before </body>
            new_links_html = "<ul>\n"
            for display_name, relative_path in links:
                if relative_path not in existing_links:
                    new_links_html += (
                        f'  <li><a href="{relative_path}">{display_name}</a></li>\n'
                    )
                    changes_made = True
            new_links_html += "</ul>\n"

            body_index = main_content.rfind("</body>")
            if body_index != -1:
                main_content = (
                    main_content[:body_index]
                    + new_links_html
                    + main_content[body_index:]
                )
                changes_made = True
        else:
            # Insert new links before </ul>
            for display_name, relative_path in links:
                if relative_path not in existing_links:
                    new_link_html = (
                        f'  <li><a href="{relative_path}">{display_name}</a></li>\n'
                    )
                    main_content = (
                        main_content[:insert_index]
                        + new_link_html
                        + main_content[insert_index:]
                    )
                    changes_made = True

        # Save updated main HTML file
        if changes_made or not os.path.exists(main_html_path):
            with open(main_html_path, "w", encoding="utf-8") as main_file:
                main_file.write(main_content)
            print(f"Main HTML index updated: {main_html_path}")
        else:
            print(f"No changes needed for main HTML index: {main_html_path}")

    async def generate_html_async(self, plots_data, output_path):
        """Async HTML generation for better I/O performance"""
        html_content = await self._build_html_content_async(plots_data)
        
        async with aiofiles.open(output_path, 'w', encoding='utf-8') as f:
            await f.write(html_content)
    
    async def _build_html_content_async(self, plots_data):
        """Build HTML content asynchronously"""
        # Use asyncio.gather for concurrent template processing
        tasks = []
        for plot_name, plot_data in plots_data.items():
            task = asyncio.create_task(
                self._process_plot_template_async(plot_name, plot_data)
            )
            tasks.append(task)
        
        plot_htmls = await asyncio.gather(*tasks)
        return self._combine_html_sections(plot_htmls)
