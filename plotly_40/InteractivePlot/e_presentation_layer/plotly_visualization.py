import plotly.graph_objects as go
import pandas as pd
import numpy as np

class PlotlyCharts:
    
    @staticmethod
    def create_scatter(x, y, title='Scatter Plot', x_label='X-axis', y_label='Y-axis'):
        """
        Create a basic scatter plot.
        
        Parameters:
        - x: List of x values
        - y: List of y values
        - title: Title of the plot
        - x_label: Label for the x-axis
        - y_label: Label for the y-axis
        
        Returns:
        - fig: Plotly figure object
        """
        fig = go.Figure(data=go.Scattergl(x=x, y=y, mode='markers'))
        fig.update_layout(title=title, xaxis_title=x_label, yaxis_title=y_label)
        return fig

    @staticmethod
    def create_intersection_scatter(data, data_out, title='Scatter Plot with Intersections', x_label='Keys', y_label='Values'):
        """
        Create a scatter plot with intersection handling.
        
        Parameters:
        - data: Dictionary with keys and lists of values for dataset 1
        - data_out: Dictionary with keys and lists of values for dataset 2
        - title: Title of the plot
        - x_label: Label for the x-axis
        - y_label: Label for the y-axis
        
        Returns:
        - fig: Plotly figure object
        """
        fig = go.Figure()
        
        # Create a set to store unique keys
        unique_keys = set(data.keys()).union(set(data_out.keys()))

        # Iterate through each key to plot
        for key in unique_keys:
            # Get values from both datasets
            data_values = data.get(key, [])
            data_out_values = data_out.get(key, [])

            intersection = set(data_values).intersection(set(data_out_values))
            
            if intersection:
                fig.add_trace(go.Scattergl(
                    x=[key] * len(intersection),
                    y=list(intersection),
                    mode='markers',
                    name=f'Intersection {key}',
                    marker=dict(color='purple', size=10)
                ))
            
            # Plot unique data points not in the intersection
            unique_data_values = set(data_values) - intersection
            if unique_data_values:
                fig.add_trace(go.Scattergl(
                    x=[key] * len(unique_data_values),
                    y=list(unique_data_values),
                    mode='markers',
                    name=f'Data {key}',
                    marker=dict(color='blue')
                ))

            unique_data_out_values = set(data_out_values) - intersection
            if unique_data_out_values:
                fig.add_trace(go.Scattergl(
                    x=[key] * len(unique_data_out_values),
                    y=list(unique_data_out_values),
                    mode='markers',
                    name=f'Data Out {key}',
                    marker=dict(color='red')
                ))

        # Update layout
        fig.update_layout(
            title=title,
            xaxis_title=x_label,
            yaxis_title=y_label,
            legend_title='Legend'
        )

        return fig

    @staticmethod
    def create_box(data, title='Box Plot', y_label='Values'):
        """
        Create a box plot.
        
        Parameters:
        - data: List of values to plot
        - title: Title of the plot
        - y_label: Label for the y-axis
        
        Returns:
        - fig: Plotly figure object
        """
        fig = go.Figure(data=go.Box(y=data))
        fig.update_layout(title=title, yaxis_title=y_label)
        return fig

    @staticmethod
    def create_pie(labels, values, title='Pie Chart'):
        """
        Create a pie chart.
        
        Parameters:
        - labels: List of labels for each sector
        - values: List of values corresponding to each label
        - title: Title of the plot
        
        Returns:
        - fig: Plotly figure object
        """
        fig = go.Figure(data=[go.Pie(labels=labels, values=values)])
        fig.update_layout(title=title)
        return fig

    @staticmethod
    def create_data_comparison_scatter(data_records, key, val_sig_map_in):
        """
        Create a scatter plot from data records with intersection highlighting.
        
        Parameters:
        - data_records: List of tuples (scan_idx, val, type) for the plot
        - key: Key identifier for the plot title and labels
        - val_sig_map_in: Dictionary mapping signal identifiers
        
        Returns:
        - fig: Plotly figure object
        """
        fig = go.Figure()
        
        # Create a DataFrame from the data records
        df = pd.DataFrame(data_records, columns=['ScanIndex', 'Values', 'Type'])

        # Define color mapping
        color_map = {'Intersection': 'purple', 'Input': 'blue', 'Output': 'red'}

        # Iterate through each type and add a scatter trace
        for trace_type, color in color_map.items():
            trace_data = df[df['Type'].str.contains(trace_type)]
            fig.add_trace(go.Scattergl(
                x=trace_data['ScanIndex'], 
                y=trace_data['Values'], 
                mode='markers', 
                name=trace_type,
                marker=dict(color=color)
            ))
        
        fig.update_layout(
            title=f'Scatter Plot of {val_sig_map_in[key]}',
            xaxis_title='ScanIndex',
            yaxis_title=f'Values of {val_sig_map_in[key]}',
            legend_title='Legend'
        )
        
        return fig

    @staticmethod
    def organize_plots_by_tabs(plots, grp_idx, key, val_sig_map_in, plots_hash):
        """
        Organize plots by parent tabs in a hash dictionary.
        
        Parameters:
        - plots: Dictionary containing generated figures/plots
        - grp_idx: Group index
        - key: Key identifier for tab name determination
        - val_sig_map_in: Dictionary mapping signal identifiers
        - plots_hash: Dictionary to store organized plots
        
        Returns:
        - plots_hash: Updated dictionary with plots organized by tabs
        """
        mapp = key
        if mapp.startswith(f"{grp_idx}_") and not mapp.endswith(f"_None"):
            tab_name = val_sig_map_in[f"{grp_idx}_None"].rsplit('/', 1)[-1]
            if tab_name not in plots_hash:
                plots_hash[tab_name] = []
            plots_hash[tab_name].append(plots[f"fig{key}"])
        
        return plots_hash

    @staticmethod
    def create_comparison_scatter(data_container_in, data_container_out, val_sig_map_in):
        """
        Create comparison scatter plots showing intersections and differences between input and output data.
        
        Parameters:
        - data_container_in: Dictionary containing input data
        - data_container_out: Dictionary containing output data
        - val_sig_map_in: Dictionary mapping signal identifiers
        
        Returns:
        - figs: Dictionary of Plotly figure objects
        """
        figs = {}
        unique_keys = set(data_container_in.keys()).union(set(data_container_out.keys()))
        inp_values = list(data_container_in.values())[0]
        
        for j in range(len(inp_values)):
            for k in range(len(inp_values[j])):
                fig_id = f"fig{j}_{k}"
                fig = go.Figure()
                
                for key, input_data, output_data in zip(unique_keys, data_container_in.values(), data_container_out.values()):
                    data_values = input_data[j][k]
                    data_out_values = output_data[j][k]
                    intersection = set(data_values).intersection(set(data_out_values))

                    # Plot intersection points
                    if intersection:
                        fig.add_trace(go.Scattergl(
                            x=[key] * len(intersection),
                            y=list(intersection),
                            mode='markers',
                            name=f'Intersec {val_sig_map_in[f"{j}_{k}"]}',
                            marker=dict(color='purple', size=10)
                        ))

                    # Plot unique data points from input
                    unique_data_values = set(data_values) - intersection
                    if unique_data_values:
                        fig.add_trace(go.Scattergl(
                            x=[key] * len(unique_data_values),
                            y=list(unique_data_values),
                            mode='markers',
                            name=f'Data {val_sig_map_in[f"{j}_{k}"]}',
                            marker=dict(color='blue')
                        ))

                    # Plot unique data points from output
                    unique_data_out_values = set(data_out_values) - intersection
                    if unique_data_out_values:
                        fig.add_trace(go.Scattergl(
                            x=[key] * len(unique_data_out_values),
                            y=list(unique_data_out_values),
                            mode='markers',
                            name=f'Data Out {val_sig_map_in[f"{j}_{k}"]}',
                            marker=dict(color='red')
                        ))

                # Update layout for each figure
                fig.update_layout(
                    title=f'Scatter Plot with {val_sig_map_in[f"{j}_{k}"]}',
                    xaxis_title='ScanIndex',
                    yaxis_title='Values',
                    legend_title='Legend'
                )
                figs[fig_id] = fig
                
        return figs
