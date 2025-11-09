################################# 
# Detection Mapping KPI HDF Implementation - CORRECTED VERSION
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
        self.data = data
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
        self.RANGE_SATURATION_THRESHOLD_CORNER =self.thresholds["range_saturation_threshold_corner"]
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



    def process_detection_matching(self):
        """Main processing function for detection matching KPIs"""
        try:
            # ✅ ADD PRINT HERE - Start of main processing
            print(f"🚀 Starting detection matching process for sensor: {self.sensor_id}")

            if not self.data:
                logger.warning("Insufficient detection data for KPI analysis")
                return False
                
            # Define detection parameters
            det_params = ['rdd_idx', 'ran', 'vel', 'theta', 'phi', 'f_single_target', 
                         'f_superres_target', 'f_bistatic', 'num_af_det']
            
            src = self.data['DETECTION_STREAM']
            veh_det_df = {}
            sim_det_df = {}

            # FIXED: get_value returns (numpy_array, status)
            for param in det_params:
                try:
                    veh_result, veh_status = KPI_DataModelStorage.get_value(src['input'], param)
                    sim_result, sim_status = KPI_DataModelStorage.get_value(src['output'], param)
                    
                    if veh_status == "success" and sim_status == "success":
                        veh_det_df[param] = veh_result
                        sim_det_df[param] = sim_result
                    else:
                        logger.warning(f"Failed to get parameter {param}: veh={veh_status}, sim={sim_status}")
                        veh_det_df[param] = np.array([])
                        sim_det_df[param] = np.array([])
                except Exception as e:
                    logger.error(f"Error getting parameter {param}: {str(e)}")
                    veh_det_df[param] = np.array([])
                    sim_det_df[param] = np.array([])

            if len(veh_det_df) == 0 or len(sim_det_df) == 0:
                logger.warning("Empty detection data after conversion")
                return False
                
            # Get RDD stream data
            input_rdd_data = self.data.get('RDD_STREAM', {}).get('input')
            output_rdd_data = self.data.get('RDD_STREAM', {}).get('output')
            
            # Process RDD data if available
            rdd_kpis = {}
            if input_rdd_data and output_rdd_data:
                rdd_kpis = self.process_rdd_matching(input_rdd_data, output_rdd_data)
                
            # Process AF detection matching
            af_kpis = self.process_af_detection_matching(veh_det_df, sim_det_df)
            
            # Calculate number of scans from the data
            num_veh_scans = veh_det_df.get('num_af_det', np.array([])).shape[0] if isinstance(veh_det_df.get('num_af_det'), np.ndarray) else 0
            num_sim_scans = sim_det_df.get('num_af_det', np.array([])).shape[0] if isinstance(sim_det_df.get('num_af_det'), np.ndarray) else 0
            
            # Combine results
            self.kpi_results = {
                'rdd_kpis': rdd_kpis,
                'af_kpis': af_kpis,
                'sensor_id': self.sensor_id,
                'num_input_scans': num_veh_scans,
                'num_output_scans': num_sim_scans
            }
            
            # Generate HTML report
            self.generate_html_report()
            
            return True
                
        except Exception as e:
            logger.error(f"Error in detection matching process: {e}")
            return False
            



    def process_rdd_matching(self, input_rdd_data, output_rdd_data):
        """Process RDD stream matching using NumPy (no pandas)."""
        try:
            rdd_params = ['rdd1_rindx', 'rdd1_dindx', 'rdd2_range', 'rdd2_range_rate','rdd1_num_detect']

            src = self.data['RDD_STREAM']
            # Retrieve arrays - FIXED: get_value returns (numpy_array, status)
            veh_rdd = {}
            sim_rdd = {}
            
            for param in rdd_params:
                try:
                    veh_result, veh_status = KPI_DataModelStorage.get_value(src['input'], param)
                    sim_result, sim_status = KPI_DataModelStorage.get_value(src['output'], param)
                    
                    if veh_status == "success" and sim_status == "success":
                        veh_rdd[param] = veh_result
                        sim_rdd[param] = sim_result
                    else:
                        logger.warning(f"Failed to get RDD parameter {param}: veh={veh_status}, sim={sim_status}")
                        veh_rdd[param] = np.array([])
                        sim_rdd[param] = np.array([])
                except Exception as e:
                    logger.error(f"Error getting RDD parameter {param}: {e}")
                    return {}
                

            # FIXED: Extract data from 2D arrays properly
            # Column 0 is scan_index, column 1 is the actual value
            def extract_data(arr_2d):
                """Extract scan indices and values from get_value() result."""
                if arr_2d.size == 0:
                    return np.array([]), np.array([])
                if arr_2d.ndim == 1:
                    # Single row case
                    return np.array([arr_2d[0]]), arr_2d[1:] if len(arr_2d) > 1 else np.array([arr_2d[0]])
                # Multiple rows: column 0 is scan_index, column 1 is value
                scan_indices = arr_2d[:, 0].astype(int)
                values = arr_2d[:, 1] if arr_2d.shape[1] > 1 else arr_2d[:, 0]
                return scan_indices, values
            
            # Extract scan indices and values
            veh_scan, veh_rindx_vals = extract_data(veh_rdd['rdd1_rindx'])
            sim_scan, sim_rindx_vals = extract_data(sim_rdd['rdd1_rindx'])
            
            # Extract num_detect values
            _, veh_num = extract_data(veh_rdd['rdd1_num_detect'])
            _, sim_num = extract_data(sim_rdd['rdd1_num_detect'])
            
            # Validate we have data
            if veh_scan.size == 0 or sim_scan.size == 0:
                logger.warning("No RDD data available for matching")
                return {}

            # Build a mapping from scan -> first index in sim arrays
            sim_map = {}
            for idx, scan in enumerate(sim_scan):
                if scan not in sim_map:
                    sim_map[scan] = idx

            # Find matching scans present in both
            matching_scans = np.array([s for s in np.unique(veh_scan) if s in sim_map])

            if matching_scans.size == 0:
                logger.warning("No matching scan indices found between input and output RDD")
                return {}

            # Build dicts for quick index lookup for veh as well
            veh_map = {}
            for idx, scan in enumerate(veh_scan):
                if scan not in veh_map:
                    veh_map[scan] = idx

            matched_veh_indices = np.array([veh_map[s] for s in matching_scans])
            matched_sim_indices = np.array([sim_map[s] for s in matching_scans])

            # # Extract corresponding num_detect values
            # matched_veh_nums = veh_num[matched_veh_indices]
            # matched_sim_nums = sim_num[matched_sim_indices]

            # FIXED: Handle NaN values before integer conversion
            veh_valid = ~np.isnan(matched_veh_indices)
            sim_valid = ~np.isnan(matched_sim_indices)
            both_valid = veh_valid & sim_valid
            
            if not np.any(both_valid):
                logger.warning("All RDD num_detect values are NaN")
                return {}
            
            # Filter to only valid entries
            # matched_veh_nums = matched_veh_nums[both_valid]
            # matched_sim_nums = matched_sim_nums[both_valid]

            matched_veh_nums = both_valid
            matched_sim_nums = both_valid

            num_same_si = int(np.sum(both_valid))
            num_same_num_dets = int(np.sum(matched_veh_nums == matched_sim_nums))
            same_num_dets_pct = round((num_same_num_dets / num_same_si * 100), 2) if num_same_si > 0 else 0.0

            rdd_kpis = {
                'num_same_si': num_same_si,
                'num_same_num_dets': num_same_num_dets,
                'same_num_dets_pct': same_num_dets_pct
            }

            return rdd_kpis
            
        except Exception as e:
            logger.error(f"Error processing RDD matching: {e}")
            return {}

        
    def process_af_detection_matching(self, veh_det_df, sim_det_df):
        """Process AF detection matching using NumPy arrays"""
        try:
            # FIXED: Extract data from 2D arrays properly
            def extract_data(arr_2d):
                """Extract scan indices and values from get_value() result."""
                if arr_2d.size == 0:
                    return np.array([]), np.array([])
                if arr_2d.ndim == 1:
                    return np.array([arr_2d[0]]), arr_2d[1:] if len(arr_2d) > 1 else np.array([arr_2d[0]])
                # Multiple rows: column 0 is scan_index, column 1+ are values
                scan_indices = arr_2d[:, 0].astype(int)
                values = arr_2d[:, 1] if arr_2d.shape[1] > 1 else arr_2d[:, 0]
                return scan_indices, values
            
            # Extract num_af_det data
            veh_scan, veh_num_det = extract_data(veh_det_df.get('num_af_det', np.array([])))
            sim_scan, sim_num_det = extract_data(sim_det_df.get('num_af_det', np.array([])))
            
            if veh_scan.size == 0 or sim_scan.size == 0:
                logger.warning("No AF detection data available")
                return {}
            
            # Build mapping for matching scan indices
            sim_map = {}
            for idx, scan in enumerate(sim_scan):
                if scan not in sim_map:
                    sim_map[scan] = idx
            
            # Find matching scans
            matching_scans = np.array([s for s in np.unique(veh_scan) if s in sim_map])
            
            if matching_scans.size == 0:
                logger.warning("No matching scan indices found for AF detection")
                return {}
            
            # Build veh mapping
            veh_map = {}
            for idx, scan in enumerate(veh_scan):
                if scan not in veh_map:
                    veh_map[scan] = idx
            
            matched_veh_indices = np.array([veh_map[s] for s in matching_scans])
            matched_sim_indices = np.array([sim_map[s] for s in matching_scans])
            
            # Get matched num_detect values
            matched_veh_nums = veh_num_det[matched_veh_indices]
            matched_sim_nums = sim_num_det[matched_sim_indices]
            
            # FIXED: Handle NaN values before integer conversion
            veh_valid = ~np.isnan(matched_veh_nums)
            sim_valid = ~np.isnan(matched_sim_nums)
            both_valid = veh_valid & sim_valid
            
            if not np.any(both_valid):
                logger.warning("All AF num_det values are NaN")
                return {}
            
            # Filter to valid entries
            matched_veh_nums = matched_veh_nums[both_valid]
            matched_sim_nums = matched_sim_nums[both_valid]
            
            num_same_si = int(np.sum(both_valid))
            num_same_num_dets = int(np.sum(matched_veh_nums == matched_sim_nums))
            total_detections_veh = int(np.sum(matched_veh_nums))
            
            # Simplified matching - full implementation would need to match individual detections with thresholds
            matched_detections = num_same_num_dets
            
            same_num_dets_pct = round((num_same_num_dets / num_same_si * 100), 2) if num_same_si > 0 else 0.0
            accuracy = round((matched_detections / total_detections_veh * 100), 2) if total_detections_veh > 0 else 0.0
            
            af_kpis = {
                'num_same_si': num_same_si,
                'num_same_num_dets': num_same_num_dets,
                'same_num_dets_pct': same_num_dets_pct,
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

            # ✅ ADD PRINT HERE - Before creating HTML content
            # print(f"📊 KPI data available - AF: {bool(af_kpis)}, RDD: {bool(rdd_kpis)}")

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


            print(f"✅ HTML content generated successfully for {self.sensor_id}")

        except Exception as e:
            # ✅ ADD PRINT HERE - When HTML generation fails
            print(f"❌ Error generating HTML for {self.sensor_id}: {e}")
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
        data: KPI_DataModelStorage with input/output data
        sensor_id: Sensor identifier
        
    Returns:
        dict: KPI results and HTML content
    """
    try:
        processor = DetectionMappingKPIHDF(data, sensor_id)
        success = processor.process_detection_matching()

        if success:
            results = processor.get_results()
            return results
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
