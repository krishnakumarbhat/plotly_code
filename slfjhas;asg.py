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
        self.data = data
        # Example of writing data to a file
        try:
            with open('output.txt', 'a') as f:
                
                from dataclasses import asdict
                sensor = self.data['DETECTION_STREAM']['input']
                f.write(f"Processing data for sensor: {asdict(sensor).get('name', sensor)}\n")
                                
                print("Type:", type(sensor))
                print("Attrs:", [a for a in dir(sensor) if not a.startswith('_')])

        except Exception as e:
            logger.error(f"Error writing to file: {e}")

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

    def process_rdd_matching(self, input_rdd_data, output_rdd_data):
        """Process RDD stream matching using NumPy (no pandas)."""
        # try:
        rdd_params = ['rdd1_rindx', 'rdd1_dindx', 'rdd2_range', 'rdd2_range_rate','rdd1_num_detect']

        src = self.data['RDD_STREAM']
        # Retrieve arrays - FIXED: get_value returns (numpy_array, status)
        veh_rdd = {}
        sim_rdd = {}
        
        for param in rdd_params:
            # try:
            veh_result, veh_status = KPI_DataModelStorage.get_value(src['input'], param)
            sim_result, sim_status = KPI_DataModelStorage.get_value(src['output'], param)
            
            if veh_status == "success" and sim_status == "success":
                veh_rdd[param] = veh_result
                sim_rdd[param] = sim_result
            else:
                logger.warning(f"Failed to get RDD parameter {param}: veh={veh_status}, sim={sim_status} problem in hdf give proper HDF")
                veh_rdd[param] = np.array([])
                sim_rdd[param] = np.array([])
            # except Exception as e:
            #     logger.error(f"Error getting RDD parameter {param}: {e}")
            #     return {}
            





    def process_detection_matching(self):
        """Main processing function for detection matching KPIs"""
        try:
            print(f"*Starting detection matching process for sensor: {self.sensor_id}")

            if not self.data:
                logger.warning("Insufficient detection data for KPI analysis")
                return False
                
            # Define detection parameters
            det_params = ['rdd_idx', 'ran', 'vel', 'theta', 'phi', 'f_single_target', 
                         'f_superres_target', 'f_bistatic', 'num_af_det']
            
            src = self.data['DETECTION_STREAM']
            veh_det_df = {}
            sim_det_df = {}

            for param in det_params:
                
                veh_result, veh_status = KPI_DataModelStorage.get_value(src['input'], param)
                sim_result, sim_status = KPI_DataModelStorage.get_value(src['output'], param)
                
                if veh_status == "success" and sim_status == "success":
                    veh_det_df[param] = veh_result
                    sim_det_df[param] = sim_result
                else:
                    logger.warning(f"Failed to get parameter {param}: veh={veh_status}, sim={sim_status} give proper HDF")
                    veh_det_df[param] = np.array([])
                    sim_det_df[param] = np.array([])
                # except Exception as e:
                #     logger.error(f"Error getting parameter {param}: {str(e)}")
                #     veh_det_df[param] = np.array([])
                #     sim_det_df[param] = np.array([])

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
            af_kpis = self.process_af_detection_matching(veh_det_df, sim_det_df)\
            
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
            




#             # Column 0 is scan_index, column 1 is the actual value
#             def extract_data(arr_2d):
#                 """Extract scan indices and values from get_value() result."""
#                 if arr_2d.size == 0:
#                     return np.array([]), np.array([])
#                 if arr_2d.ndim == 1:
#                     # Single row case
#                     return np.array([arr_2d[0]]), arr_2d[1:] if len(arr_2d) > 1 else np.array([arr_2d[0]])
#                 # Multiple rows: column 0 is scan_index, column 1 is value
#                 scan_indices = arr_2d[:, 0].astype(int)
#                 values = arr_2d[:, 1] if arr_2d.shape[1] > 1 else arr_2d[:, 0]
#                 return scan_indices, values
            
#             # Extract scan indices and values
#             veh_scan, veh_rindx_vals = extract_data(veh_rdd['rdd1_rindx'])
#             sim_scan, sim_rindx_vals = extract_data(sim_rdd['rdd1_rindx'])
            
#             # Extract num_detect values
#             veha, veh_num = extract_data(veh_rdd['rdd1_num_detect'])
#             sima, sim_num = extract_data(sim_rdd['rdd1_num_detect'])
            
#             # Validate we have data
#             if veh_scan.size == 0 or sim_scan.size == 0:
#                 logger.warning("No RDD data available for matching")
#                 return {}

#             # Build a mapping from scan -> first index in sim arrays
#             sim_map = {}
#             for idx, scan in enumerate(sim_scan):
#                 if scan not in sim_map:
#                     sim_map[scan] = idx

#             # Find matching scans present in both
#             matching_scans = np.array([s for s in np.unique(veh_scan) if s in sim_map])

#             if matching_scans.size == 0:
#                 logger.warning("No matching scan indices found between input and output RDD")
#                 return {}

#             # Build dicts for quick index lookup for veh as well
#             veh_map = {}
#             for idx, scan in enumerate(veh_scan):
#                 if scan not in veh_map:
#                     veh_map[scan] = idx

#             # matched_veh_indices = np.array([veh_map[s] for s in matching_scans])
#             # matched_sim_indices = np.array([sim_map[s] for s in matching_scans])

#             # Extract range and range rate data
#             veh_range_scan, veh_range = extract_data(veh_rdd['rdd2_range'])
#             veh_rr_scan, veh_range_rate = extract_data(veh_rdd['rdd2_range_rate'])
#             sim_range_scan, sim_range = extract_data(sim_rdd['rdd2_range'])
#             sim_rr_scan, sim_range_rate = extract_data(sim_rdd['rdd2_range_rate'])

#             # Initialize match count
#             match_count = 0
#             total_comparisons = 0

#             # For each matching scan, compare range and range rate
#             for scan in matching_scans:
#                 # Get indices for this scan
#                 veh_idx = veh_map.get(scan, -1)
#                 sim_idx = sim_map.get(scan, -1)
                
#                 if veh_idx == -1 or sim_idx == -1:
#                     continue

#                 # Get the number of detections for this scan
#                 veh_num_det = int(veh_num[veh_idx]) if veh_idx < len(veh_num) else 0
#                 sim_num_det = int(sim_num[sim_idx]) if sim_idx < len(sim_num) else 0
                
#                 # Compare each detection
#                 for v in range(veh_num_det):
#                     for s in range(sim_num_det):
#                         # Get range and range rate values
#                         veh_r = veh_range[veh_idx + v] if (veh_idx + v) < len(veh_range) else 0
#                         veh_rr = veh_range_rate[veh_idx + v] if (veh_idx + v) < len(veh_range_rate) else 0
#                         sim_r = sim_range[sim_idx + s] if (sim_idx + s) < len(sim_range) else 0
#                         sim_rr = sim_range_rate[sim_idx + s] if (sim_idx + s) < len(sim_range_rate) else 0
                        
#                         # Check if within thresholds
#                         if (abs(sim_r - veh_r) <= self.RAN_THRESHOLD and 
#                             abs(sim_rr - veh_rr) <= self.VEL_THRESHOLD):
#                             match_count += 1
#                             break  # Each vehicle detection can match at most one sim detection
                
#                 total_comparisons += max(veh_num_det, sim_num_det)

#             if total_comparisons == 0:
#                 logger.warning("No valid detections for range/range rate comparison")
#                 return {}

#             # Calculate matching percentage
#             matching_percentage = (match_count / total_comparisons) * 100 if total_comparisons > 0 else 0
            
#             # Use the match count for the KPI calculation
#             matched_veh_nums = np.array([match_count])
#             matched_sim_nums = np.array([total_comparisons])
            
#             veh_valid = ~np.isnan(matched_veh_nums)
#             sim_valid = ~np.isnan(matched_sim_nums)
#             both_valid = veh_valid & sim_valid
            
#             if not np.any(both_valid):
#                 logger.warning("All RDD num_detect values are NaN")
#                 return {}

#             num_same_si = int(np.sum(both_valid))
#             num_same_num_dets = int(np.sum(matched_veh_nums == matched_sim_nums))
#             same_num_dets_pct = round((num_same_num_dets / num_same_si * 100), 2) if num_same_si > 0 else 0.0

#             rdd_kpis = {
#                 'num_same_si': num_same_si,
#                 'num_same_num_dets': num_same_num_dets,
#                 'same_num_dets_pct': same_num_dets_pct
#             }

#             return rdd_kpis
            
#         except Exception as e:
#             logger.error(f"Error processing RDD matching: {e}")
#             return {}

        
#     def process_af_detection_matching(self, veh_det_df, sim_det_df):
#         """Process AF detection matching using NumPy arrays"""
#         try:
#             def extract_data(arr_2d):
#                 """Extract scan indices and values from get_value() result."""
#                 if arr_2d.size == 0:
#                     return np.array([]), np.array([])
#                 if arr_2d.ndim == 1:
#                     return np.array([arr_2d[0]]), arr_2d[1:] if len(arr_2d) > 1 else np.array([arr_2d[0]])
#                 # Multiple rows: column 0 is scan_index, column 1+ are values
#                 scan_indices = arr_2d[:, 0].astype(int)
#                 values = arr_2d[:, 1] if arr_2d.shape[1] > 1 else arr_2d[:, 0]
#                 return scan_indices, values
            
#             # Extract num_af_det data
#             veh_scan, veh_num_det = extract_data(veh_det_df.get('num_af_det', np.array([])))
#             sim_scan, sim_num_det = extract_data(sim_det_df.get('num_af_det', np.array([])))
            
#             if veh_scan.size == 0 or sim_scan.size == 0:
#                 logger.warning("No AF detection data available")
#                 return {}
            
#             # Build mapping for matching scan indices
#             sim_map = {}
#             for idx, scan in enumerate(sim_scan):
#                 if scan not in sim_map:
#                     sim_map[scan] = idx
            
#             # Find matching scans
#             matching_scans = np.array([s for s in np.unique(veh_scan) if s in sim_map])
            
#             if matching_scans.size == 0:
#                 logger.warning("No matching scan indices found for AF detection")
#                 return {}
            
#             # Build veh mapping
#             veh_map = {}
#             for idx, scan in enumerate(veh_scan):
#                 if scan not in veh_map:
#                     veh_map[scan] = idx
            
#             matched_veh_indices = np.array([veh_map[s] for s in matching_scans])
#             matched_sim_indices = np.array([sim_map[s] for s in matching_scans])
            
#             # Get matched num_detect values
#             matched_veh_nums = veh_num_det[matched_veh_indices]
#             matched_sim_nums = sim_num_det[matched_sim_indices]
            
#             veh_valid = ~np.isnan(matched_veh_nums)
#             sim_valid = ~np.isnan(matched_sim_nums)
#             both_valid = veh_valid & sim_valid
            
#             if not np.any(both_valid):
#                 logger.warning("All AF num_det values are NaN")
#                 return {}
            
#             # Filter to valid entries
#             matched_veh_nums = matched_veh_nums[both_valid]
#             matched_sim_nums = matched_sim_nums[both_valid]
            
#             num_same_si = int(np.sum(both_valid))
#             num_same_num_dets = int(np.sum(matched_veh_nums == matched_sim_nums))
#             total_detections_veh = int(np.sum(matched_veh_nums))
            
#             # Simplified matching - full implementation would need to match individual detections with thresholds
#             matched_detections = num_same_num_dets
            
#             same_num_dets_pct = round((num_same_num_dets / num_same_si * 100), 2) if num_same_si > 0 else 0.0
#             accuracy = round((matched_detections / total_detections_veh * 100), 2) if total_detections_veh > 0 else 0.0
            
#             af_kpis = {
#                 'num_same_si': num_same_si,
#                 'num_same_num_dets': num_same_num_dets,
#                 'same_num_dets_pct': same_num_dets_pct,
#                 'total_detections_veh': total_detections_veh,
#                 'matched_detections': matched_detections,
#                 'accuracy': accuracy
#             }
            
#             return af_kpis
            
#         except Exception as e:
#             logger.error(f"Error in AF detection matching: {e}")
#             return {}
            
#     def generate_html_report(self):
#         """Generate HTML report for detection KPIs"""
#         try:

#             af_kpis = self.kpi_results.get('af_kpis', {})
#             rdd_kpis = self.kpi_results.get('rdd_kpis', {})
#             af_kpis = self.kpi_results.get('af_kpis', {})
            
#             # Calculate overall accuracy
#             total_detections = af_kpis.get('total_detections_veh', 1)  # Avoid division by zero
#             matched_detections = af_kpis.get('matched_detections', 0)
#             overall_accuracy = (matched_detections / total_detections) * 100 if total_detections > 0 else 0

#             self.html_content = f"""
#             <html>
#             <head>
#                 <title>Detection KPIs and Plots</title>
#                 <script src="https://cdn.plot.ly/plotly-2.27.0.min.js"></script>
#             </head>
#             <body>
#                 <h1>Detection KPIs and Plots</h1>
#                 <small>Sensor ID: {self.sensor_id}</small>
                
#                 <details>
#                     <summary><b><i>Glossary</i></b></summary>
#                     <ol>
#                         <li><b>Definition of match:</b>
#                             A detection is said to match a re-simulated detection if it has the same RDD range and doppler index(within a scan index), 
#                             and the difference(error) in the range, range rate, azimuth and elevation are within the thresholds mentioned below
#                             <ul>
#                                 <li>Range : {self.RAN_THRESHOLD:.6f} m</li>
#                                 <li>Range rate : {self.VEL_THRESHOLD:.6f} m/s</li>
#                                 <li>Azimuth : {self.THETA_THRESHOLD:.6f} radians</li>
#                                 <li>Elevation : {self.PHI_THRESHOLD:.6f} radians</li>
#                             </ul>
#                         <li><b>Accuracy:</b> (Number of matching detections / total number of detections) * 100
#                         <li><b>Saturation:</b> A SI is said to be saturated if the maximum range among all the vehicle detections is >= 135m and re-simulated detections is < 135m
#                     </ol>
#                     <b>Note:</b> The plots are interactive
#                 </details>

#                 <hr width="100%" size="2" color="blue" noshade>

#                 <b>Log:</b> {self.sensor_id}
                
#                 <b>KPI:</b> Accuracy: ({matched_detections}/{total_detections}) --> <b>{overall_accuracy:.2f}%</b>
                
#                 <details>
#                     <summary><i>Details</i></summary>
#                     <b>RDD Streams</b><br>
#                     Number of SI in (vehicle, simulation) : {af_kpis.get('num_same_si', 0)}, {af_kpis.get('num_same_si', 0)}<br>
#                     Number of same SI available in both vehicle and simulation: {af_kpis.get('num_same_si', 0)}<br>
#                     % of SI with same number of RDD1 detections: 
#                     ({rdd_kpis.get('num_same_num_dets', 0)}/{rdd_kpis.get('num_same_si', 1)}) --> {rdd_kpis.get('same_num_dets_pct', 0):.2f}%
#                     <br>

#                     <b>Detection Streams</b><br>
#                     Number of SI in (vehicle, simulation) : {af_kpis.get('num_same_si', 0)}, {af_kpis.get('num_same_si', 0)}<br>
#                     Number of same SI available in both vehicle and simulation: {af_kpis.get('num_same_si', 0)}<br>
#                     % of SI with same number of AF detections: 
#                     ({af_kpis.get('num_same_num_dets', 0)}/{af_kpis.get('num_same_si', 1)}) --> {af_kpis.get('same_num_dets_pct', 0):.2f}%
#                     <br>

#                     <b>Detection Streams Merged</b><br>
#                     Number of SI in (vehicle, simulation) : {af_kpis.get('num_same_si', 0)}, {af_kpis.get('num_same_si', 0)}<br>
#                     Number of same SI available in both vehicle and simulation: {af_kpis.get('num_same_si', 0)}<br>
                    
#                     <b><u>Thresholds Used:</u></b>
#                     <ul>
#                         <li>Range threshold: {self.RAN_THRESHOLD:.6f} m</li>
#                         <li>Velocity threshold: {self.VEL_THRESHOLD:.6f} m/s</li>
#                         <li>Azimuth threshold: {self.THETA_THRESHOLD:.6f} rad</li>
#                         <li>Elevation threshold: {self.PHI_THRESHOLD:.6f} rad</li>
#                     </ul>
#                 </details>
                
#                 <!-- Placeholder for plots -->
#                 <div id="plots-container">
#                     <!-- Plots will be dynamically inserted here -->
#                 </div>
                
#                 <hr width="100%" size="2" color="blue" noshade>
#             </body>
#             </html>
#             """


#             print(f"✅ HTML content generated successfully for {self.sensor_id}")

#         except Exception as e:
#             # ✅ ADD PRINT HERE - When HTML generation fails
#             print(f"❌ Error generating HTML for {self.sensor_id}: {e}")
#             logger.error(f"Error generating HTML report: {e}")
#             self.html_content = f"<p>Error generating detection KPI report: {e}</p>"

#     def get_results(self):
#         """Return KPI results and HTML content"""


#         return {
#             'kpi_results': self.kpi_results,
#             'html_content': self.html_content,
#             'success': bool(self.kpi_results)
#         }

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
        success = processor.process_rdd_matching()

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
