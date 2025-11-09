#################################
# Alignment Matching KPI HDF Implementation
#################################
import sys
import os
import re
import numpy as np
import pandas as pd
import plotly.graph_objs as go
import plotly.subplots as sp
import plotly.io as pio
import json
import logging

from KPI.b_data_storage.kpi_data_model_storage import KPI_DataModelStorage
from KPI.b_data_storage.kpi_config_storage import KPI_VALIDATION_RULES

logger = logging.getLogger(__name__)

class AlignmentMappingKPIHDF:
    """Alignment KPI analysis using HDF data from self.input_data and self.output_data"""
    
    def __init__(self, data: KPI_DataModelStorage, 
                 sensor_id: str, stream_name: str):
        self.data = data
        self.sensor_id = sensor_id
        self.stream_name = stream_name
        
        # Get thresholds from config
        self.thresholds = KPI_VALIDATION_RULES["alignment_thresholds"]
        self.az_misalign_threshold = self.thresholds["az_misalign_threshold"]
        self.el_misalign_threshold = self.thresholds["el_misalign_threshold"]
        
        # Initialize result variables
        self.html_content = ""
        self.kpi_results = {}
        self.scan_index_list = []
        self.az_misalign_lists = {}
        self.el_misalign_lists = {}
        
    def extract_alignment_data_from_hdf(self, data_model: KPI_DataModelStorage):
        """Extract alignment data from HDF KPI_DataModelStorage"""
        try:
            alignment_data = {}
            
            # List of alignment signals to extract
            alignment_signals = [
                "vacs_boresight_az_nominal",
                "vacs_boresight_az_estimated", 
                "vacs_boresight_az_kf_internal",
                "vacs_boresight_el_nominal",
                "vacs_boresight_el_estimated",
                "vacs_boresight_el_kf_internal"
            ]
            
            # Extract scan indices if available
            if hasattr(data_model, 'scan_index') and data_model.scan_index is not None:
                alignment_data['scan_index'] = np.array(data_model.scan_index)
            
            # Extract each alignment signal
            for signal in alignment_signals:
                if hasattr(data_model, signal) and getattr(data_model, signal) is not None:
                    signal_data = getattr(data_model, signal)
                    if isinstance(signal_data, (list, np.ndarray)):
                        alignment_data[signal] = np.array(signal_data)
                    else:
                        alignment_data[signal] = signal_data
                        
            return alignment_data
            
        except Exception as e:
            logger.error(f"Error extracting alignment data: {e}")
            return {}
            
    def process_alignment_matching(self):
        """Main processing function for alignment matching KPIs"""
        try:
            # Extract data from HDF
            input_align_data = self.extract_alignment_data_from_hdf(self.input_data)
            output_align_data = self.extract_alignment_data_from_hdf(self.output_data)
            
            if not input_align_data or not output_align_data:
                logger.warning("Insufficient alignment data for KPI analysis")
                return False
                
            # Check for required signals
            required_signals = [
                "vacs_boresight_az_nominal",
                "vacs_boresight_az_estimated", 
                "vacs_boresight_az_kf_internal",
                "vacs_boresight_el_nominal",
                "vacs_boresight_el_estimated",
                "vacs_boresight_el_kf_internal"
            ]
            
            missing_input = [sig for sig in required_signals if sig not in input_align_data]
            missing_output = [sig for sig in required_signals if sig not in output_align_data]
            
            if missing_input or missing_output:
                logger.warning(f"Missing alignment signals - Input: {missing_input}, Output: {missing_output}")
                return False
                
            # Get scan indices
            if 'scan_index' in input_align_data:
                self.scan_index_list = input_align_data['scan_index'].tolist()
            else:
                # Generate scan indices if not available
                data_length = len(input_align_data[required_signals[0]])
                self.scan_index_list = list(range(1, data_length + 1))
            
            # Calculate misalignments and differences
            alignment_results = self.calculate_alignment_kpis(input_align_data, output_align_data)
            
            # Store results
            self.kpi_results = alignment_results
            
            # Generate HTML report
            self.generate_html_report()
            
            return True
            
        except Exception as e:
            logger.error(f"Error in alignment matching process: {e}")
            return False
            
    def calculate_alignment_kpis(self, input_data, output_data):
        """Calculate alignment KPIs and misalignment differences"""
        try:
            # Extract signal arrays
            signals = {
                'az_nominal_real': input_data['vacs_boresight_az_nominal'],
                'az_estimated_real': input_data['vacs_boresight_az_estimated'],
                'az_kf_internal_real': input_data['vacs_boresight_az_kf_internal'],
                'el_nominal_real': input_data['vacs_boresight_el_nominal'],
                'el_estimated_real': input_data['vacs_boresight_el_estimated'],
                'el_kf_internal_real': input_data['vacs_boresight_el_kf_internal'],
                'az_nominal_sim': output_data['vacs_boresight_az_nominal'],
                'az_estimated_sim': output_data['vacs_boresight_az_estimated'],
                'az_kf_internal_sim': output_data['vacs_boresight_az_kf_internal'],
                'el_nominal_sim': output_data['vacs_boresight_el_nominal'],
                'el_estimated_sim': output_data['vacs_boresight_el_estimated'],
                'el_kf_internal_sim': output_data['vacs_boresight_el_kf_internal']
            }
            
            # Calculate misalignments in degrees (convert from radians)
            az_misalign_est_real = (signals['az_nominal_real'] - signals['az_estimated_real']) * (180 / np.pi)
            az_misalign_ref_real = (signals['az_nominal_real'] - signals['az_kf_internal_real']) * (180 / np.pi)
            az_misalign_est_sim = (signals['az_nominal_sim'] - signals['az_estimated_sim']) * (180 / np.pi)
            az_misalign_ref_sim = (signals['az_nominal_sim'] - signals['az_kf_internal_sim']) * (180 / np.pi)
            
            el_misalign_est_real = (signals['el_nominal_real'] - signals['el_estimated_real']) * (180 / np.pi)
            el_misalign_ref_real = (signals['el_nominal_real'] - signals['el_kf_internal_real']) * (180 / np.pi)
            el_misalign_est_sim = (signals['el_nominal_sim'] - signals['el_estimated_sim']) * (180 / np.pi)
            el_misalign_ref_sim = (signals['el_nominal_sim'] - signals['el_kf_internal_sim']) * (180 / np.pi)
            
            # Calculate differences between real and sim
            az_misalign_est_diff = az_misalign_est_real - az_misalign_est_sim
            el_misalign_est_diff = el_misalign_est_real - el_misalign_est_sim
            
            # Store lists for plotting
            self.az_misalign_lists = {
                'est_real': az_misalign_est_real.tolist(),
                'ref_real': az_misalign_ref_real.tolist(),
                'est_sim': az_misalign_est_sim.tolist(),
                'ref_sim': az_misalign_ref_sim.tolist(),
                'est_diff': az_misalign_est_diff.tolist()
            }
            
            self.el_misalign_lists = {
                'est_real': el_misalign_est_real.tolist(),
                'ref_real': el_misalign_ref_real.tolist(),
                'est_sim': el_misalign_est_sim.tolist(),
                'ref_sim': el_misalign_ref_sim.tolist(),
                'est_diff': el_misalign_est_diff.tolist()
            }
            
            # Count matches within thresholds
            num_scans_total = len(az_misalign_est_diff)
            num_scans_az_match = np.sum(np.abs(az_misalign_est_diff) < self.az_misalign_threshold)
            num_scans_el_match = np.sum(np.abs(el_misalign_est_diff) < self.el_misalign_threshold)
            
            # Calculate KPI percentages
            az_accuracy = round((num_scans_az_match / num_scans_total) * 100, 2) if num_scans_total > 0 else 0
            el_accuracy = round((num_scans_el_match / num_scans_total) * 100, 2) if num_scans_total > 0 else 0
            
            # Return KPI results
            kpi_results = {
                'az_kpi': {
                    'numerator': int(num_scans_az_match),
                    'denominator': num_scans_total,
                    'accuracy': az_accuracy
                },
                'el_kpi': {
                    'numerator': int(num_scans_el_match),
                    'denominator': num_scans_total,
                    'accuracy': el_accuracy
                },
                'num_scans': num_scans_total,
                'thresholds': {
                    'az_threshold': self.az_misalign_threshold,
                    'el_threshold': self.el_misalign_threshold
                }
            }
            
            return kpi_results
            
        except Exception as e:
            logger.error(f"Error calculating alignment KPIs: {e}")
            return {}
            
    def generate_plots(self):
        """Generate interactive plots for alignment data"""
        try:
            # Plot misalignment vs scan index
            fig_misalign = sp.make_subplots(rows=2, cols=1, vertical_spacing=0.2,
                                          subplot_titles=('Azimuth Misalignment', 'Elevation Misalignment'))
            
            # Azimuth plots
            fig_misalign.add_trace(go.Scatter(x=self.scan_index_list, y=self.az_misalign_lists['est_real'], 
                                            mode="lines", name="Veh Az Est", line=dict(color="blue")), row=1, col=1)
            fig_misalign.add_trace(go.Scatter(x=self.scan_index_list, y=self.az_misalign_lists['ref_real'], 
                                            mode="lines", name="Veh Az Ref", line=dict(color="blue"), opacity=0.2), row=1, col=1)
            fig_misalign.add_trace(go.Scatter(x=self.scan_index_list, y=self.az_misalign_lists['est_sim'], 
                                            mode="lines", name="Sim Az Est", line=dict(color="red")), row=1, col=1)
            fig_misalign.add_trace(go.Scatter(x=self.scan_index_list, y=self.az_misalign_lists['ref_sim'], 
                                            mode="lines", name="Sim Az Ref", line=dict(color="red"), opacity=0.2), row=1, col=1)
            
            # Elevation plots
            fig_misalign.add_trace(go.Scatter(x=self.scan_index_list, y=self.el_misalign_lists['est_real'], 
                                            mode="lines", name="Veh El Est", line=dict(color="blue")), row=2, col=1)
            fig_misalign.add_trace(go.Scatter(x=self.scan_index_list, y=self.el_misalign_lists['ref_real'], 
                                            mode="lines", name="Veh El Ref", line=dict(color="blue"), opacity=0.2), row=2, col=1)
            fig_misalign.add_trace(go.Scatter(x=self.scan_index_list, y=self.el_misalign_lists['est_sim'], 
                                            mode="lines", name="Sim El Est", line=dict(color="red")), row=2, col=1)
            fig_misalign.add_trace(go.Scatter(x=self.scan_index_list, y=self.el_misalign_lists['ref_sim'], 
                                            mode="lines", name="Sim El Ref", line=dict(color="red"), opacity=0.2), row=2, col=1)
            
            fig_misalign.update_yaxes(title_text="Azimuth Misalignment (deg)", row=1, col=1)
            fig_misalign.update_yaxes(title_text="Elevation Misalignment (deg)", row=2, col=1)
            fig_misalign.update_xaxes(title_text="Scan Index", row=1, col=1)
            fig_misalign.update_xaxes(title_text="Scan Index", row=2, col=1)
            fig_misalign.update_layout(height=1000, width=1250, title_text="Misalignment vs Scan Index", showlegend=True)
            
            misalign_plot_html = pio.to_html(fig_misalign, full_html=False, include_plotlyjs='cdn')
            
            # Plot misalignment difference vs scan index
            fig_diff = sp.make_subplots(rows=2, cols=1, vertical_spacing=0.2,
                                       subplot_titles=('Azimuth Misalignment Difference', 'Elevation Misalignment Difference'))
            
            fig_diff.add_trace(go.Scatter(x=self.scan_index_list, y=self.az_misalign_lists['est_diff'], 
                                        mode="lines", name="Az Diff", line=dict(color="blue")), row=1, col=1)
            fig_diff.add_trace(go.Scatter(x=self.scan_index_list, y=self.el_misalign_lists['est_diff'], 
                                        mode="lines", name="El Diff", line=dict(color="red")), row=2, col=1)
            
            fig_diff.update_yaxes(title_text="Azimuth Misalignment Difference (deg)", row=1, col=1)
            fig_diff.update_yaxes(title_text="Elevation Misalignment Difference (deg)", row=2, col=1)
            fig_diff.update_xaxes(title_text="Scan Index", row=1, col=1)
            fig_diff.update_xaxes(title_text="Scan Index", row=2, col=1)
            fig_diff.update_layout(height=1000, width=1250, title_text="Misalignment Difference vs Scan Index", showlegend=True)
            
            diff_plot_html = pio.to_html(fig_diff, full_html=False, include_plotlyjs='cdn')
            
            return misalign_plot_html, diff_plot_html
            
        except Exception as e:
            logger.error(f"Error generating plots: {e}")
            return "", ""
            
    def generate_html_report(self):
        """Generate HTML report for alignment KPIs"""
        try:
            az_kpi = self.kpi_results.get('az_kpi', {})
            el_kpi = self.kpi_results.get('el_kpi', {})
            
            # Generate plots
            misalign_plot_html, diff_plot_html = self.generate_plots()
            
            self.html_content = f"""
            <div class="kpi-section">
                <h3>Alignment Matching KPI Results - {self.sensor_id}</h3>
                
                <div class="kpi-summary">
                    <p><strong>Azimuth Accuracy:</strong> ({az_kpi.get('numerator', 0)}/{az_kpi.get('denominator', 0)}) → <strong>{az_kpi.get('accuracy', 0):.2f}%</strong></p>
                    <p><strong>Elevation Accuracy:</strong> ({el_kpi.get('numerator', 0)}/{el_kpi.get('denominator', 0)}) → <strong>{el_kpi.get('accuracy', 0):.2f}%</strong></p>
                    <p><strong>Total Scans Processed:</strong> {self.kpi_results.get('num_scans', 0)}</p>
                </div>
                
                <details>
                    <summary><strong>Detailed Results</strong></summary>
                    
                    <h4>Threshold Configuration</h4>
                    <ul>
                        <li>Azimuth misalignment threshold: {self.az_misalign_threshold:.2f} degrees</li>
                        <li>Elevation misalignment threshold: {self.el_misalign_threshold:.2f} degrees</li>
                    </ul>
                    
                    <h4>Alignment Analysis</h4>
                    <p>This KPI measures the accuracy of alignment estimation by comparing the difference between 
                    nominal and estimated boresight angles in vehicle data versus simulation data.</p>
                    
                    <h4>Interactive Plots</h4>
                    <details>
                        <summary><strong>Plot A: Misalignment vs Scan Index</strong></summary>
                        {misalign_plot_html}
                    </details>
                    
                    <details>
                        <summary><strong>Plot B: Misalignment Difference vs Scan Index</strong></summary>
                        {diff_plot_html}
                    </details>
                </details>
            </div>
            <hr>
            """
            
        except Exception as e:
            logger.error(f"Error generating HTML report: {e}")
            self.html_content = f"<p>Error generating alignment KPI report: {e}</p>"

    def get_results(self):
        """Return KPI results and HTML content"""
        return {
            'kpi_results': self.kpi_results,
            'html_content': self.html_content,
            'success': bool(self.kpi_results)
        }

# Main processing function to be called by KPI factory
def process_alignment_kpi(input_data: KPI_DataModelStorage, output_data: KPI_DataModelStorage, 
                         sensor_id: str, stream_name: str) -> dict:
    """
    Main function to process alignment KPIs from HDF data
    
    Args:
        input_data: KPI_DataModelStorage with input/vehicle data
        output_data: KPI_DataModelStorage with output/simulation data  
        sensor_id: Sensor identifier
        stream_name: Stream name being processed
        
    Returns:
        dict: KPI results and HTML content
    """
    try:
        processor = AlignmentMappingKPIHDF(input_data, output_data, sensor_id, stream_name)
        success = processor.process_alignment_matching()
        
        if success:
            return processor.get_results()
        else:
            return {
                'kpi_results': {},
                'html_content': f"<p>Failed to process alignment KPIs for {sensor_id}</p>",
                'success': False
            }
            
    except Exception as e:
        logger.error(f"Error in process_alignment_kpi: {e}")
        return {
            'kpi_results': {},
            'html_content': f"<p>Error processing alignment KPIs: {e}</p>",
            'success': False
        }