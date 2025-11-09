"""
KPI HTML Report Generator
Generates HTML reports for KPI analysis results
"""

import json
import time
from datetime import datetime
from typing import Dict, Any, List
from pathlib import Path

class KPIHTMLGenerator:
    """Generates HTML reports for KPI analysis"""
    
    def __init__(self):
        self.template_dir = Path(__file__).parent / "templates"
        self.template_dir.mkdir(exist_ok=True)
        
    def generate_kpi_report(self, report_data: Dict[str, Any]) -> str:
        """
        Generate HTML report for KPI analysis
        
        Args:
            report_data: Dictionary containing KPI metrics and data summary
            
        Returns:
            HTML content as string
        """
        sensor_id = report_data.get("sensor_id", "Unknown")
        kpi_metrics = report_data.get("kpi_metrics", {})
        data_summary = report_data.get("data_summary", {})
        processing_timestamp = report_data.get("processing_timestamp", time.time())
        missing_fields = report_data.get("missing_fields", [])
        
        # Convert timestamp to readable format
        processing_time = datetime.fromtimestamp(processing_timestamp).strftime("%Y-%m-%d %H:%M:%S")
        
        html_content = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>KPI Report - Sensor {sensor_id}</title>
    <style>
        body {{
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f5f5f5;
            color: #333;
        }}
        .container {{
            max-width: 1200px;
            margin: 0 auto;
            background-color: white;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            overflow: hidden;
        }}
        .header {{
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            text-align: center;
        }}
        .header h1 {{
            margin: 0;
            font-size: 2.5em;
            font-weight: 300;
        }}
        .header p {{
            margin: 10px 0 0 0;
            opacity: 0.9;
            font-size: 1.1em;
        }}
        .content {{
            padding: 30px;
        }}
        .section {{
            margin-bottom: 40px;
        }}
        .section h2 {{
            color: #667eea;
            border-bottom: 2px solid #667eea;
            padding-bottom: 10px;
            margin-bottom: 20px;
        }}
        .metrics-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }}
        .metric-card {{
            background: #f8f9fa;
            border: 1px solid #e9ecef;
            border-radius: 8px;
            padding: 20px;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.05);
        }}
        .metric-card h3 {{
            margin: 0 0 15px 0;
            color: #495057;
            font-size: 1.2em;
        }}
        .metric-value {{
            font-size: 2em;
            font-weight: bold;
            color: #667eea;
            margin-bottom: 10px;
        }}
        .metric-description {{
            color: #6c757d;
            font-size: 0.9em;
            line-height: 1.4;
        }}
        .data-summary {{
            background: #e3f2fd;
            border: 1px solid #bbdefb;
            border-radius: 8px;
            padding: 20px;
            margin-bottom: 20px;
        }}
        .data-summary h3 {{
            margin: 0 0 15px 0;
            color: #1976d2;
        }}
        .summary-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
        }}
        .summary-item {{
            text-align: center;
        }}
        .summary-value {{
            font-size: 1.5em;
            font-weight: bold;
            color: #1976d2;
        }}
        .summary-label {{
            color: #424242;
            font-size: 0.9em;
            margin-top: 5px;
        }}
        .missing-fields {{
            background: #fff3e0;
            border: 1px solid #ffcc02;
            border-radius: 8px;
            padding: 20px;
            margin-bottom: 20px;
        }}
        .missing-fields h3 {{
            margin: 0 0 15px 0;
            color: #f57c00;
        }}
        .missing-list {{
            list-style: none;
            padding: 0;
            margin: 0;
        }}
        .missing-list li {{
            background: #fff8e1;
            margin: 5px 0;
            padding: 8px 12px;
            border-radius: 4px;
            color: #e65100;
        }}
        .footer {{
            background: #f8f9fa;
            padding: 20px;
            text-align: center;
            color: #6c757d;
            border-top: 1px solid #e9ecef;
        }}
        .status-indicator {{
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 8px;
        }}
        .status-success {{
            background-color: #28a745;
        }}
        .status-warning {{
            background-color: #ffc107;
        }}
        .status-error {{
            background-color: #dc3545;
        }}
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>KPI Analysis Report</h1>
            <p>Sensor ID: {sensor_id} | Generated: {processing_time}</p>
        </div>
        
        <div class="content">
            <!-- Data Summary Section -->
            <div class="section">
                <h2>Data Summary</h2>
                <div class="data-summary">
                    <h3>Data Completeness and Statistics</h3>
                    <div class="summary-grid">
                        <div class="summary-item">
                            <div class="summary-value">{data_summary.get('sensor_id', 'N/A')}</div>
                            <div class="summary-label">Sensor ID</div>
                        </div>
                        <div class="summary-item">
                            <div class="summary-value">{len(data_summary.get('streams_available', []))}</div>
                            <div class="summary-label">Available Streams</div>
                        </div>
                        <div class="summary-item">
                            <div class="summary-value">{len(missing_fields)}</div>
                            <div class="summary-label">Missing Fields</div>
                        </div>
                        <div class="summary-item">
                            <div class="summary-value">{len(kpi_metrics)}</div>
                            <div class="summary-label">KPI Metrics</div>
                        </div>
                    </div>
                </div>
                
                <!-- Data Completeness Details -->
                <div class="metrics-grid">
        """
        
        # Add data completeness metrics
        for stream_name, completeness in data_summary.get("data_completeness", {}).items():
            status_class = "status-success" if completeness >= 0.95 else "status-warning" if completeness >= 0.8 else "status-error"
            html_content += f"""
                    <div class="metric-card">
                        <h3>{stream_name.replace('_', ' ').title()}</h3>
                        <div class="metric-value">{completeness:.1%}</div>
                        <div class="metric-description">
                            <span class="status-indicator {status_class}"></span>
                            Data completeness for {stream_name.replace('_', ' ')} stream
                        </div>
                    </div>
            """
            
        html_content += """
                </div>
            </div>
            
            <!-- KPI Metrics Section -->
            <div class="section">
                <h2>KPI Metrics</h2>
                <div class="metrics-grid">
        """
        
        # Add KPI metrics
        for metric_name, metric_value in kpi_metrics.items():
            # Format metric value based on type
            if isinstance(metric_value, float):
                if abs(metric_value) < 0.01:
                    formatted_value = f"{metric_value:.4f}"
                else:
                    formatted_value = f"{metric_value:.2f}"
            else:
                formatted_value = str(metric_value)
                
            html_content += f"""
                    <div class="metric-card">
                        <h3>{metric_name.replace('_', ' ').title()}</h3>
                        <div class="metric-value">{formatted_value}</div>
                        <div class="metric-description">
                            Calculated KPI metric for {metric_name.replace('_', ' ')}
                        </div>
                    </div>
            """
            
        html_content += """
                </div>
            </div>
        """
        
        # Add missing fields section if any
        if missing_fields:
            html_content += f"""
            <div class="section">
                <h2>Missing Data Fields</h2>
                <div class="missing-fields">
                    <h3>Required fields not found in data</h3>
                    <ul class="missing-list">
            """
            
            for field in missing_fields:
                html_content += f"""
                        <li>{field}</li>
                """
                
            html_content += """
                    </ul>
                </div>
            </div>
            """
            
        html_content += """
        </div>
        
        <div class="footer">
            <p>Generated by KPI Analysis System | Processing completed at {processing_time}</p>
        </div>
    </div>
</body>
</html>
        """.format(processing_time=processing_time)
        
        return html_content
        
    def generate_simple_report(self, sensor_id: str, kpi_metrics: Dict[str, Any]) -> str:
        """
        Generate a simple HTML report for quick viewing
        
        Args:
            sensor_id: Sensor identifier
            kpi_metrics: Dictionary of KPI metrics
            
        Returns:
            Simple HTML content
        """
        html_content = f"""
<!DOCTYPE html>
<html>
<head>
    <title>KPI Report - {sensor_id}</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; }}
        .metric {{ margin: 10px 0; padding: 10px; background: #f0f0f0; }}
        .metric-name {{ font-weight: bold; }}
        .metric-value {{ color: #0066cc; }}
    </style>
</head>
<body>
    <h1>KPI Report for Sensor {sensor_id}</h1>
    <p>Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>
"""
        
        for metric_name, metric_value in kpi_metrics.items():
            html_content += f"""
    <div class="metric">
        <div class="metric-name">{metric_name}:</div>
        <div class="metric-value">{metric_value}</div>
    </div>
"""
            
        html_content += """
</body>
</html>
"""
        
        return html_content