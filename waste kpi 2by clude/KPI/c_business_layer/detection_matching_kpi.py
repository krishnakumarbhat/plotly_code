################################# 
# Detection Mapping KPI HDF Implementation
#################################
import numpy as np
import logging
import plotly.graph_objects as go
from collections import Counter
from KPI.d_presentation_layer.detection_report import detection_html

from KPI.b_data_storage.kpi_data_model_storage import KPI_DataModelStorage
from KPI.b_data_storage.kpi_config_storage import KPI_VALIDATION_RULES

logger = logging.getLogger(__name__)
logger.setLevel(logging.NOTSET)
logger.propagate = True

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
        rdd_params = ['rdd1_rindx', 'rdd1_dindx', 'rdd2_range', 'rdd2_range_rate', 'rdd1_num_detect']
        src = self.data['RDD_STREAM']

        # Initialize dictionaries to store RDD data
        veh_rdd = {}
        sim_rdd = {}

        # Get RDD parameters
        for param in rdd_params:
            veh_result, veh_status = KPI_DataModelStorage.get_value(src['input'], param)
            sim_result, sim_status = KPI_DataModelStorage.get_value(src['output'], param)

            if veh_status == "success" and sim_status == "success":
                veh_rdd[param] = veh_result
                sim_rdd[param] = sim_result
            else:
                logger.warning(f"Failed to get RDD parameter {param}: veh={veh_status}, sim={sim_status}. Check HDF.")
                veh_rdd[param] = np.array([])
                sim_rdd[param] = np.array([])

        # Get AF detection counts
        veh_rdd['num_af_det'], _ = KPI_DataModelStorage.get_value(self.data['DETECTION_STREAM']['input'], 'num_af_det')
        sim_rdd['num_af_det'], _ = KPI_DataModelStorage.get_value(self.data['DETECTION_STREAM']['output'], 'num_af_det')

        # Optional debug: rdd_idx lists per AF detection (used to diagnose matching quality)
        veh_det_rdd_idx, _ = KPI_DataModelStorage.get_value(self.data['DETECTION_STREAM']['input'], 'rdd_idx')
        sim_det_rdd_idx, _ = KPI_DataModelStorage.get_value(self.data['DETECTION_STREAM']['output'], 'rdd_idx')

        
        # Initialize variables for tracking matches and total detections
        # NOTE: "Matching Accuracy" follows the original CSV script logic:
        # - Match by (rdd1_rindx, rdd1_dindx) pairs built via rdd_idx lookup
        # - Validate range/velocity/theta/phi within thresholds
        # - Denominator = num_af_det_veh (raw vehicle detection count)
        # - One-to-one matching (remove matched sim pairs)
        total_matches = 0
        total_detections = 0
        per_scan_accuracy = []
        matched_scan_indices = []
        per_scan_matches = []
        per_scan_den = []

        # Prefer scan_index from DETECTION_STREAM (if present), otherwise fall back to 0..N-1.
        scan_indices, scan_status = KPI_DataModelStorage.get_value(self.data['DETECTION_STREAM']['input'], 'scan_index')
        if scan_status != "success":
            scan_indices = list(range(len(veh_rdd.get('num_af_det', []))))

        # Get detection parameters for value matching
        det_params = ['ran', 'vel', 'theta', 'phi']
        veh_det = {}
        sim_det = {}
        det_src = self.data['DETECTION_STREAM']
        for param in det_params:
            veh_result, veh_status = KPI_DataModelStorage.get_value(det_src['input'], param)
            sim_result, sim_status = KPI_DataModelStorage.get_value(det_src['output'], param)
            veh_det[param] = veh_result if veh_status == "success" else []
            sim_det[param] = sim_result if sim_status == "success" else []

        # Log number of SIs in vehicle and simulation
        veh_si_count = len(veh_rdd.get('num_af_det', []))
        sim_si_count = len(sim_rdd.get('num_af_det', []))
        logger.info("Number of SI in (vehicle, simulation): (%s, %s)", veh_si_count, sim_si_count)

        def _as_int_count(raw):
            try:
                return int(raw[0]) if isinstance(raw, (list, np.ndarray)) else int(raw)
            except Exception:
                return 0

        def _safe_get(arr, idx, default=0):
            try:
                if arr is None or idx >= len(arr):
                    return default
                val = arr[idx]
                if isinstance(val, float) and np.isnan(val):
                    return default
                return val
            except Exception:
                return default

        scans_processed = int(len(veh_rdd.get('num_af_det', [])))

        for i, arr in enumerate(veh_rdd.get('num_af_det', [])):
            num_af_det_veh = _as_int_count(arr)
            if num_af_det_veh <= 0:
                per_scan_accuracy.append(None)
                per_scan_matches.append(0)
                per_scan_den.append(0)
                matched_scan_indices.append(i)
                continue

            num_af_det_sim = 0
            if i < len(sim_rdd.get('num_af_det', [])):
                num_af_det_sim = _as_int_count(sim_rdd['num_af_det'][i])

            # Build (rdd1_rindx, rdd1_dindx) pairs for vehicle detections
            # by looking up rdd_idx in the RDD stream
            veh_pairs = []  # list of ((rindx, dindx), det_idx)
            veh_rdd_idx_row = veh_det_rdd_idx[i] if i < len(veh_det_rdd_idx) else []
            veh_rindx_row = veh_rdd.get('rdd1_rindx', [])[i] if i < len(veh_rdd.get('rdd1_rindx', [])) else []
            veh_dindx_row = veh_rdd.get('rdd1_dindx', [])[i] if i < len(veh_rdd.get('rdd1_dindx', [])) else []
            
            for j in range(num_af_det_veh):
                try:
                    rdd_idx = int(_safe_get(veh_rdd_idx_row, j, -1))
                    if rdd_idx < 0:
                        continue
                    rindx = _safe_get(veh_rindx_row, rdd_idx, None)
                    dindx = _safe_get(veh_dindx_row, rdd_idx, None)
                    if rindx is not None and dindx is not None:
                        veh_pairs.append(((int(rindx), int(dindx)), j))
                except Exception:
                    continue

            # Build (rdd1_rindx, rdd1_dindx) pairs for sim detections
            sim_pairs = []  # list of ((rindx, dindx), det_idx)
            sim_rdd_idx_row = sim_det_rdd_idx[i] if i < len(sim_det_rdd_idx) else []
            sim_rindx_row = sim_rdd.get('rdd1_rindx', [])[i] if i < len(sim_rdd.get('rdd1_rindx', [])) else []
            sim_dindx_row = sim_rdd.get('rdd1_dindx', [])[i] if i < len(sim_rdd.get('rdd1_dindx', [])) else []
            
            for j in range(num_af_det_sim):
                try:
                    rdd_idx = int(_safe_get(sim_rdd_idx_row, j, -1))
                    if rdd_idx < 0:
                        continue
                    rindx = _safe_get(sim_rindx_row, rdd_idx, None)
                    dindx = _safe_get(sim_dindx_row, rdd_idx, None)
                    if rindx is not None and dindx is not None:
                        sim_pairs.append(((int(rindx), int(dindx)), j))
                except Exception:
                    continue

            # Build sim lookup: (rindx, dindx) -> list of (det_idx)
            sim_pair_dict = {}
            for pair, det_idx in sim_pairs:
                sim_pair_dict.setdefault(pair, []).append(det_idx)

            # Get detection value arrays for this scan
            veh_ran_row = veh_det.get('ran', [])[i] if i < len(veh_det.get('ran', [])) else []
            veh_vel_row = veh_det.get('vel', [])[i] if i < len(veh_det.get('vel', [])) else []
            veh_theta_row = veh_det.get('theta', [])[i] if i < len(veh_det.get('theta', [])) else []
            veh_phi_row = veh_det.get('phi', [])[i] if i < len(veh_det.get('phi', [])) else []
            sim_ran_row = sim_det.get('ran', [])[i] if i < len(sim_det.get('ran', [])) else []
            sim_vel_row = sim_det.get('vel', [])[i] if i < len(sim_det.get('vel', [])) else []
            sim_theta_row = sim_det.get('theta', [])[i] if i < len(sim_det.get('theta', [])) else []
            sim_phi_row = sim_det.get('phi', [])[i] if i < len(sim_det.get('phi', [])) else []

            # Match: for each veh pair, find a sim pair with same (rindx, dindx)
            # and validate values within thresholds (one-to-one, remove matched sim)
            matches = 0
            used_sim_det_indices = set()
            
            for (veh_pair, veh_det_idx) in veh_pairs:
                if veh_pair not in sim_pair_dict:
                    continue
                for sim_det_idx in sim_pair_dict[veh_pair]:
                    if sim_det_idx in used_sim_det_indices:
                        continue
                    # Validate values within thresholds
                    try:
                        veh_ran = _safe_get(veh_ran_row, veh_det_idx, 0)
                        sim_ran = _safe_get(sim_ran_row, sim_det_idx, 0)
                        veh_vel = _safe_get(veh_vel_row, veh_det_idx, 0)
                        sim_vel = _safe_get(sim_vel_row, sim_det_idx, 0)
                        veh_theta = _safe_get(veh_theta_row, veh_det_idx, 0)
                        sim_theta = _safe_get(sim_theta_row, sim_det_idx, 0)
                        veh_phi = _safe_get(veh_phi_row, veh_det_idx, 0)
                        sim_phi = _safe_get(sim_phi_row, sim_det_idx, 0)
                        
                        if (abs(veh_ran - sim_ran) <= self.RAN_THRESHOLD and
                            abs(veh_vel - sim_vel) <= self.VEL_THRESHOLD and
                            abs(veh_theta - sim_theta) <= self.THETA_THRESHOLD and
                            abs(veh_phi - sim_phi) <= self.PHI_THRESHOLD):
                            matches += 1
                            used_sim_det_indices.add(sim_det_idx)
                            break  # one-to-one: move to next veh detection
                    except Exception:
                        continue

            # Denominator is num_af_det_veh (raw vehicle detection count)
            total_detections += num_af_det_veh
            total_matches += matches

            # Debug for first few scans
            if i < 5:
                logger.info(
                    "Scan %s: matches=%s / veh_dets=%s sim_dets=%s veh_pairs=%s sim_pairs=%s",
                    i, matches, num_af_det_veh, num_af_det_sim, len(veh_pairs), len(sim_pairs)
                )

            # Per-scan accuracy: matches / num_af_det_veh
            acc_i = (matches / num_af_det_veh) * 100.0 if num_af_det_veh > 0 else None
            per_scan_accuracy.append(acc_i)
            matched_scan_indices.append(i)
            per_scan_matches.append(int(matches))
            per_scan_den.append(int(num_af_det_veh))

        # Calculate detection matching accuracy
        detection_accuracy = (total_matches / total_detections) * 100.0 if total_detections > 0 else None

        # Get RDD KPI results if available (not currently used, kept minimal)

        # Prepare and store per-scan arrays and summary metrics
        valid_scan_indices = []
        for i in matched_scan_indices:
            try:
                valid_scan_indices.append(int(scan_indices[i]))
            except Exception:
                valid_scan_indices.append(i)

        # AF detections vs scan index arrays (vehicle side)
        af_det_counts = []
        af_det_scanindex = []
        for i, a in enumerate(veh_rdd.get('num_af_det', [])):
            try:
                af_det_counts.append(int(a[0]) if isinstance(a, (list, np.ndarray)) else int(a))
            except (TypeError, IndexError, ValueError):
                af_det_counts.append(0)
            try:
                af_det_scanindex.append(int(scan_indices[i]))
            except Exception:
                af_det_scanindex.append(i)

        # Summary metrics
        valid_acc_values = [x for x in per_scan_accuracy if x is not None]
        min_acc = (min(valid_acc_values) if valid_acc_values else None)
        max_acc = (max(valid_acc_values) if valid_acc_values else None)
        scans_processed = int(len(veh_rdd.get('num_af_det', [])))
        scans_with_matches = int(sum(1 for m in per_scan_matches if m > 0))

        # Store results for HTML/plots
        self.kpi_results['per_scan_accuracy'] = per_scan_accuracy
        self.kpi_results['per_scan_scanindex'] = valid_scan_indices
        self.kpi_results['per_scan_matches'] = per_scan_matches
        self.kpi_results['per_scan_den'] = per_scan_den
        self.kpi_results['af_det_counts'] = af_det_counts
        self.kpi_results['af_det_scanindex'] = af_det_scanindex
        self.kpi_results['veh_si_count'] = int(veh_si_count)
        self.kpi_results['sim_si_count'] = int(sim_si_count)
        self.kpi_results['scans_processed'] = scans_processed
        self.kpi_results['scans_with_matches'] = scans_with_matches
        self.kpi_results['min_accuracy'] = min_acc
        self.kpi_results['max_accuracy'] = max_acc

        # Store both accuracies with clear naming
        self.kpi_results['matching_accuracy'] = {
            'matches': int(total_matches),
            'total_detections': int(total_detections),
            'detection_accuracy_percentage': detection_accuracy,
        }
        

                    
        acc_str = "N/A" if detection_accuracy is None else f"{detection_accuracy:.2f}%"
        logger.info(
            "Detection matching completed. Matches: %s, Total Detections: %s, Accuracy: %s",
            total_matches,
            total_detections,
            acc_str,
        )
        
        # Generate HTML report
        self.generate_html_report()
        
        return True
        
    def process_detection_matching(
        self,
        scan_gen_idx,
        store,
        store_is_detection_indices: bool = False,
        validate_values: bool = True,
    ):
        """
        Process detection matching for a single scan using the provided store mapping.
        
        Args:
            scan_gen_idx (int): Current scan index
            store (dict): Dictionary containing mapping between vehicle and sim detections
            
        Returns:
            int: Number of matches found in this scan
        """
        try:
            if not self.data:
                logger.warning("Insufficient detection data for KPI analysis")
                return 0

            # Define detection parameters to extract
            det_params = ['rdd_idx', 'ran', 'vel', 'theta', 'phi', 'f_single_target', 
                         'f_superres_target', 'f_bistatic', 'num_af_det']
            src = self.data['DETECTION_STREAM']
            
            # Initialize detection data structures
            veh_det = {param: [] for param in det_params}
            sim_det = {param: [] for param in det_params}
            
            # Get detection data for current scan
            for param in det_params:
                veh_result, veh_status = KPI_DataModelStorage.get_value(src['input'], param)
                sim_result, sim_status = KPI_DataModelStorage.get_value(src['output'], param)
                
                if veh_status == "success" and scan_gen_idx < len(veh_result):
                    veh_det[param] = veh_result[scan_gen_idx] if isinstance(veh_result[scan_gen_idx], (list, np.ndarray)) else [veh_result[scan_gen_idx]]
                if sim_status == "success" and scan_gen_idx < len(sim_result):
                    sim_det[param] = sim_result[scan_gen_idx] if isinstance(sim_result[scan_gen_idx], (list, np.ndarray)) else [sim_result[scan_gen_idx]]
            
            # Initialize match counter
            matches = 0

            # Pre-fetch arrays to avoid repeated dict lookups
            veh_ran_arr = veh_det.get('ran', [])
            veh_vel_arr = veh_det.get('vel', []) if 'vel' in veh_det else []
            veh_theta_arr = veh_det.get('theta', []) if 'theta' in veh_det else []
            veh_phi_arr = veh_det.get('phi', []) if 'phi' in veh_det else []
            sim_ran_arr = sim_det.get('ran', [])
            sim_vel_arr = sim_det.get('vel', []) if 'vel' in sim_det else []
            sim_theta_arr = sim_det.get('theta', []) if 'theta' in sim_det else []
            sim_phi_arr = sim_det.get('phi', []) if 'phi' in sim_det else []

            # Track used indices to enforce one-to-one without mutating lists during iteration
            used_veh = set()
            used_sim = set()

            # `store` can either already contain DETECTION_STREAM indices (AF detection indices),
            # or RDD indices which must be remapped via per-detection `rdd_idx`.
            if store_is_detection_indices:
                mapped_store = store
            else:
                veh_rdd_idx_arr = veh_det.get('rdd_idx', [])
                sim_rdd_idx_arr = sim_det.get('rdd_idx', [])

                def _build_rdd_to_det_map(rdd_idx_arr):
                    mapping = {}
                    for det_idx, rdd_idx in enumerate(rdd_idx_arr):
                        try:
                            if isinstance(rdd_idx, float) and np.isnan(rdd_idx):
                                continue
                        except Exception:
                            pass
                        try:
                            rid = int(rdd_idx)
                        except (TypeError, ValueError):
                            continue
                        mapping.setdefault(rid, []).append(det_idx)
                    return mapping

                veh_rdd_to_det = _build_rdd_to_det_map(veh_rdd_idx_arr)
                sim_rdd_to_det = _build_rdd_to_det_map(sim_rdd_idx_arr)

                mapped_store = {}
                unmapped_groups = 0

                for veh_rdd_key, sim_rdd_indices in store.items():
                    veh_rdd_list = list(veh_rdd_key) if isinstance(veh_rdd_key, tuple) else [veh_rdd_key]
                    sim_rdd_list = list(sim_rdd_indices) if isinstance(sim_rdd_indices, (list, tuple)) else [sim_rdd_indices]

                    veh_det_idxs = []
                    for rid in veh_rdd_list:
                        try:
                            rid_int = int(rid)
                        except (TypeError, ValueError):
                            continue
                        veh_det_idxs.extend(veh_rdd_to_det.get(rid_int, []))

                    sim_det_idxs = []
                    for rid in sim_rdd_list:
                        try:
                            rid_int = int(rid)
                        except (TypeError, ValueError):
                            continue
                        sim_det_idxs.extend(sim_rdd_to_det.get(rid_int, []))

                    if not veh_det_idxs or not sim_det_idxs:
                        unmapped_groups += 1
                        continue

                    veh_det_idxs = list(dict.fromkeys(veh_det_idxs))
                    sim_det_idxs = list(dict.fromkeys(sim_det_idxs))

                    veh_key_det = tuple(veh_det_idxs) if len(veh_det_idxs) > 1 else veh_det_idxs[0]
                    mapped_store[veh_key_det] = sim_det_idxs

                logger.debug(
                    "Scan %s: store_groups=%s mapped_groups=%s unmapped_groups=%s",
                    scan_gen_idx,
                    len(store),
                    len(mapped_store),
                    unmapped_groups,
                )

            for veh_key, sim_indices in mapped_store.items():
                veh_indices = list(veh_key) if isinstance(veh_key, tuple) else [veh_key]
                sim_list = list(sim_indices) if isinstance(sim_indices, (list, tuple)) else [sim_indices]

                for vi in veh_indices:
                    if (not isinstance(vi, (int, np.integer)) or
                        vi in used_veh or
                        vi < 0 or vi >= len(veh_ran_arr)):
                        # print("dontiunesd[adsfasdjfiosd]")
                        continue
                        
                    for si in sim_list:
                        if (not isinstance(si, (int, np.integer)) or
                            si in used_sim or
                            si < 0 or si >= len(sim_ran_arr)):
                            # print("dontiunesd[adsfasdjfiosd]")
                            continue

                        if not validate_values:
                            matches += 1
                            used_veh.add(vi)
                            used_sim.add(si)
                            break

                        try:
                            veh_ran = veh_ran_arr[vi]
                            sim_ran = sim_ran_arr[si]
                            veh_vel = veh_vel_arr[vi] if vi < len(veh_vel_arr) else 0
                            sim_vel = sim_vel_arr[si] if si < len(sim_vel_arr) else 0
                            veh_theta = veh_theta_arr[vi] if vi < len(veh_theta_arr) else 0
                            sim_theta = sim_theta_arr[si] if si < len(sim_theta_arr) else 0
                            veh_phi = veh_phi_arr[vi] if vi < len(veh_phi_arr) else 0
                            sim_phi = sim_phi_arr[si] if si < len(sim_phi_arr) else 0

                            if (
                                abs(veh_ran - sim_ran) <= self.RAN_THRESHOLD
                                and abs(veh_vel - sim_vel) <= self.VEL_THRESHOLD
                                and abs(veh_theta - sim_theta) <= self.THETA_THRESHOLD
                                and abs(veh_phi - sim_phi) <= self.PHI_THRESHOLD
                            ):
                                matches += 1
                                used_veh.add(vi)
                                used_sim.add(si)
                                break  # one match per veh detection, move to next vi
                        except (IndexError, KeyError, TypeError) as e:
                            logger.warning(f"Error comparing detections: {e}")
                            continue
            
            logger.debug(f"Scan {scan_gen_idx}: Found {matches} matching detections")
            return matches
            
        except Exception as e:
            logger.error(f"Error in process_detection_matching: {e}")
            import traceback
            logger.error(traceback.format_exc())
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

            extra_kpi_boxes = ""
            
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
                extra_kpi_boxes=extra_kpi_boxes,
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
