import plotly.express as px
import plotly.graph_objects as go
import pandas as pd
import numpy as np
from KPI.detection_matching_kpi import KpiDataModel
from InteractivePlot.d_presentation_layer.plotly_visualization import PlotlyCharts
from InteractivePlot.d_presentation_layer.html_generator import HtmlGenerator

class DataPrep:
    def __init__(self, data_container_in, val_sig_map_in,sig_val_map_in , data_container_out, val_sig_map_out,sig_val_map_out, html_name):
        """Initialize DataPrep with necessary parameters."""
        self.data_container_in = data_container_in
        self.val_sig_map_in = val_sig_map_in
        self.sig_val_map_in = sig_val_map_in
        self.data_container_out = data_container_out
        self.val_sig_map_out = val_sig_map_out
        self.sig_val_map_out = sig_val_map_out
        self.html_name = html_name

        #get the plotly plot from kpi class
        self.kpi_plots = KpiDataModel(data_container_in, val_sig_map_in,sig_val_map_in, data_container_out, val_sig_map_out,sig_val_map_out)

        # Create parent tabs and generate HTML content
        # self.parent_tabs = self.create_parent_tabs()
        self.plots_hash = self.generate_plots()
        
        # Initialize HtmlGenerator with plots_hash and parent_tabs
        HtmlGenerator(self.plots_hash,self.kpi_plots,self.sig_val_map_in,self.html_name)

    # def create_parent_tabs(self):
    #     """Create tabs based on the number of parents."""
    #     tabs = {}
    #     for parent_key in self.val_sig_map_in.keys():
    #         if parent_key.endswith("_None"):
    #             tabs[parent_key.split('_')[0]] = self.val_sig_map_in[parent_key]
    #     return tabs

    def generate_plots(self):
        """Generate HTML content for plots."""
        plots_hash = {}
        
        # Check for missing data between unique maps
        if self.val_sig_map_in != self.val_sig_map_out:
            missing_data = set(self.val_sig_map_in.keys()) - set(self.val_sig_map_out.keys())
            # print(f"Missing data: {[self.sig_val_map_in[f'{value}'] for value in missing_data]}")

            # Remove missing data from the output map if not in the input map
            # for key in missing_data:
            #     if key in self.data_container_out:
            #         del self.data_container_out[key]

        unique_keys = set(self.data_container_in.keys()).union(set(self.data_container_out.keys()))

        # Initialize a dictionary to hold figures/plots.
        plots = {}
        inp_values = list(self.data_container_in.values())[0]
        for j in range(len(inp_values)):
            for k in range(len(inp_values[j])):
                
                fig_id = f"fig{j}_{k}"  # Create a unique identifier for the figure
                fig = go.Figure()
                # Collect data for DataFrame
                data_records = []

                for scan_idx, input_data, output_data in zip(unique_keys,self.data_container_in.values(), self.data_container_out.values()):

                    try:
                        data_values = input_data[j][k]
                    except:
                        data_values = np.array([])

                    try:
                        data_out_values = output_data[j][k]
                    except:
                        data_out_values = np.array([])
                    
                    intersection = set(data_values).intersection(set(data_out_values))

                    # Plot intersection points
                    if intersection:
                        data_records.extend([(scan_idx, val, 'Intersection') for val in intersection])
                        fig.add_trace(go.Scatter(
                            x=[scan_idx ] * len(intersection),
                            y=list(intersection),
                            mode='markers',
                            name=f'Intersec {self.val_sig_map_in[f"{j}_{k}"]}',
                            marker=dict(color='purple', size=10)
                        ))

                    # Plot unique data points from input
                    if unique_data_values := set(data_values) - intersection:
                        data_records.extend([(scan_idx, val, 'Input') for val in unique_data_values])

                        fig.add_trace(go.Scatter(
                            x=[scan_idx ] * len(unique_data_values),
                            y=list(unique_data_values),
                            mode='markers',
                            name=f'Data {self.val_sig_map_in[f"{j}_{k}"]}',
                            marker=dict(color='blue')
                        ))

                    # Plot unique data points from output
                    if unique_data_out_values := set(data_out_values) - intersection:
                        data_records.extend([(scan_idx, val, 'Output') for val in unique_data_out_values])
                        fig.add_trace(go.Scatter(
                            x=[scan_idx ] * len(unique_data_out_values),
                            y=list(unique_data_out_values),
                            mode='markers',
                            name=f'Data Out {self.val_sig_map_in[f"{j}_{k}"]}',
                            marker=dict(color='red')
                        ))

                    # Update layout for each figure
                    fig.update_layout(
                        title=f'Scatter Plot with {self.val_sig_map_in[f"{j}_{k}"]}',
                        xaxis_title='ScanIndex',
                        yaxis_title='Values',
                        legend_title='Legend'
                    )
                plots[fig_id] = fig

            df = pd.DataFrame(data_records, columns=['ScanIndex', 'Values', 'Type'])

            # Create scatter plot using the DataFrame
            fig = px.scatter(df, x='ScanIndex', y='Values', color='Type', 
                             color_discrete_map={'Intersection': 'purple', 'Input': 'blue', 'Output': 'red'},
                             title=f'Scatter Plot with {self.val_sig_map_in[f"{j}_{k}"]}')

        # Populate plots_hash with figures based on unique map keys
        
        for j in range(len(inp_values)):
            for k in range(len(inp_values[j])):
                fig_id = f"fig{j}_{k}"
                for p in self.val_sig_map_in.keys():
                    if p.startswith(f"{j}_")and not p.endswith(f"_None"):
                        if self.val_sig_map_in[f"{j}_None"] not in plots_hash:
                            plots_hash[self.val_sig_map_in[f"{j}_None"]] = []
                        else:
                            plots_hash[self.val_sig_map_in[f"{j}_None"]].append(plots[fig_id])

        return plots_hash
