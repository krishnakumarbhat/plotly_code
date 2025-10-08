import numpy as np
import plotly.graph_objs as go
import plotly.subplots as sp
import plotly.io as pio
from collections import Counter

class DetectionKPINumPy:
    """
    Processes detection mapping KPIs from a dictionary of NumPy arrays,
    offering a high-performance alternative to a pandas-based approach.
    """
    def __init__(self, data: dict, sensor_id: str):
        """
        Initializes the processor with data and configuration.

        Args:
            data (dict): A dictionary containing 'DETECTION_STREAM' and 'RDD' data.
                         All values in the dictionary are expected to be NumPy arrays.
                         Detection parameters (e.g., 'ran', 'vel') should be 2D arrays
                         of shape (num_scans, max_detections).
            sensor_id (str): The identifier for the sensor being processed.
        """
        self.data = data
        self.sensor_id = sensor_id

        # --- 1. Thresholds and Configuration ---
        self.thresholds = {
            "range_threshold": 0.05,
            "velocity_threshold": 0.05,
            "theta_threshold": 0.002,
            "phi_threshold": 0.004,
        }

        # --- Initialize result containers ---
        self.kpi_results = {}
        self.html_content = ""
        self.error_logs = {
            'ran_diff_list': [], 'vel_diff_list': [],
            'theta_diff_list': [], 'phi_diff_list': []
        }

    def run(self):
        """
        Executes the full KPI processing pipeline.
        
        Returns:
            bool: True if processing was successful, False otherwise.
        """
        try:
            # --- 2. Data Preparation and Indexing ---
            # Find common scan indices across all necessary streams
            common_scans = np.intersect1d(self.data['DETECTION_STREAM']['input']['scan_index'],
                                          self.data['DETECTION_STREAM']['output']['scan_index'])
            common_scans = np.intersect1d(common_scans, self.data['RDD']['input']['scan_index'])
            common_scans = np.intersect1d(common_scans, self.data['RDD']['output']['scan_index'])
            
            # Further filter for scans that have detections in both input and output
            veh_det_scans = self.data['DETECTION_STREAM']['input']['scan_index'][self.data['DETECTION_STREAM']['input']['num_af_det'] > 0]
            sim_det_scans = self.data['DETECTION_STREAM']['output']['scan_index'][self.data['DETECTION_STREAM']['output']['num_af_det'] > 0]
            
            common_scans = np.intersect1d(common_scans, veh_det_scans)
            common_scans = np.intersect1d(common_scans, sim_det_scans)

            if common_scans.size == 0:
                self.html_content = f"<p><strong>Warning for {self.sensor_id}:</strong> No common scan indices with valid detections found across all data streams.</p>"
                return False

            # Create a lookup map from scan_index to array index for fast access
            self.idx_maps = {
                'veh_det': {si: i for i, si in enumerate(self.data['DETECTION_STREAM']['input']['scan_index'])},
                'sim_det': {si: i for i, si in enumerate(self.data['DETECTION_STREAM']['output']['scan_index'])},
                'veh_rdd': {si: i for i, si in enumerate(self.data['RDD']['input']['scan_index'])},
                'sim_rdd': {si: i for i, si in enumerate(self.data['RDD']['output']['scan_index'])}
            }
            
            self.common_scans = common_scans

            # --- 3. RDD and AF KPI Processing ---
            rdd_kpis = self._process_rdd_matching()
            af_kpis = self._process_af_matching()

            # --- 4. Combine Results & Generate Report ---
            self.kpi_results = {'rdd_kpis': rdd_kpis, 'af_kpis': af_kpis, 'sensor_id': self.sensor_id}
            self._generate_html_report()
            
            return True
        except KeyError as e:
            self.html_content = f"<h3>Error: Missing expected key in input data for {self.sensor_id}</h3><p>Could not find key: {e}</p>"
            return False
        except Exception as e:
            self.html_content = f"<h3>An error occurred during processing for {self.sensor_id}</h3><p>{e}</p>"
            return False
            
    def _process_rdd_matching(self):
        """Calculates KPIs for the RDD stream using NumPy arrays."""
        num_same_si = len(self.common_scans)
        num_same_num_dets = 0
        
        veh_rdd_num_detect = self.data['RDD']['input']['rdd1_num_detect']
        sim_rdd_num_detect = self.data['RDD']['output']['rdd1_num_detect']
        
        for si in self.common_scans:
            veh_idx = self.idx_maps['veh_rdd'][si]
            sim_idx = self.idx_maps['sim_rdd'][si]
            if veh_rdd_num_detect[veh_idx] == sim_rdd_num_detect[sim_idx]:
                num_same_num_dets += 1

        return {
            'num_same_si': num_same_si,
            'num_same_num_dets': num_same_num_dets,
            'same_num_dets_pct': round((num_same_num_dets / num_same_si * 100), 2) if num_same_si > 0 else 0
        }

    def _process_af_matching(self):
        """Calculates KPIs for the AF Detection stream."""
        total_detections_veh = 0
        matched_detections = 0
        
        for si in self.common_scans:
            veh_det_idx = self.idx_maps['veh_det'][si]
            sim_det_idx = self.idx_maps['sim_det'][si]
            
            num_veh_dets = self.data['DETECTION_STREAM']['input']['num_af_det'][veh_det_idx]
            total_detections_veh += num_veh_dets
            
            matched_detections += self._count_matches_per_scan(si, veh_det_idx, sim_det_idx)
        
        return {
            'total_detections_veh': total_detections_veh,
            'matched_detections': matched_detections,
            'accuracy': round((matched_detections / total_detections_veh * 100), 2) if total_detections_veh > 0 else 0,
            'num_scans_processed': len(self.common_scans)
        }

    def _get_rdd_pair(self, stream_type, det_idx, rdd_idx_val):
        """Fetches the (rindx, dindx) pair for a given detection."""
        rdd_idx = self.idx_maps[f'{stream_type}_rdd'][self.data['DETECTION_STREAM'][stream_type]['scan_index'][det_idx]]
        rindx = self.data['RDD'][stream_type]['rdd1_rindx'][rdd_idx, rdd_idx_val]
        dindx = self.data['RDD'][stream_type]['rdd1_dindx'][rdd_idx, rdd_idx_val]
        return (rindx, dindx)

    def _count_matches_per_scan(self, scan_index, veh_det_idx, sim_det_idx):
        """Core matching logic for a single scan_index."""
        veh_stream = self.data['DETECTION_STREAM']['input']
        sim_stream = self.data['DETECTION_STREAM']['output']
        
        num_veh = veh_stream['num_af_det'][veh_det_idx]
        num_sim = sim_stream['num_af_det'][sim_det_idx]
        
        # Create a lookup map for simulation detections for better performance
        sim_map = {}
        for j in range(num_sim):
            rdd_idx_val = int(sim_stream['rdd_idx'][sim_det_idx, j])
            sim_pair = self._get_rdd_pair('output', sim_det_idx, rdd_idx_val)
            
            if sim_pair not in sim_map:
                sim_map[sim_pair] = []
            
            sim_map[sim_pair].append({
                'ran': sim_stream['ran'][sim_det_idx, j], 'vel': sim_stream['vel'][sim_det_idx, j],
                'theta': sim_stream['theta'][sim_det_idx, j], 'phi': sim_stream['phi'][sim_det_idx, j],
                'used': False
            })

        match_count = 0
        for i in range(num_veh):
            rdd_idx_val = int(veh_stream['rdd_idx'][veh_det_idx, i])
            veh_pair = self._get_rdd_pair('input', veh_det_idx, rdd_idx_val)

            if veh_pair in sim_map:
                for sim_det in sim_map[veh_pair]:
                    if not sim_det['used']:
                        veh_ran = veh_stream['ran'][veh_det_idx, i]
                        veh_vel = veh_stream['vel'][veh_det_idx, i]
                        veh_theta = veh_stream['theta'][veh_det_idx, i]
                        veh_phi = veh_stream['phi'][veh_det_idx, i]
                        
                        ran_ok = abs(veh_ran - sim_det['ran']) <= self.thresholds['range_threshold']
                        vel_ok = abs(veh_vel - sim_det['vel']) <= self.thresholds['velocity_threshold']
                        theta_ok = abs(veh_theta - sim_det['theta']) <= self.thresholds['theta_threshold']
                        phi_ok = abs(veh_phi - sim_det['phi']) <= self.thresholds['phi_threshold']

                        if ran_ok and vel_ok and theta_ok and phi_ok:
                            match_count += 1
                            sim_det['used'] = True
                            break
                        else:
                            veh_props = (veh_ran, veh_vel, veh_theta, veh_phi)
                            if not ran_ok: self.error_logs['ran_diff_list'].append(veh_props + (veh_ran - sim_det['ran'],))
                            if not vel_ok: self.error_logs['vel_diff_list'].append(veh_props + (veh_vel - sim_det['vel'],))
                            if not theta_ok: self.error_logs['theta_diff_list'].append(veh_props + (veh_theta - sim_det['theta'],))
                            if not phi_ok: self.error_logs['phi_diff_list'].append(veh_props + (veh_phi - sim_det['phi'],))
        return match_count

    # --- HTML and Plotting Methods (Largely unchanged) ---
    def _generate_html_report(self):
        """Assembles the final HTML content with summaries and plots."""
        summary_html = self._create_summary_html()
        plots_html = self._create_plots_html()
        self.html_content = f"""
        <div class="kpi-section" style="font-family: sans-serif; border: 1px solid #ccc; padding: 15px; margin-bottom: 15px;">
            <h2>Detection Mapping KPI Results - {self.sensor_id}</h2>
            {summary_html}
            <hr>
            {plots_html}
        </div>
        """

    def _create_summary_html(self):
        af = self.kpi_results.get('af_kpis', {})
        rdd = self.kpi_results.get('rdd_kpis', {})
        return f"""
        <div class="kpi-summary">
            <p><strong>Overall Accuracy:</strong> {af.get('accuracy', 0):.2f}%</p>
            <p><strong>Total Detections (Input):</strong> {int(af.get('total_detections_veh', 0))}</p>
            <p><strong>Matched Detections:</strong> {int(af.get('matched_detections', 0))}</p>
            <p><strong>Common Scans Processed:</strong> {af.get('num_scans_processed', 0)}</p>
        </div>
        <details>
            <summary><strong>Detailed Results & Thresholds</strong></summary>
            <h4>RDD Stream</h4>
            <ul>
                <li>Scans with same number of RDD detections: {rdd.get('num_same_num_dets', 'N/A')} / {rdd.get('num_same_si', 'N/A')} ({rdd.get('same_num_dets_pct', 0):.2f}%)</li>
            </ul>
            <h4>Thresholds Used</h4>
            <ul>
                <li>Range: &plusmn;{self.thresholds['range_threshold']:.4f} m</li>
                <li>Velocity: &plusmn;{self.thresholds['velocity_threshold']:.4f} m/s</li>
                <li>Azimuth: &plusmn;{self.thresholds['theta_threshold']:.4f} rad</li>
                <li>Elevation: &plusmn;{self.thresholds['phi_threshold']:.4f} rad</li>
            </ul>
        </details>
        """
        
    def _create_plots_html(self):
        """Generates Plotly figures and converts them to HTML."""
        def bar_trace(diffs):
            if not diffs: return go.Bar()
            counts = Counter(np.round(diffs, 3))
            total = len(diffs)
            x_vals, y_vals = zip(*sorted({k: (v / total) * 100 for k, v in counts.items()}.items()))
            return go.Bar(x=x_vals, y=y_vals)

        def scatter_trace(props, errors, prop_idx):
            if not props: return go.Scatter()
            return go.Scatter(x=[p[prop_idx] for p in props], y=errors, mode='markers', marker={'size': 4, 'opacity': 0.7})
        
        fig_bar = sp.make_subplots(rows=2, cols=2, subplot_titles=("Range Error", "Velocity Error", "Azimuth Error", "Elevation Error"))
        fig_bar.add_trace(bar_trace([e[-1] for e in self.error_logs['ran_diff_list']]), 1, 1)
        fig_bar.add_trace(bar_trace([e[-1] for e in self.error_logs['vel_diff_list']]), 1, 2)
        fig_bar.add_trace(bar_trace([e[-1] for e in self.error_logs['theta_diff_list']]), 2, 1)
        fig_bar.add_trace(bar_trace([e[-1] for e in self.error_logs['phi_diff_list']]), 2, 2)
        fig_bar.update_layout(height=600, title_text="Unmatched Detections: Error Distributions (%)", showlegend=False)
        bar_html = pio.to_html(fig_bar, full_html=False, include_plotlyjs=False)

        return f"""
        <h3>Analysis Plots</h3>
        <details open>
            <summary><b>Plot 1: Error Distributions</b></summary>
            {bar_html}
        </details>
        """
        
    def get_results(self):
        """Returns the processed KPI results and the generated HTML content."""
        return {
            'kpi_results': self.kpi_results,
            'html_content': self.html_content
        }

# --- Example Usage ---
if __name__ == '__main__':
    # --- 1. Create Sample Data using NumPy Arrays ---
    # This simulates the data you would load from your HDF files.
    # Note the 2D shape for detection parameters.
    
    NUM_SCANS = 100
    MAX_AF_DETS = 64
    MAX_RDD_DETS = 256
    
    # Generate common scan indices
    scan_indices = np.arange(1000, 1000 + NUM_SCANS)
    
    def create_stream_data(is_sim):
        num_af_det = np.random.randint(5, MAX_AF_DETS - 10, size=NUM_SCANS)
        ran_data = np.random.uniform(10, 150, size=(NUM_SCANS, MAX_AF_DETS))
        vel_data = np.random.uniform(-30, 30, size=(NUM_SCANS, MAX_AF_DETS))
        
        # Add slight variations for simulation data
        if is_sim:
            ran_data += np.random.normal(0, 0.02, size=ran_data.shape)
            vel_data += np.random.normal(0, 0.02, size=vel_data.shape)

        return {
            'scan_index': scan_indices,
            'num_af_det': num_af_det,
            'rdd_idx': np.random.randint(0, MAX_RDD_DETS, size=(NUM_SCANS, MAX_AF_DETS)),
            'ran': ran_data,
            'vel': vel_data,
            'theta': np.random.uniform(-0.5, 0.5, size=(NUM_SCANS, MAX_AF_DETS)),
            'phi': np.random.uniform(-0.1, 0.1, size=(NUM_SCANS, MAX_AF_DETS)),
        }
        
    def create_rdd_data():
        return {
            'scan_index': scan_indices,
            'rdd1_num_detect': np.full(NUM_SCANS, MAX_RDD_DETS),
            'rdd1_rindx': np.random.randint(0, 512, size=(NUM_SCANS, MAX_RDD_DETS)),
            'rdd1_dindx': np.random.randint(0, 256, size=(NUM_SCANS, MAX_RDD_DETS)),
        }

    sample_np_data = {
        'DETECTION_STREAM': {
            'input': create_stream_data(is_sim=False),
            'output': create_stream_data(is_sim=True)
        },
        'RDD': {
            'input': create_rdd_data(),
            'output': create_rdd_data() 
        }
    }
    
    # --- 2. Instantiate and Run the Processor ---
    processor_np = DetectionKPINumPy(data=sample_np_data, sensor_id="ARS540_FC_NumPy_Example")
    success = processor_np.run()

    # --- 3. Get the Results and Save HTML Report ---
    if success:
        results = processor_np.get_results()
        
        print("KPI Results (NumPy):", results['kpi_results'])
        
        with open("detection_kpi_report_numpy.html", "w") as f:
            f.write("<html><head><title>NumPy KPI Report</title>")
            f.write('<script src="https://cdn.plot.ly/plotly-latest.min.js"></script>')
            f.write("</head><body>")
            f.write(results['html_content'])
            f.write("</body></html>")
        print("\nSuccessfully generated report: detection_kpi_report_numpy.html")
    else:
        print("\nKPI processing with NumPy failed.")
        # Print the HTML content which might contain the error message
        print(processor_np.get_results()['html_content'])