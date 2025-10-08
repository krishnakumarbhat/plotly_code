################################# 
# Detection Mapping KPI HDF Implementation
#################################
import sys
import os
import glob
import re
import numpy as np
import pandas as pd
import plotly.graph_objs as go
import plotly.subplots as sp
import plotly.io as pio
from collections import Counter
import uuid
import time
import logging

from KPI.b_data_storage.kpi_data_model_storage import KPI_DataModelStorage
from KPI.b_data_storage.kpi_config_storage import KPI_VALIDATION_RULES

logger = logging.getLogger(__name__)

class DetectionMappingKPIHDF:
    """Detection KPI analysis using HDF data from self.input_data and self.output_data"""
    
    def __init__(self, data: KPI_DataModelStorage, 
                 sensor_id: str):
        self.input_data = data['DETECTION_STREAM']['input']
        self.output_data = data['DETECTION_STREAM']['output']
        self.sensor_id = sensor_id
        
        # Get thresholds from config
        self.thresholds = KPI_VALIDATION_RULES["detection_thresholds"]
        self.RAN_THRESHOLD = self.thresholds["range_threshold"]
        self.VEL_THRESHOLD = self.thresholds["velocity_threshold"]
        self.THETA_THRESHOLD = self.thresholds["theta_threshold"]
        self.PHI_THRESHOLD = self.thresholds["phi_threshold"]
        self.RADAR_CYCLE_S = self.thresholds["radar_cycle_s"]
        self.MAX_CDC_RECORDS = self.thresholds["max_cdc_records"]
        self.RANGE_SATURATION_THRESHOLD_FRONT = self.thresholds["range_saturation_threshold_front"]
        self.RANGE_SATURATION_THRESHOLD_CORNER = self.thresholds["range_saturation_threshold_corner"]
        self.MAX_NUM_OF_AF_DETS_FRONT = self.thresholds["max_num_af_dets_front"]
        self.MAX_NUM_OF_AF_DETS_CORNER = self.thresholds["max_num_af_dets_corner"]
        self.MAX_NUM_OF_RDD_DETS = self.thresholds["max_num_rdd_dets"]
        
        # Determine sensor type and set limits
        if '_FC_' in sensor_id.upper() or 'FRONT' in sensor_id.upper():
            self.MAX_NUM_OF_AF_DETS = self.MAX_NUM_OF_AF_DETS_FRONT
            self.RANGE_SATURATION_THRESHOLD = self.RANGE_SATURATION_THRESHOLD_FRONT
        else:
            self.MAX_NUM_OF_AF_DETS = self.MAX_NUM_OF_AF_DETS_CORNER
            self.RANGE_SATURATION_THRESHOLD = self.RANGE_SATURATION_THRESHOLD_CORNER
            
        # Initialize result variables
        self.html_content = ""
        self.kpi_results = {}
        
    def extract_data_from_hdf(self, data_model: KPI_DataModelStorage, stream_type: str):
        """Extract relevant data arrays from HDF KPI_DataModelStorage"""
        try:
            if stream_type == "DETECTION":
                # Get detection stream data
                det_data = {}
                if hasattr(data_model,) and data_model.scan_index is not None:
                    det_data['scan_index'] = np.array(data_model.scan_index)
                    
                # Get number of detections per scan
                if hasattr(data_model, 'num_af_det') and data_model.num_af_det is not None:
                    det_data['num_af_det'] = np.array(data_model.num_af_det)
                
                # Get detection parameters (assuming they are stored as arrays with indices)
                param_names = ['rdd_idx', 'ran', 'vel', 'theta', 'phi', 
                              'f_single_target', 'f_superres_target', 'f_bistatic']
                
                for param in param_names:
                    if hasattr(data_model, param) and getattr(data_model, param) is not None:
                        param_data = getattr(data_model, param)
                        if isinstance(param_data, (list, np.ndarray)):
                            det_data[param] = np.array(param_data)
                
                return det_data
                
            elif stream_type == "RDD":
                # Get RDD stream data
                rdd_data = {}
                if hasattr(data_model, 'scan_index') and data_model.scan_index is not None:
                    rdd_data['scan_index'] = np.array(data_model.scan_index)
                    
                if hasattr(data_model, 'rdd1_num_detect') and data_model.rdd1_num_detect is not None:
                    rdd_data['rdd1_num_detect'] = np.array(data_model.rdd1_num_detect)
                
                # Get RDD parameters
                rdd_params = ['rdd1_rindx', 'rdd1_dindx', 'rdd2_range', 'rdd2_range_rate']
                for param in rdd_params:
                    if hasattr(data_model, param) and getattr(data_model, param) is not None:
                        param_data = getattr(data_model, param)
                        if isinstance(param_data, (list, np.ndarray)):
                            rdd_data[param] = np.array(param_data)
                            
                return rdd_data
                
            elif stream_type == "VSE":
                # Get VSE stream data
                vse_data = {}
                if hasattr(data_model, 'scan_index') and data_model.scan_index is not None:
                    vse_data['scan_index'] = np.array(data_model.scan_index)
                    
                if hasattr(data_model, 'veh_speed') and data_model.veh_speed is not None:
                    vse_data['veh_speed'] = np.array(data_model.veh_speed)
                    
                return vse_data
                
        except Exception as e:
            logger.error(f"Error extracting {stream_type} data: {e}")
            return {}
            
    # def convert_to_dataframe(self, data_dict: dict, max_items: int, param_names: list):
    #     """Convert extracted HDF data to pandas DataFrame format similar to CSV"""
    #     try:
    #         if not data_dict or 'scan_index' not in data_dict:
    #             return pd.DataFrame()
                
    #         # Start with scan_index and num_detections
    #         base_cols = ['scan_index']
    #         df_data = {'scan_index': data_dict['scan_index']}
            
    #         # Add number of detections if available
    #         if 'num_af_det' in data_dict:
    #             df_data['num_af_det'] = data_dict['num_af_det']
    #             base_cols.append('num_af_det')
    #         elif 'rdd1_num_detect' in data_dict:
    #             df_data['rdd1_num_detect'] = data_dict['rdd1_num_detect']
    #             base_cols.append('rdd1_num_detect')
                
    #         # Create expanded columns for detection parameters
    #         for param in param_names:
    #             if param in data_dict:
    #                 param_data = data_dict[param]
    #                 # Reshape data to have max_items columns per scan
    #                 if len(param_data.shape) == 1:
    #                     # If 1D, reshape assuming it contains all detections sequentially
    #                     num_scans = len(data_dict['scan_index'])
    #                     param_data = param_data.reshape(num_scans, -1)
                    
    #                 # Pad or truncate to max_items
    #                 if param_data.shape[1] > max_items:
    #                     param_data = param_data[:, :max_items]
    #                 elif param_data.shape[1] < max_items:
    #                     pad_width = max_items - param_data.shape[1]
    #                     param_data = np.pad(param_data, ((0, 0), (0, pad_width)), 'constant')
                    
    #                 # Add columns to dataframe
    #                 for i in range(max_items):
    #                     col_name = f"{param}_{i}"
    #                     df_data[col_name] = param_data[:, i] if i < param_data.shape[1] else 0
                        
    #         df = pd.DataFrame(df_data)
            
    #         # Filter out zero scan indices
    #         if len(df) > 0:
    #             df = df[df['scan_index'] != 0]
                
    #         return df
            
    #     except Exception as e:
    #         logger.error(f"Error converting to dataframe: {e}")
    #         return pd.DataFrame()

    def process_detection_matching(self):
        """Main processing function for detection matching KPIs"""
        try:
            # # Extract data from HDF
            # input_det_data = self.extract_data_from_hdf(self.input_data, "DETECTION")
            # output_det_data = self.extract_data_from_hdf(self.output_data, "DETECTION")
            # input_rdd_data = self.extract_data_from_hdf(self.input_data, "RDD")
            # output_rdd_data = self.extract_data_from_hdf(self.output_data, "RDD")
            
            if not input_det_data or not output_det_data:
                logger.warning("Insufficient detection data for KPI analysis")
                return False
                
            # Convert to DataFrames
            det_params = ['rdd_idx', 'ran', 'vel', 'theta', 'phi', 'f_single_target', 'f_superres_target', 'f_bistatic']
            # veh_det_df = self.convert_to_dataframe(input_det_data, self.MAX_NUM_OF_AF_DETS, det_params)
            # sim_det_df = self.convert_to_dataframe(output_det_data, self.MAX_NUM_OF_AF_DETS, det_params)
            
            if len(veh_det_df) == 0 or len(sim_det_df) == 0:
                logger.warning("Empty detection dataframes after conversion")
                return False
                
            # Filter for non-zero detections
            veh_det_df = veh_det_df[veh_det_df['num_af_det'] > 0]
            sim_det_df = sim_det_df[sim_det_df['num_af_det'] > 0]
            
            # Process RDD data if available
            rdd_kpis = {}
            if input_rdd_data and output_rdd_data:
                rdd_kpis = self.process_rdd_matching(input_rdd_data, output_rdd_data)
                
            # Process AF detection matching
            af_kpis = self.process_af_detection_matching(veh_det_df, sim_det_df)
            
            # Combine results
            self.kpi_results = {
                'rdd_kpis': rdd_kpis,
                'af_kpis': af_kpis,
                'sensor_id': self.sensor_id,
                'num_input_scans': len(veh_det_df),
                'num_output_scans': len(sim_det_df)
            }
            
            # Generate HTML report
            self.generate_html_report()
            
            return True
            
        except Exception as e:
            logger.error(f"Error in detection matching process: {e}")
            return False
            
    def process_rdd_matching(self, input_rdd_data, output_rdd_data):
        """Process RDD stream matching"""
        try:
            rdd_params = ['rdd1_rindx', 'rdd1_dindx', 'rdd2_range', 'rdd2_range_rate']
            # veh_rdd_df = self.convert_to_dataframe(input_rdd_data, self.MAX_NUM_OF_RDD_DETS, rdd_params)
            # sim_rdd_df = self.convert_to_dataframe(output_rdd_data, self.MAX_NUM_OF_RDD_DETS, rdd_params)
            
            if len(veh_rdd_df) == 0 or len(sim_rdd_df) == 0:
                return {}
                
            # Filter for non-zero detections
            veh_rdd_df = veh_rdd_df[veh_rdd_df['rdd1_num_detect'] > 0]
            sim_rdd_df = sim_rdd_df[sim_rdd_df['rdd1_num_detect'] > 0]
            
            # Merge dataframes
            merged_df = pd.merge(veh_rdd_df, sim_rdd_df, on='scan_index', suffixes=('_veh', '_sim'))
            
            if len(merged_df) == 0:
                return {}
                
            # Calculate RDD matching metrics
            num_same_si = len(merged_df)
            num_same_num_dets = len(merged_df[merged_df['rdd1_num_detect_veh'] == merged_df['rdd1_num_detect_sim']])
            
            # Calculate matching percentages
            rdd_kpis = {
                'num_same_si': num_same_si,
                'num_same_num_dets': num_same_num_dets,
                'same_num_dets_pct': round((num_same_num_dets / num_same_si * 100), 2) if num_same_si > 0 else 0
            }
            
            return rdd_kpis
            
        except Exception as e:
            logger.error(f"Error in RDD matching: {e}")
            return {}
            
    def process_af_detection_matching(self, veh_det_df, sim_det_df):
        """Process AF detection matching"""
        try:
            # Merge dataframes
            merged_df = pd.merge(veh_det_df, sim_det_df, on='scan_index', suffixes=('_veh', '_sim'))
            
            if len(merged_df) == 0:
                return {}
                
            # Calculate basic metrics
            num_same_si = len(merged_df)
            num_same_num_dets = len(merged_df[merged_df['num_af_det_veh'] == merged_df['num_af_det_sim']])
            
            # Calculate detection matching within thresholds
            matched_detections = 0
            total_detections_veh = merged_df['num_af_det_veh'].sum()
            
            for _, row in merged_df.iterrows():
                num_dets_veh = int(row['num_af_det_veh'])
                num_dets_sim = int(row['num_af_det_sim'])
                
                # Simple matching based on position thresholds
                matches = 0
                for i in range(min(num_dets_veh, num_dets_sim, self.MAX_NUM_OF_AF_DETS)):
                    ran_veh = row.get(f'ran_{i}_veh', 0)
                    ran_sim = row.get(f'ran_{i}_sim', 0)
                    vel_veh = row.get(f'vel_{i}_veh', 0)
                    vel_sim = row.get(f'vel_{i}_sim', 0)
                    theta_veh = row.get(f'theta_{i}_veh', 0)
                    theta_sim = row.get(f'theta_{i}_sim', 0)
                    phi_veh = row.get(f'phi_{i}_veh', 0)
                    phi_sim = row.get(f'phi_{i}_sim', 0)
                    
                    if (abs(ran_veh - ran_sim) <= self.RAN_THRESHOLD and
                        abs(vel_veh - vel_sim) <= self.VEL_THRESHOLD and
                        abs(theta_veh - theta_sim) <= self.THETA_THRESHOLD and
                        abs(phi_veh - phi_sim) <= self.PHI_THRESHOLD):
                        matches += 1
                        
                matched_detections += matches
                
            # Calculate KPI metrics
            accuracy = round((matched_detections / total_detections_veh * 100), 2) if total_detections_veh > 0 else 0
            
            af_kpis = {
                'num_same_si': num_same_si,
                'num_same_num_dets': num_same_num_dets,
                'same_num_dets_pct': round((num_same_num_dets / num_same_si * 100), 2) if num_same_si > 0 else 0,
                'total_detections_veh': total_detections_veh,
                'matched_detections': matched_detections,
                'accuracy': accuracy
            }
            
            return af_kpis
            
        except Exception as e:
            logger.error(f"Error in AF detection matching: {e}")
            return {}
            
    def generate_html_report(self):
        """Generate HTML report for detection KPIs"""
        try:
            af_kpis = self.kpi_results.get('af_kpis', {})
            rdd_kpis = self.kpi_results.get('rdd_kpis', {})
            
            self.html_content = f"""
            <div class="kpi-section">
                <h3>Detection Mapping KPI Results - {self.sensor_id}</h3>
                
                <div class="kpi-summary">
                    <p><strong>Overall Accuracy:</strong> {af_kpis.get('accuracy', 0):.2f}%</p>
                    <p><strong>Total Detections (Input):</strong> {af_kpis.get('total_detections_veh', 0)}</p>
                    <p><strong>Matched Detections:</strong> {af_kpis.get('matched_detections', 0)}</p>
                    <p><strong>Scans Processed:</strong> {af_kpis.get('num_same_si', 0)}</p>
                </div>
                
                <details>
                    <summary><strong>Detailed Results</strong></summary>
                    
                    <h4>AF Detection Stream</h4>
                    <ul>
                        <li>Scans with same number of AF detections: {af_kpis.get('num_same_num_dets', 0)}/{af_kpis.get('num_same_si', 0)} 
                            ({af_kpis.get('same_num_dets_pct', 0):.2f}%)</li>
                        <li>Detection accuracy (all parameters): {af_kpis.get('accuracy', 0):.2f}%</li>
                    </ul>
                    
                    <h4>RDD Stream</h4>
                    <ul>
                        <li>Scans processed: {rdd_kpis.get('num_same_si', 0)}</li>
                        <li>Scans with same number of RDD detections: {rdd_kpis.get('num_same_num_dets', 0)}/{rdd_kpis.get('num_same_si', 0)} 
                            ({rdd_kpis.get('same_num_dets_pct', 0):.2f}%)</li>
                    </ul>
                    
                    <h4>Thresholds Used</h4>
                    <ul>
                        <li>Range threshold: {self.RAN_THRESHOLD:.6f} m</li>
                        <li>Velocity threshold: {self.VEL_THRESHOLD:.6f} m/s</li>
                        <li>Azimuth threshold: {self.THETA_THRESHOLD:.6f} rad</li>
                        <li>Elevation threshold: {self.PHI_THRESHOLD:.6f} rad</li>
                    </ul>
                </details>
            </div>
            <hr>
            """
            
        except Exception as e:
            logger.error(f"Error generating HTML report: {e}")
            self.html_content = f"<p>Error generating detection KPI report: {e}</p>"

    def get_results(self):
        """Return KPI results and HTML content"""
        return {
            'kpi_results': self.kpi_results,
            'html_content': self.html_content,
            'success': bool(self.kpi_results)
        }

# Main processing function to be called by KPI factory
def process_detection_kpi(data: KPI_DataModelStorage, 
                         sensor_id: str) -> dict:
    """
    Main function to process detection KPIs from HDF data
    
    Args:
        input_data: KPI_DataModelStorage with input/vehicle data
        output_data: KPI_DataModelStorage with output/simulation data  
        sensor_id: Sensor identifier
        stream_name: Stream name being processed
        
    Returns:
        dict: KPI results and HTML content
    """
    try:
        processor = DetectionMappingKPIHDF(data, sensor_id)
        success = processor.process_detection_matching()
        
        if success:
            return processor.get_results()
        else:
            return {
                'kpi_results': {},
                'html_content': f"<p>Failed to process detection KPIs for {sensor_id}</p>",
                'success': False
            }
            
    except Exception as e:
        logger.error(f"Error in process_detection_kpi: {e}")
        return {
            'kpi_results': {},
            'html_content': f"<p>Error processing detection KPIs: {e}</p>",
            'success': False
        }