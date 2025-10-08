import plotly.express as px
import plotly.graph_objects as go
from d_presentation_layer.plotlyapi import PlotlyCharts
from d_presentation_layer.plot_generator import PlotGenerator

class DataPrep:
    def __init__(self, data_container, unique_map, data_container_out, unique_map_out, html_name):
        """Initialize DataPrep with necessary parameters."""
        self.data_container = data_container
        self.unique_map = unique_map
        self.data_container_out = data_container_out
        self.unique_map_out = unique_map_out
        self.html_name = html_name

        # Create parent tabs and generate HTML content
        self.parent_tabs = self.create_parent_tabs()
        self.plots_hash = self.generate_html_content()
        
        # Initialize PlotGenerator with plots_hash and parent_tabs
        PlotGenerator(self.plots_hash, self.parent_tabs,self.html_name)

    def create_parent_tabs(self):
        """Create tabs based on the number of parents."""
        tabs = {}
        for parent_key in self.unique_map.keys():
            if parent_key.endswith("_None"):
                tabs[parent_key.split('_')[0]] = self.unique_map[parent_key]
        return tabs

    def generate_html_content(self):
        """Generate HTML content for plots."""
        plots_hash = {}
        
        # Check for missing data between unique maps
        if self.unique_map != self.unique_map_out:
            missing_data = set(self.unique_map.keys()) - set(self.unique_map_out.keys())
            print(f"Missing data: {missing_data}")

            # Remove missing data from the output map if not in the input map
            for key in missing_data:
                if key in self.data_container_out:
                    del self.data_container_out[key]

        unique_keys = set(self.data_container.keys()).union(set(self.data_container_out.keys()))
        unique_keys.pop()

        # Initialize a dictionary to hold figures
        figs = {}
        inp_values = list(self.data_container.values())[0]
        for j in range(len(inp_values)):
            for k in range(len(inp_values[j])):
                
                fig_id = f"fig{j}_{k}"  # Create a unique identifier for the figure
                fig = go.Figure()
                    
                for key, input_data, output_data in zip(unique_keys,self.data_container.values(), self.data_container_out.values()):

                    
                    data_values = input_data[j][k]
                    data_out_values = output_data[j][k]
                    intersection = set(data_values).intersection(set(data_out_values))

                    # Plot intersection points
                    if intersection:
                        fig.add_trace(go.Scatter(
                            x=[key] * len(intersection),
                            y=list(intersection),
                            mode='markers',
                            name=f'Intersec {self.unique_map[f"{j}_{k}"]}',
                            marker=dict(color='purple', size=10)
                        ))

                    # Plot unique data points from input
                    unique_data_values = set(data_values) - intersection
                    if unique_data_values:
                        fig.add_trace(go.Scatter(
                            x=[key] * len(unique_data_values),
                            y=list(unique_data_values),
                            mode='markers',
                            name=f'Data {self.unique_map[f"{j}_{k}"]}',
                            marker=dict(color='blue')
                        ))

                    # Plot unique data points from output
                    unique_data_out_values = set(data_out_values) - intersection
                    if unique_data_out_values:
                        fig.add_trace(go.Scatter(
                            x=[key] * len(unique_data_out_values),
                            y=list(unique_data_out_values),
                            mode='markers',
                            name=f'Data Out {self.unique_map[f"{j}_{k}"]}',
                            marker=dict(color='red')
                        ))

                    # Update layout for each figure
                    fig.update_layout(
                        title=f'Scatter Plot with {self.unique_map[f"{j}_{k}"]}',
                        xaxis_title='ScanIndex',
                        yaxis_title='Values',
                        legend_title='Legend'
                    )
                figs[fig_id] = fig

        # Populate plots_hash with figures based on unique map keys
        
        for j in range(len(inp_values)):
            for k in range(len(inp_values[j])):
                fig_id = f"fig{j}_{k}"
                for p in self.unique_map.keys():
                    if p.startswith(f"{j}_")and not p.endswith(f"_None"):
                        if self.unique_map[f"{j}_None"] not in plots_hash:
                            plots_hash[self.unique_map[f"{j}_None"]] = []
                        else:
                            plots_hash[self.unique_map[f"{j}_None"]].append(figs[fig_id])

        return plots_hash
