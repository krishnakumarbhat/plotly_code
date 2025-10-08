from typing import Dict, List, Optional
from dataclasses import dataclass
import plotly.graph_objects as go
from pathlib import Path
import json

@dataclass
class HtmlConfig:
    """Configuration for HTML generation"""
    template_path: Optional[str] = None
    output_dir: str = "visualizations"
    include_plotlyjs: bool = True
    include_mathjax: bool = False
    include_bootstrap: bool = True

class HtmlGenerator:
    """
    Generates HTML reports with interactive Plotly visualizations,
    organized in a responsive layout with tabs and sections.
    """
    
    def __init__(self, plots: Dict[str, List[go.Figure]], 
                 kpi_data: Any, output_name: str,
                 config: Optional[HtmlConfig] = None):
        """
        Initialize HTML generator with plots and configuration.
        
        Args:
            plots: Dictionary mapping categories to lists of plotly figures
            kpi_data: KPI analysis data to include in the report
            output_name: Name for the output HTML file
            config: Optional HTML generation configuration
        """
        self.plots = plots
        self.kpi_data = kpi_data
        self.output_name = output_name
        self.config = config or HtmlConfig()
        
        # Create output directory if it doesn't exist
        Path(self.config.output_dir).mkdir(parents=True, exist_ok=True)
        
        # Generate and save the HTML report
        self._generate_html()
    
    def _generate_html(self) -> None:
        """Generate the HTML report with all visualizations and data."""
        html_content = []
        
        # Add HTML header and required dependencies
        html_content.extend(self._generate_header())
        
        # Add navigation tabs
        html_content.extend(self._generate_navigation())
        
        # Add plot sections
        html_content.extend(self._generate_plot_sections())
        
        # Add KPI section if available
        if self.kpi_data:
            html_content.extend(self._generate_kpi_section())
        
        # Add footer
        html_content.extend(self._generate_footer())
        
        # Write complete HTML to file
        output_path = Path(self.config.output_dir) / f"{self.output_name}.html"
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write('\n'.join(html_content))
    
    def _generate_header(self) -> List[str]:
        """Generate HTML header with required dependencies."""
        header = [
            "<!DOCTYPE html>",
            "<html lang='en'>",
            "<head>",
            "    <meta charset='UTF-8'>",
            "    <meta name='viewport' content='width=device-width, initial-scale=1.0'>",
            f"    <title>{self.output_name}</title>"
        ]
        
        # Add required CSS and JavaScript
        if self.config.include_bootstrap:
            header.extend([
                "    <link href='https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css' rel='stylesheet'>",
                "    <script src='https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.bundle.min.js'></script>"
            ])
        
        if self.config.include_plotlyjs:
            header.append("    <script src='https://cdn.plot.ly/plotly-latest.min.js'></script>")
            
        if self.config.include_mathjax:
            header.append("    <script src='https://polyfill.io/v3/polyfill.min.js?features=es6'></script>")
            header.append("    <script id='MathJax-script' src='https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js'></script>")
        
        header.extend([
            "    <style>",
            "        .plot-container { margin: 20px 0; }",
            "        .nav-tabs { margin-bottom: 20px; }",
            "        .tab-content { padding: 20px; }",
            "    </style>",
            "</head>",
            "<body>",
            "    <div class='container-fluid'>"
        ])
        
        return header
    
    def _generate_navigation(self) -> List[str]:
        """Generate navigation tabs for different sections."""
        nav = [
            "        <ul class='nav nav-tabs' role='tablist'>"
        ]
        
        # Add tab for each plot category
        for i, category in enumerate(self.plots.keys()):
            active = "active" if i == 0 else ""
            nav.extend([
                f"            <li class='nav-item' role='presentation'>",
                f"                <button class='nav-link {active}' data-bs-toggle='tab' ",
                f"                        data-bs-target='#{category}-tab' type='button'>",
                f"                    {category.title()}",
                f"                </button>",
                f"            </li>"
            ])
        
        # Add KPI tab if available
        if self.kpi_data:
            nav.extend([
                f"            <li class='nav-item' role='presentation'>",
                f"                <button class='nav-link' data-bs-toggle='tab' ",
                f"                        data-bs-target='#kpi-tab' type='button'>",
                f"                    KPI Analysis",
                f"                </button>",
                f"            </li>"
            ])
        
        nav.append("        </ul>")
        return nav
    
    def _generate_plot_sections(self) -> List[str]:
        """Generate sections containing plots for each category."""
        sections = ["        <div class='tab-content'>"]
        
        for i, (category, plot_list) in enumerate(self.plots.items()):
            active = "show active" if i == 0 else ""
            sections.extend([
                f"            <div class='tab-pane fade {active}' id='{category}-tab'>",
                f"                <h2>{category.title()}</h2>",
                "                <div class='row'>"
            ])
            
            # Add plots in a responsive grid
            for plot in plot_list:
                sections.extend([
                    "                    <div class='col-md-6 plot-container'>",
                    f"                        {plot.to_html(full_html=False, include_plotlyjs=False)}",
                    "                    </div>"
                ])
            
            sections.extend([
                "                </div>",
                "            </div>"
            ])
        
        sections.append("        </div>")
        return sections
    
    def _generate_kpi_section(self) -> List[str]:
        """Generate section for KPI analysis data."""
        kpi_section = [
            "            <div class='tab-pane fade' id='kpi-tab'>",
            "                <h2>KPI Analysis</h2>",
            "                <div class='row'>",
            "                    <div class='col-12'>",
            f"                        <pre>{json.dumps(self.kpi_data, indent=2)}</pre>",
            "                    </div>",
            "                </div>",
            "            </div>"
        ]
        return kpi_section
    
    def _generate_footer(self) -> List[str]:
        """Generate HTML footer."""
        return [
            "    </div>",
            "</body>",
            "</html>"
        ]
