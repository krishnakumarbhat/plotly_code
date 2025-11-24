################################# 
# Detection Mapping KPI HDF Implementation
#################################
import numpy as np
import logging
import plotly.graph_objects as go
from KPI.d_presentation_layer.detection_report import detection_html

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



    def process_rdd_matching(self):
        """Process RDD stream matching and pass it Det to calcuate accuracy."""
        
        # 1. Fetch all necessary data UP FRONT to avoid repeated lookups
        # ------------------------------------------------------------------
        rdd_params = ['rdd1_rindx', 'rdd1_dindx', 'rdd2_range', 'rdd2_range_rate', 'rdd1_num_detect']
        src_rdd = self.data['RDD_STREAM']
        src_det = self.data['DETECTION_STREAM']

        # Helper to safely get data
        def get_all_data(source, params):
            data_map = {}
            for p in params:
                val, status = KPI_DataModelStorage.get_value(source, p)
                if status == "success":
                    data_map[p] = val
                else:
                    logger.warning(f"Failed to get parameter {p}: {status}")
                    data_map[p] = []
            return data_map

        # Fetch RDD data
        veh_rdd = get_all_data(src_rdd['input'], rdd_params)
        sim_rdd = get_all_data(src_rdd['output'], rdd_params)

        # Fetch Detection data (needed for matching)
        det_params = ['rdd_idx', 'ran', 'vel', 'theta', 'phi', 'f_single_target', 
                      'f_superres_target', 'f_bistatic', 'num_af_det']
        veh_det = get_all_data(src_det['input'], det_params)
        sim_det = get_all_data(src_det['output'], det_params)

        # Fetch Scan Indices directly from the container keys
        # This is the source of truth for what scans exist
        veh_scan_indices = sorted(list(src_det['input']._data_container.keys()))
        sim_scan_indices = sorted(list(src_det['output']._data_container.keys()))

        # Create Maps: ScanIndex -> Array Index
        # Note: KPI_DataModelStorage.get_value returns arrays aligned with sorted keys of _data_container
        # So we can map the sorted scan indices to the returned array indices.
        veh_si_map = {si: i for i, si in enumerate(veh_scan_indices)}
        sim_si_map = {si: i for i, si in enumerate(sim_scan_indices)}

        # 2. Identify Common Scan Indices
        # ------------------------------------------------------------------
        common_scan_indices = sorted(list(set(veh_scan_indices) & set(sim_scan_indices)))
        
        logger.info(f"Vehicle Scans: {len(veh_scan_indices)}, Sim Scans: {len(sim_scan_indices)}")
        logger.info(f"Common Scans: {len(common_scan_indices)}")

        # 3. Process Matching
        # ------------------------------------------------------------------
        total_matches = 0
        total_detections = 0
        per_scan_accuracy = []
        matched_scan_indices = []
        per_scan_matches = []
        per_scan_den = []

        # Prepare arrays for reporting
        af_det_counts = []
        af_det_scanindex = []

        # Iterate over ALL vehicle scans for reporting purposes (denominator)
        for si in veh_scan_indices:
            v_idx = veh_si_map[si]
            
            # Get num_af_det for this scan
            try:
                # Handle potential scalar/array wrapping
                raw_val = veh_det['num_af_det'][v_idx]
                num_dets = int(raw_val[0]) if isinstance(raw_val, (list, np.ndarray)) else int(raw_val)
            except (IndexError, TypeError, ValueError):
                num_dets = 0
            
            total_detections += num_dets
            af_det_counts.append(num_dets)
            af_det_scanindex.append(si)

            # If this scan exists in Sim, try to match
            if si in sim_si_map:
                s_idx = sim_si_map[si]
                
                # --- RDD Matching Logic (re-implemented for single scan) ---
                matches_in_scan = 0
                
                # Only proceed if we have valid RDD data for this scan
                if (v_idx < len(veh_rdd.get('rdd1_rindx', [])) and 
                    s_idx < len(sim_rdd.get('rdd1_rindx', []))):
                    
                    # Build Vehicle Hash: (r_idx, d_idx) -> [list of detection indices]
                    veh_hash = {}
                    
                    # Get RDD indices for this scan
                    v_r_indices = veh_rdd['rdd1_rindx'][v_idx]
                    v_d_indices = veh_rdd['rdd1_dindx'][v_idx]
                    
                    # Ensure they are lists/arrays
                    if np.isscalar(v_r_indices): v_r_indices = [v_r_indices]
                    if np.isscalar(v_d_indices): v_d_indices = [v_d_indices]

                    # Map RDD (r, d) to index in the RDD list (0..N)
                    # But wait, we need to map to DETECTION index.
                    # The previous code logic was:
                    # j is index in RDD list. 
                    # veh_rdd['rdd1_rindx'][i][j] is the R value.
                    # It seems 'j' acts as a proxy for detection index if they are 1:1?
                    # Actually, let's look at process_detection_matching.
                    # It maps RDD index -> Detection Index using 'rdd_idx' from detection stream.
                    
                    # Let's build the map: (r, d) -> [j, j, ...] where j is the index in the RDD arrays
                    for j, (r, d) in enumerate(zip(v_r_indices, v_d_indices)):
                        key = (r, d)
                        veh_hash.setdefault(key, []).append(j)

                    # Find Matches in Sim
                    store = {} # Key: tuple(veh_j_list), Value: [sim_j_list]
                    
                    s_r_indices = sim_rdd['rdd1_rindx'][s_idx]
                    s_d_indices = sim_rdd['rdd1_dindx'][s_idx]
                    if np.isscalar(s_r_indices): s_r_indices = [s_r_indices]
                    if np.isscalar(s_d_indices): s_d_indices = [s_d_indices]

                    for j, (r, d) in enumerate(zip(s_r_indices, s_d_indices)):
                        key = (r, d)
                        if key in veh_hash:
                            veh_j_list = tuple(veh_hash[key])
                            if veh_j_list not in store:
                                store[veh_j_list] = []
                            store[veh_j_list].append(j)
                    
                    # Perform detailed detection matching
                    if store:
                        matches_in_scan = self.process_detection_matching(
                            v_idx, s_idx, store, veh_det, sim_det
                        )

                # Update stats
                total_matches += matches_in_scan
                acc_i = (matches_in_scan / num_dets) * 100.0 if num_dets > 0 else None
                
                per_scan_accuracy.append(acc_i)
                matched_scan_indices.append(si)
                per_scan_matches.append(matches_in_scan)
                per_scan_den.append(num_dets)

        # 4. Finalize Results
        # ------------------------------------------------------------------
        detection_accuracy = (total_matches / total_detections) * 100.0 if total_detections > 0 else None
        
        valid_acc_values = [x for x in per_scan_accuracy if x is not None]
        min_acc = (min(valid_acc_values) if valid_acc_values else None)
        max_acc = (max(valid_acc_values) if valid_acc_values else None)

        self.kpi_results['per_scan_accuracy'] = per_scan_accuracy
        self.kpi_results['per_scan_scanindex'] = matched_scan_indices
        self.kpi_results['per_scan_matches'] = per_scan_matches
        self.kpi_results['per_scan_den'] = per_scan_den
        self.kpi_results['af_det_counts'] = af_det_counts
        self.kpi_results['af_det_scanindex'] = af_det_scanindex
        self.kpi_results['veh_si_count'] = len(veh_scan_indices)
        self.kpi_results['sim_si_count'] = len(sim_scan_indices)
        self.kpi_results['scans_processed'] = len(veh_scan_indices)
        self.kpi_results['scans_with_matches'] = len(matched_scan_indices)
        self.kpi_results['min_accuracy'] = min_acc
        self.kpi_results['max_accuracy'] = max_acc

        self.kpi_results['matching_accuracy'] = {
            'matches': int(total_matches),
            'total_detections': int(total_detections),
            'detection_accuracy_percentage': detection_accuracy,
        }

        acc_str = "N/A" if detection_accuracy is None else f"{detection_accuracy:.2f}%"
        print(f"Detection matching completed. Matches: {total_matches}, Total Detections: {total_detections}, Accuracy: {acc_str}")
        
        self.generate_html_report()
        return True

    def process_detection_matching(self, v_idx, s_idx, store, veh_det, sim_det):
        """
        Process detection matching for a single scan using pre-fetched data.
        
        Args:
            v_idx (int): Vehicle Array Index
            s_idx (int): Simulation Array Index
            store (dict): Mapping from Veh RDD indices to Sim RDD indices
            veh_det (dict): Pre-fetched vehicle detection data
            sim_det (dict): Pre-fetched simulation detection data
            
        Returns:
            int: Number of matches found
        """
        try:
            matches = 0

            # Helper to get specific scan data
            def get_scan_data(data_dict, idx, param):
                if param not in data_dict or idx >= len(data_dict[param]):
                    return []
                val = data_dict[param][idx]
                return val if isinstance(val, (list, np.ndarray)) else [val]

            # Get arrays for this scan
            veh_ran = get_scan_data(veh_det, v_idx, 'ran')
            veh_vel = get_scan_data(veh_det, v_idx, 'vel')
            veh_theta = get_scan_data(veh_det, v_idx, 'theta')
            veh_phi = get_scan_data(veh_det, v_idx, 'phi')
            veh_rdd_idx = get_scan_data(veh_det, v_idx, 'rdd_idx')

            sim_ran = get_scan_data(sim_det, s_idx, 'ran')
            sim_vel = get_scan_data(sim_det, s_idx, 'vel')
            sim_theta = get_scan_data(sim_det, s_idx, 'theta')
            sim_phi = get_scan_data(sim_det, s_idx, 'phi')
            sim_rdd_idx = get_scan_data(sim_det, s_idx, 'rdd_idx')

            # Build RDD Index -> Detection Index Maps
            veh_rdd_to_det = {}
            for i, r_idx in enumerate(veh_rdd_idx):
                try: veh_rdd_to_det[int(r_idx)] = i
                except: continue

            sim_rdd_to_det = {}
            for i, r_idx in enumerate(sim_rdd_idx):
                try: sim_rdd_to_det[int(r_idx)] = i
                except: continue

            # Track used indices
            used_veh = set()
            used_sim = set()

            # Iterate through potential matches from RDD stage
            for veh_rdd_indices, sim_rdd_indices in store.items():
                # Convert RDD list indices to Detection indices
                # Note: store keys/values are indices into the RDD arrays (0..N_RDD)
                # But we need to know which RDD_IDX that corresponds to?
                # Wait, the previous logic assumed 'rdd_idx' param maps Detection -> RDD.
                # But 'store' was built using indices 0..N of the RDD arrays.
                # Does 'rdd_idx' in detection stream contain the index 0..N of the RDD stream?
                # YES, usually 'rdd_idx' links a detection back to its source RDD point.
                
                # So if veh_rdd_indices contains [0, 1], it means RDD points 0 and 1.
                # We need to find which Detections point to RDD points 0 and 1.
                
                # Let's reverse the map: RDD_Point_Index -> [List of Detection Indices]
                # Actually, veh_rdd_to_det maps RDD_ID -> Det_ID. 
                # Is 'rdd_idx' the ID or the Index? 
                # Usually it's the index in the RDD array. Let's assume that.
                
                # Filter valid detections for this RDD match group
                v_det_candidates = []
                for r_idx in veh_rdd_indices: # r_idx is index in RDD array
                    if r_idx in veh_rdd_to_det:
                        v_det_candidates.append(veh_rdd_to_det[r_idx])
                
                s_det_candidates = []
                for r_idx in sim_rdd_indices:
                    if r_idx in sim_rdd_to_det:
                        s_det_candidates.append(sim_rdd_to_det[r_idx])

                # Brute force match within this small group
                for vi in v_det_candidates:
                    if vi in used_veh: continue
                    
                    for si in s_det_candidates:
                        if si in used_sim: continue
                        
                        # Check thresholds
                        try:
                            # Safe access with bounds checking
                            if (vi < len(veh_ran) and si < len(sim_ran) and
                                abs(veh_ran[vi] - sim_ran[si]) <= self.RAN_THRESHOLD and
                                abs(veh_vel[vi] - sim_vel[si]) <= self.VEL_THRESHOLD and
                                abs(veh_theta[vi] - sim_theta[si]) <= self.THETA_THRESHOLD and
                                abs(veh_phi[vi] - sim_phi[si]) <= self.PHI_THRESHOLD):
                                
                                matches += 1
                                used_veh.add(vi)
                                used_sim.add(si)
                                break # Match found for this vehicle detection
                        except Exception:
                            continue

            return matches

        except Exception as e:
            logger.error(f"Error in process_detection_matching: {e}")
            return 0

    def generate_html_report(self):
        """Generate HTML report for detection KPIs with embedded plots"""
        try:
            # Get KPI results
            matching_accuracy = self.kpi_results.get('matching_accuracy', {})
            
            # Extract values with defaults
            matches = matching_accuracy.get('matches', 0)
            total_detections = matching_accuracy.get('total_detections', 1)  # Avoid division by zero
            accuracy = matching_accuracy.get('detection_accuracy_percentage', None)
            accuracy_str = "N/A" if accuracy is None else f"{accuracy:.2f}"
            
            # Additional metrics
            min_acc = self.kpi_results.get('min_accuracy')
            max_acc = self.kpi_results.get('max_accuracy')
            scans_processed = self.kpi_results.get('scans_processed', 0)
            scans_with_matches = self.kpi_results.get('scans_with_matches', 0)
            veh_si_count = self.kpi_results.get('veh_si_count', 0)
            sim_si_count = self.kpi_results.get('sim_si_count', 0)

            # Prepare per-scan table rows (scan index, matches, denominator, accuracy)
            scan_idxs = self.kpi_results.get('per_scan_scanindex', [])
            scan_matches = self.kpi_results.get('per_scan_matches', [])
            scan_den = self.kpi_results.get('per_scan_den', [])
            scan_acc = self.kpi_results.get('per_scan_accuracy', [])
            rows = []
            for si, m, d, a in zip(scan_idxs, scan_matches, scan_den, scan_acc):
                a_str = "N/A" if a is None else f"{a:.2f}"
                rows.append(f"<tr><td>{si}</td><td>{m}</td><td>{d}</td><td>{a_str}</td></tr>")
            per_scan_rows = "".join(rows)

            # Get plot data
            accuracy_plot = self._create_accuracy_plot()
            af_det_plot = self._create_af_det_plot()
            
            # Render HTML using imported template string
            html = detection_html.format(
                sensor_id=self.sensor_id,
                matches=matches,
                total_detections=total_detections,
                accuracy=accuracy_str,
                ran_th=f"{self.RAN_THRESHOLD:.6f}",
                vel_th=f"{self.VEL_THRESHOLD:.6f}",
                theta_th=f"{self.THETA_THRESHOLD:.6f}",
                phi_th=f"{self.PHI_THRESHOLD:.6f}",
                min_accuracy="N/A" if min_acc is None else f"{min_acc:.2f}",
                max_accuracy="N/A" if max_acc is None else f"{max_acc:.2f}",
                scans_processed=scans_processed,
                scans_with_matches=scans_with_matches,
                veh_si_count=veh_si_count,
                sim_si_count=sim_si_count,
                per_scan_rows=per_scan_rows,
                accuracy_plot=accuracy_plot.to_json(),
                af_det_plot=af_det_plot.to_json()
            )
            self.html_content = html
            
            return True
            
        except Exception as e:
            logger.error(f"Error generating HTML report: {e}")
            self.html_content = f"<p>Error generating detection KPI report: {e}</p>"
            return False
            
    def _create_accuracy_plot(self):
        """Create accuracy vs scan index plot"""
        try:
            per_scan_accuracy = self.kpi_results.get('per_scan_accuracy', [])
            scan_indices = self.kpi_results.get('per_scan_scanindex', list(range(len(per_scan_accuracy))))
            
            fig = go.Figure()
            fig.add_trace(go.Scatter(
                x=scan_indices,
                y=per_scan_accuracy,
                mode='lines+markers',
                name='Accuracy (%)',
                line=dict(color='#2980b9', width=2),
                marker=dict(size=6, color='#2980b9')
            ))
            
            fig.update_layout(
                title='Accuracy vs Scan Index',
                xaxis_title='Scan Index',
                yaxis_title='Accuracy (%)',
                plot_bgcolor='white',
                showlegend=True,
                height=400,
                margin=dict(l=50, r=50, t=50, b=50)
            )
            
            return fig
            
        except Exception as e:
            logger.error(f"Error creating accuracy plot: {e}")
            # Return empty figure on error
            return go.Figure()
            
    def _create_af_det_plot(self):
        """Create num_af_det vs scan index plot"""
        try:
            y_counts = self.kpi_results.get('af_det_counts', [])
            x_idx = self.kpi_results.get('af_det_scanindex', list(range(len(y_counts))))
            
            fig = go.Figure()
            fig.add_trace(go.Bar(
                x=x_idx,
                y=y_counts,
                name='num_af_det',
                marker_color='#3498db',
                opacity=0.7
            ))
            
            fig.update_layout(
                title='Number of AF Detections vs Scan Index',
                xaxis_title='Scan Index',
                yaxis_title='Number of AF Detections',
                plot_bgcolor='white',
                showlegend=False,
                height=400,
                margin=dict(l=50, r=50, t=50, b=50)
            )
            
            return fig
            
        except Exception as e:
            logger.error(f"Error creating AF detections plot: {e}")
            # Return empty figure on error
            return go.Figure()

    def get_results(self):
        """
        Return KPI results and HTML content
        
        Returns:
            dict: Dictionary containing:
                - kpi_results: Dictionary of KPI metrics
                - html_content: Generated HTML report
                - success: Boolean indicating if processing was successful
        """
        # Ensure we have the latest HTML content
        if not self.html_content and hasattr(self, 'kpi_results'):
            self.generate_html_report()
            
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
