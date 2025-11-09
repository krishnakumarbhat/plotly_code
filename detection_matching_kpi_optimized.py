################################# 
# Detection Mapping KPI HDF Implementation - OPTIMIZED
#################################
import numpy as np
import time
import logging

from KPI.b_data_storage.kpi_data_model_storage import KPI_DataModelStorage
from KPI.b_data_storage.kpi_config_storage import KPI_VALIDATION_RULES

logger = logging.getLogger(__name__)

class DetectionMappingKPIHDF:
    """Detection KPI analysis using HDF data from self.input_data and self.output_data"""
    
    # Required parameters for RDD and detection streams
    RDD_PARAMS = ['rdd1_rindx', 'rdd1_dindx', 'rdd2_range', 'rdd2_range_rate', 'rdd1_num_detect']
    DET_PARAMS = ['ran', 'vel', 'theta', 'phi']
    
    def __init__(self, data: KPI_DataModelStorage, sensor_id: str):
        self.data = data
        self.sensor_id = sensor_id
        
        # Load thresholds in bulk
        thresholds = KPI_VALIDATION_RULES["detection_thresholds"]
        self.RAN_THRESHOLD = thresholds["range_threshold"]
        self.VEL_THRESHOLD = thresholds["velocity_threshold"]
        self.THETA_THRESHOLD = thresholds["theta_threshold"]
        self.PHI_THRESHOLD = thresholds["phi_threshold"]
        
        # Set sensor-specific limits
        is_front = '_FC_' in sensor_id.upper() or 'FRONT' in sensor_id.upper()
        self.RANGE_SATURATION_THRESHOLD = (
            thresholds["range_saturation_threshold_front"] if is_front 
            else thresholds["range_saturation_threshold_corner"]
        )
        
        self.html_content = ""
        self.kpi_results = {}

    def process_rdd_matching(self):
        """Process RDD stream matching and compute detection KPIs."""
        try:
            rdd_params = ['rdd1_rindx', 'rdd1_dindx', 'rdd2_range', 'rdd2_range_rate', 'rdd1_num_detect']
            rdd_stream = self.data['RDD_STREAM']

            veh_rdd = {}
            sim_rdd = {}
            for param in rdd_params:
                veh_res, veh_status = KPI_DataModelStorage.get_value(rdd_stream['input'], param)
                sim_res, sim_status = KPI_DataModelStorage.get_value(rdd_stream['output'], param)
                veh_rdd[param] = veh_res if veh_status == "success" else []
                sim_rdd[param] = sim_res if sim_status == "success" else []

            det_stream = self.data['DETECTION_STREAM']
            veh_num_af_det, _ = KPI_DataModelStorage.get_value(det_stream['input'], 'num_af_det')
            sim_num_af_det, _ = KPI_DataModelStorage.get_value(det_stream['output'], 'num_af_det')

            total_matches = 0
            total_detections = 0
            veh_af_counts, sim_af_counts = [], []
            veh_rdd_counts, sim_rdd_counts = [], []

            num_scans = max(
                len(veh_num_af_det),
                len(sim_num_af_det),
                len(veh_rdd.get('rdd1_num_detect', [])),
                len(sim_rdd.get('rdd1_num_detect', []))
            )

            for scan_idx in range(num_scans):
                def extract_count(source, idx):
                    if idx >= len(source):
                        return 0
                    raw = source[idx]
                    if isinstance(raw, (list, np.ndarray)):
                        if len(raw) == 0:
                            return 0
                        try:
                            return int(raw[0])
                        except (TypeError, ValueError, IndexError):
                            return 0
                    try:
                        return int(raw)
                    except (TypeError, ValueError):
                        return 0

                veh_det_count = extract_count(veh_num_af_det, scan_idx)
                sim_det_count = extract_count(sim_num_af_det, scan_idx)
                veh_rdd_count = extract_count(veh_rdd.get('rdd1_num_detect', []), scan_idx)
                sim_rdd_count = extract_count(sim_rdd.get('rdd1_num_detect', []), scan_idx)

                veh_af_counts.append(veh_det_count)
                sim_af_counts.append(sim_det_count)
                veh_rdd_counts.append(veh_rdd_count)
                sim_rdd_counts.append(sim_rdd_count)

                total_detections += veh_det_count

                if veh_det_count == 0:
                    continue

                veh_hash = {}
                store = {}

                for det_idx in range(veh_det_count):
                    try:
                        key = (
                            veh_rdd['rdd1_rindx'][scan_idx][det_idx],
                            veh_rdd['rdd1_dindx'][scan_idx][det_idx]
                        )
                        veh_hash.setdefault(key, []).append(det_idx)
                    except (IndexError, KeyError, TypeError):
                        continue

                for det_idx in range(sim_det_count):
                    try:
                        key = (
                            sim_rdd['rdd1_rindx'][scan_idx][det_idx],
                            sim_rdd['rdd1_dindx'][scan_idx][det_idx]
                        )
                        if key in veh_hash:
                            veh_indices = tuple(veh_hash[key])
                            store.setdefault(veh_indices, []).append(det_idx)
                    except (IndexError, KeyError, TypeError):
                        continue

                if store:
                    matches = self.process_detection_matching(scan_idx, store)
                    total_matches += min(matches, veh_det_count)

            total_matches = min(total_matches, total_detections)
            accuracy = (total_matches / total_detections * 100) if total_detections > 0 else 0.0

            self.kpi_results['matching_accuracy'] = {
                'matches': int(total_matches),
                'total_detections': int(total_detections),
                'accuracy_percentage': float(accuracy)
            }

            same_len_af = min(len(veh_af_counts), len(sim_af_counts))
            same_af = sum(
                1 for idx in range(same_len_af)
                if veh_af_counts[idx] == sim_af_counts[idx] and veh_af_counts[idx] > 0
            )
            total_af = sum(veh_af_counts)
            self.kpi_results['af_kpis'] = {
                'num_veh_si': sum(1 for c in veh_af_counts if c > 0),
                'num_sim_si': sum(1 for c in sim_af_counts if c > 0),
                'num_same_si': same_len_af,
                'num_same_num_dets': same_af,
                'same_num_dets_pct': (same_af / same_len_af * 100) if same_len_af > 0 else 0.0,
                'total_detections_veh': total_af,
                'matched_detections': int(total_matches),
                'accuracy': (total_matches / total_af * 100) if total_af > 0 else 0.0
            }

            same_len_rdd = min(len(veh_rdd_counts), len(sim_rdd_counts))
            same_rdd = sum(
                1 for idx in range(same_len_rdd)
                if veh_rdd_counts[idx] == sim_rdd_counts[idx] and veh_rdd_counts[idx] > 0
            )
            total_rdd = sum(veh_rdd_counts)
            self.kpi_results['rdd_kpis'] = {
                'num_veh_si': sum(1 for c in veh_rdd_counts if c > 0),
                'num_sim_si': sum(1 for c in sim_rdd_counts if c > 0),
                'num_same_si': same_len_rdd,
                'num_same_num_dets': same_rdd,
                'same_num_dets_pct': (same_rdd / same_len_rdd * 100) if same_len_rdd > 0 else 0.0,
                'total_detections_veh': total_rdd,
                'matched_detections': int(total_matches),
                'accuracy': (total_matches / total_rdd * 100) if total_rdd > 0 else 0.0
            }

            logger.info(
                f"Detection matching completed. Matches: {total_matches}/{total_detections} = {accuracy:.2f}%"
            )

            self.generate_html_report()
            return True

        except Exception as exc:
            logger.error(f"Error in process_rdd_matching: {exc}")
            return False

    def generate_html_report(self):
        """Generate HTML report for detection KPIs"""
        try:
            af_kpis = self.kpi_results.get('af_kpis', {})
            rdd_kpis = self.kpi_results.get('rdd_kpis', {})
            
            overall_accuracy = af_kpis.get('accuracy', 0.0)
            matched_detections = af_kpis.get('matched_detections', 0)
            total_detections = af_kpis.get('total_detections_veh', 1)
            
            rdd_accuracy = rdd_kpis.get('accuracy', 0.0)
            rdd_matched = rdd_kpis.get('matched_detections', 0)
            rdd_total = rdd_kpis.get('total_detections_veh', 1)

            self.html_content = f"""
            <html>
            <head>
                <title>Detection KPIs - {self.sensor_id}</title>
                <script src="https://cdn.plot.ly/plotly-2.27.0.min.js"></script>
                <style>
                    body {{ font-family: Arial, sans-serif; margin: 20px; }}
                    .kpi-box {{ 
                        border: 1px solid #ddd; padding: 15px; margin: 10px 0; 
                        border-radius: 5px; background-color: #f9f9f9;
                    }}
                    .kpi-header {{ font-weight: bold; color: #2c3e50; margin-bottom: 10px; font-size: 1.1em; }}
                    .kpi-value {{ font-weight: bold; color: #2980b9; }}
                    .thresholds {{ background-color: #f0f7ff; padding: 10px; border-left: 4px solid #3498db; margin: 10px 0; }}
                </style>
            </head>
            <body>
                <h1>Detection KPIs - {self.sensor_id}</h1>
                
                <details>
                    <summary><b><i>Glossary</i></b></summary>
                    <ol>
                        <li><b>Definition of match:</b>
                            A detection matches a re-simulated detection if it has the same RDD range and doppler index (within a scan), 
                            and the difference in range, range rate, azimuth and elevation are within thresholds:
                            <ul>
                                <li>Range: {self.RAN_THRESHOLD:.6f} m</li>
                                <li>Range rate: {self.VEL_THRESHOLD:.6f} m/s</li>
                                <li>Azimuth: {self.THETA_THRESHOLD:.6f} radians</li>
                                <li>Elevation: {self.PHI_THRESHOLD:.6f} radians</li>
                            </ul>
                        <li><b>Accuracy:</b> (Matching detections / Total detections) × 100
                        <li><b>Saturation:</b> SI is saturated if max vehicle range ≥ {self.RANGE_SATURATION_THRESHOLD}m and sim range < {self.RANGE_SATURATION_THRESHOLD}m
                    </ol>
                    <b>Note:</b> Plots are interactive
                </details>

                <hr width="100%" size="2" color="#3498db" noshade>

                <div class="kpi-box">
                    <div class="kpi-header">Overall Detection Matching Accuracy</div>
                    <div>AF Detection Accuracy: <span class="kpi-value">{overall_accuracy:.2f}%</span> ({matched_detections}/{total_detections} matched)</div>
                    <div>RDD Detection Accuracy: <span class="kpi-value">{rdd_accuracy:.2f}%</span> ({rdd_matched}/{rdd_total} matched)</div>
                </div>

                <div class="kpi-box">
                    <div class="kpi-header">AF Detection Stream KPIs</div>
                    <table style="width:100%; border-collapse: collapse;">
                        <tr><td>Vehicle SI:</td><td><b>{af_kpis.get('num_veh_si', 0)}</b></td></tr>
                        <tr><td>Simulation SI:</td><td><b>{af_kpis.get('num_sim_si', 0)}</b></td></tr>
                        <tr><td>Common SI:</td><td><b>{af_kpis.get('num_same_si', 0)}</b></td></tr>
                        <tr><td>SI with same detection count:</td><td><b>{af_kpis.get('num_same_num_dets', 0)}</b> ({af_kpis.get('same_num_dets_pct', 0):.2f}%)</td></tr>
                        <tr><td>Total vehicle detections:</td><td><b>{af_kpis.get('total_detections_veh', 0)}</b></td></tr>
                        <tr><td>Matched detections:</td><td><b>{af_kpis.get('matched_detections', 0)}</b></td></tr>
                    </table>
                </div>

                <div class="kpi-box">
                    <div class="kpi-header">RDD Stream KPIs</div>
                    <table style="width:100%; border-collapse: collapse;">
                        <tr><td>Vehicle SI:</td><td><b>{rdd_kpis.get('num_veh_si', 0)}</b></td></tr>
                        <tr><td>Simulation SI:</td><td><b>{rdd_kpis.get('num_sim_si', 0)}</b></td></tr>
                        <tr><td>Common SI:</td><td><b>{rdd_kpis.get('num_same_si', 0)}</b></td></tr>
                        <tr><td>SI with same detection count:</td><td><b>{rdd_kpis.get('num_same_num_dets', 0)}</b> ({rdd_kpis.get('same_num_dets_pct', 0):.2f}%)</td></tr>
                        <tr><td>Total vehicle detections:</td><td><b>{rdd_kpis.get('total_detections_veh', 0)}</b></td></tr>
                        <tr><td>Matched detections:</td><td><b>{rdd_kpis.get('matched_detections', 0)}</b></td></tr>
                    </table>
                </div>

                <div class="thresholds">
                    <b>Thresholds Used:</b>
                    <ul>
                        <li>Range: {self.RAN_THRESHOLD:.6f} m</li>
                        <li>Velocity: {self.VEL_THRESHOLD:.6f} m/s</li>
                        <li>Azimuth: {self.THETA_THRESHOLD:.6f} rad</li>
                        <li>Elevation: {self.PHI_THRESHOLD:.6f} rad</li>
                        <li>Range saturation: {self.RANGE_SATURATION_THRESHOLD:.1f} m</li>
                    </ul>
                </div>
                
                <div id="plots-container"></div>
                
                <hr width="100%" size="2" color="#3498db" noshade>
                <p style="text-align: right; color: #7f8c8d; font-size: 0.9em;">
                    Generated on {time.strftime('%Y-%m-%d %H:%M:%S')}
                </p>
            </body>
            </html>
            """
            
            print(f"✅ HTML generated for {self.sensor_id}")
            
        except Exception as e:
            print(f"❌ Error generating HTML for {self.sensor_id}: {e}")
            logger.error(f"Error generating HTML report: {e}")
            self.html_content = f"<p>Error generating detection KPI report: {e}</p>"

    def get_results(self) -> Dict:
        """Return KPI results and HTML content"""
        if not self.html_content and self.kpi_results:
            self.generate_html_report()
            
        return {
            'kpi_results': self.kpi_results,
            'html_content': self.html_content,
            'success': bool(self.kpi_results)
        }


def process_detection_kpi(data: KPI_DataModelStorage, sensor_id: str) -> Dict:
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
        
        return processor.get_results() if success else {
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
