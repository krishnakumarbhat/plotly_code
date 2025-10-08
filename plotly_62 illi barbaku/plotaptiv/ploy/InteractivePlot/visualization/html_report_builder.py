"""
HTML report generator for Interactive Plot System.

This module provides functionality to generate HTML reports.
"""
import os
import time
from typing import Dict, List, Any, Optional
import plotly.graph_objects as go

from InteractivePlot.interfaces.base_interfaces import IReportGenerator, IProgressReporter


class HtmlReportGenerator(IReportGenerator):
    """
    Generator for HTML reports containing visualizations.
    
    This class creates HTML reports with interactive Plotly visualizations.
    """
    
    def __init__(self, progress_reporter: Optional[IProgressReporter] = None):
        """
        Initialize the HTML report generator.
        
        Args:
            progress_reporter: Optional progress reporter
        """
        self.progress_reporter = progress_reporter
        
    def generate_report(self, visualizations: Dict[str, Any], output_file: str, output_dir: Optional[str] = None) -> str:
        """
        Generate an HTML report with visualizations.
        
        Args:
            visualizations: Dictionary of visualizations
            output_file: Name of the output file
            output_dir: Directory to save the report in
            
        Returns:
            Path to the generated report
        """
        # Use default output directory if none provided
        if output_dir is None:
            output_dir = "html"
        
        # Create the output directory if it doesn't exist
        os.makedirs(output_dir, exist_ok=True)
        
        # Full path to the output file
        output_path = os.path.join(output_dir, output_file)
        
        try:
            # Notify about report generation start
            if self.progress_reporter:
                self.progress_reporter.notify_observers(
                    progress=0.0,
                    message=f"Generating report: {output_file}"
                )
            
            # Create HTML content
            html_content = self._create_html_content(visualizations, output_file)
            
            # Write HTML file
            with open(output_path, 'w') as f:
                f.write(html_content)
            
            # Notify about report generation completion
            if self.progress_reporter:
                self.progress_reporter.notify_observers(
                    progress=100.0,
                    message=f"Report generated: {output_path}"
                )
            
            return output_path
        except Exception as e:
            # Report error
            if self.progress_reporter:
                self.progress_reporter.notify_observers(
                    progress=0.0,
                    message=f"Error generating report: {e}"
                )
            raise ValueError(f"Error generating HTML report: {e}")
    
    def _create_html_content(self, visualizations: Dict[str, Any], title: str) -> str:
        """
        Create HTML content with visualizations.
        
        Args:
            visualizations: Dictionary of visualizations
            title: Title of the HTML report
            
        Returns:
            HTML content as a string
        """
        # Create HTML header
        html = f"""
        <!DOCTYPE html>
        <html>
        <head>
            <title>{title}</title>
            <style>
                body {{
                    font-family: Arial, sans-serif;
                    margin: 0;
                    padding: 0;
                    background-color: #f5f5f5;
                }}
                .container {{
                    width: 90%;
                    margin: 20px auto;
                    background-color: white;
                    padding: 20px;
                    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
                }}
                .header {{
                    background-color: #4285f4;
                    color: white;
                    padding: 10px 20px;
                    margin-bottom: 20px;
                }}
                .tabs {{
                    display: flex;
                    border-bottom: 1px solid #ccc;
                    margin-bottom: 20px;
                }}
                .tab {{
                    padding: 10px 20px;
                    cursor: pointer;
                    background-color: #f1f1f1;
                    margin-right: 5px;
                    border: 1px solid #ccc;
                    border-bottom: none;
                }}
                .tab.active {{
                    background-color: white;
                    border-bottom: 1px solid white;
                    margin-bottom: -1px;
                }}
                .content {{
                    display: none;
                    padding: 20px;
                    border: 1px solid #ccc;
                    border-top: none;
                }}
                .content.active {{
                    display: block;
                }}
                .footer {{
                    padding: 10px;
                    background-color: #f1f1f1;
                    text-align: center;
                    font-size: 0.8em;
                    margin-top: 20px;
                }}
            </style>
            <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
        </head>
        <body>
            <div class="container">
                <div class="header">
                    <h1>{title}</h1>
                    <p>Generated on {time.strftime('%Y-%m-%d %H:%M:%S')}</p>
                </div>
        """
        
        # Add tabs
        html += """
                <div class="tabs">
        """
        
        # Create tab buttons
        for i, tab_name in enumerate(visualizations.keys()):
            active_class = "active" if i == 0 else ""
            html += f"""
                    <div class="tab {active_class}" onclick="openTab(event, 'tab{i}')">{tab_name}</div>
            """
        
        html += """
                </div>
        """
        
        # Add tab content
        for i, (tab_name, figures) in enumerate(visualizations.items()):
            active_class = "active" if i == 0 else ""
            html += f"""
                <div id="tab{i}" class="content {active_class}">
                    <h2>{tab_name}</h2>
            """
            
            # Add figures to tab
            for j, fig in enumerate(figures):
                if isinstance(fig, go.Figure):
                    # Convert Plotly figure to HTML
                    plot_div = fig.to_html(full_html=False, include_plotlyjs=False)
                    html += f"""
                    <div class="plot-container">
                        {plot_div}
                    </div>
                    """
                else:
                    # Handle non-Figure content (e.g., tables or text)
                    html += f"""
                    <div class="content-container">
                        {fig}
                    </div>
                    """
            
            html += """
                </div>
            """
        
        # Add footer and JavaScript
        html += """
                <div class="footer">
                    <p>Interactive Plot System - Generated Report</p>
                </div>
            </div>
            
            <script>
                function openTab(evt, tabName) {
                    // Hide all tab content
                    var tabContents = document.getElementsByClassName("content");
                    for (var i = 0; i < tabContents.length; i++) {
                        tabContents[i].className = tabContents[i].className.replace(" active", "");
                    }
                    
                    // Remove active class from all tabs
                    var tabs = document.getElementsByClassName("tab");
                    for (var i = 0; i < tabs.length; i++) {
                        tabs[i].className = tabs[i].className.replace(" active", "");
                    }
                    
                    // Show the selected tab content and add active class to the button
                    document.getElementById(tabName).className += " active";
                    evt.currentTarget.className += " active";
                }
            </script>
        </body>
        </html>
        """
        
        return html 