import plotly.graph_objects as go
import pandas as pd
import numpy as np

from KPI.detection_matching_kpi import KpiDataModel
from InteractivePlot.d_presentation_layer.plotly_visualization import PlotlyCharts
from InteractivePlot.d_presentation_layer.html_generator import HtmlGenerator

class DataPrep:
    """
    Prepares data for visualization by generating plots and handling missing data.
    """
    
    def __init__(self, data_container_in, val_sig_map_in, sig_val_map_in, 
                 data_container_out, val_sig_map_out, sig_val_map_out, html_name):
        """
        Initializes DataPrep with necessary parameters.

        Parameters:
        - data_container_in: Input data container.
        - val_sig_map_in: Input value-signal map.
        - sig_val_map_in: Input signal-value map.
        - data_container_out: Output data container.
        - val_sig_map_out: Output value-signal map.
        - sig_val_map_out: Output signal-value map.
        - html_name: Name for the generated HTML file.
        """
        self.data_container_in = data_container_in
        self.val_sig_map_in = val_sig_map_in
        self.sig_val_map_in = sig_val_map_in
        self.data_container_out = data_container_out
        self.val_sig_map_out = val_sig_map_out
        self.sig_val_map_out = sig_val_map_out
        self.html_name = html_name

        # Get KPI plots
        self.kpi_plots = KpiDataModel(data_container_in, val_sig_map_in, sig_val_map_in, 
                                      data_container_out, val_sig_map_out, sig_val_map_out)

        # Generate plots and create HTML content
        self.plots_hash = self.generate_plots()
        
        # Initialize HtmlGenerator with plots_hash and parent_tabs
        HtmlGenerator(self.plots_hash,self.kpi_plots,self.html_name)


    def generate_plots(self):
        """
        Generates HTML content for plots by handling missing data and creating scatter plots.

        Returns:
        - plots_hash: A dictionary containing plots organized by parent tabs.
        """
        plots_hash = {}
        missing_data_out = set()
        missing_data_in = set()
        
        # Check for missing data between input and output maps
        if self.val_sig_map_in != self.val_sig_map_out:
            missing_data_out = set(self.val_sig_map_in.keys()) - set(self.val_sig_map_out.keys())
            missing_data_in = set(self.val_sig_map_out.keys()) - set(self.val_sig_map_in.keys())
            # print(f"Missing data: {[self.sig_val_map_in[f'{value}'] for value in missing_data]}")

        # Get all unique keys from both data containers
        unique_keys = set(self.data_container_in.keys()).union(set(self.data_container_out.keys()))

        # Initialize a dictionary to hold figures/plots
        plots = {}
        
        inp_values = list(self.data_container_in.values())[0]
        out_values = list(self.data_container_out.values())[0]

        # Loop over groups and plot indices
        for grp_idx in range(max(len(inp_values), len(out_values))):
            for plt_idx in range(max(len(inp_values[grp_idx]), len(out_values[grp_idx]))):
                # This loop creates one particular Plotly image
                key = f"{grp_idx}_{plt_idx}"
                fig_id = f"fig{key}"  # Unique identifier for the figure
                
                # List to hold data records for building the DataFrame for the scatter plot
                data_records = []
                
                # For each key (scan) in unique_keys, collect corresponding data
                for scan_idx, input_data, output_data in zip(unique_keys, self.data_container_in.values(), self.data_container_out.values()):
                    # Handle missing data
                    if (key in missing_data_in and self.data_container_in) or (key in missing_data_out and self.data_container_out):      
                        print(f"ERROR: This HDF file pair is not correct {self.html_name}. Please give proper one.")
                        # TODO: Raise an exception to skip the rest of the process.
                        # raise SkipProcessError(f"File pair error: {html_name}")
                    
                    elif f"{key}" in missing_data_out:
                        if "None" not in f"{key}":
                            self.data_container_out[scan_idx][grp_idx].insert(plt_idx, [])
                            self.val_sig_map_out[f"{key}"] = self.val_sig_map_in[f"{key}"]
                        elif "None" in f"{key}":
                            self.data_container_out[scan_idx].insert(grp_idx, [])
                            self.val_sig_map_out[f"{key}"] = self.val_sig_map_in[f"{key}"]
                    
                    elif f"{key}" in missing_data_in:
                        if "None" not in f"{key}":
                            self.data_container_in[scan_idx][grp_idx].insert(plt_idx, [])
                            self.val_sig_map_in[f"{key}"] = self.val_sig_map_out[f"{key}"]
                        elif "None" in f"{key}":
                            self.data_container_in[scan_idx].insert(grp_idx, [])
                            self.val_sig_map_in[f"{key}"] = self.val_sig_map_out[f"{key}"]
                    
                    # Initialize data arrays
                    data_in_values = np.array([])
                    data_out_values = np.array([])

                    if grp_idx < len(input_data) and plt_idx < len(input_data[grp_idx]):
                        data_in_values = input_data[grp_idx][plt_idx]
                    
                    if grp_idx < len(output_data) and plt_idx < len(output_data[grp_idx]):
                        data_out_values = output_data[grp_idx][plt_idx]
                    
                    if isinstance(data_in_values, (np.uint32, np.uint8, np.float32)):
                        data_in_values = [data_in_values]
                    if isinstance(data_out_values, (np.uint32, np.uint8, np.float32)):
                        data_out_values = [data_out_values]
                    if not isinstance(data_in_values, (list, np.ndarray)):
                        data_in_values = []
                    if not isinstance(data_out_values, (list, np.ndarray)):
                        data_out_values = []

                    # Find intersection and unique values
                    intersection = set(data_in_values).intersection(set(data_out_values))
                    unique_data_values = set(data_in_values) - intersection
                    unique_data_out_values = set(data_out_values) - intersection
                    
                    # Add data records for intersection and unique values
                    if intersection:
                        data_records.extend([(scan_idx, val, f'Intersection value in {self.val_sig_map_in[f"{key}"]}') for val in intersection])
                    
                    if unique_data_values:
                        data_records.extend([(scan_idx, val, f'Input of {self.val_sig_map_in[f"{key}"]} ') for val in unique_data_values])
                    
                    if unique_data_out_values:
                        data_records.extend([(scan_idx, val, f'Output of {self.val_sig_map_out[f"{key}"]}') for val in unique_data_out_values])
                
                # Create a scatter plot
                fig = go.Figure()
                
                # Create a DataFrame from the data records
                df = pd.DataFrame(data_records, columns=['ScanIndex', 'Values', 'Type'])

                # Define color mapping
                color_map = {'Intersection': 'purple', 'Input': 'blue', 'Output': 'red'}

                # Iterate through each type and add a scatter trace
                for trace_type, color in color_map.items():
                    trace_data = df[df['Type'].str.contains(trace_type)]
                    fig.add_trace(go.Scatter(x=trace_data['ScanIndex'], 
                                             y=trace_data['Values'], 
                                             mode='markers', 
                                             name=trace_type,
                                             marker=dict(color=color)))
                
                fig.update_layout(
                    title=f'Scatter Plot of {self.val_sig_map_in[f"{key}"]}',
                    xaxis_title='ScanIndex',
                    yaxis_title=f'Values of  {self.val_sig_map_in[f"{key}"]} ',
                    legend_title='Legend'
                )
                
                plots[fig_id] = fig
                
                # Organize plots by parent tabs
                mapp = f"{key}"
                if mapp.startswith(f"{grp_idx}_") and not mapp.endswith(f"_None"):
                    tab_name = self.val_sig_map_in[f"{grp_idx}_None"][0].rsplit('/', 1)[-1]
                    if tab_name not in plots_hash:
                        plots_hash[tab_name] = []
                    plots_hash[tab_name].append(plots[fig_id])
        
        return plots_hash
