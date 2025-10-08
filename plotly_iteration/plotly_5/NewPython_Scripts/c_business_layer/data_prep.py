import plotly.express as px
from d_presentation_layer.plotlyapi import PlotlyCharts
from c_business_layer.data_model import DataContainer
from b_db_layer.data_mapper import stla, stla_nav
from d_presentation_layer.plot_generator import PlotGenerator

class DataPrep:
    def __init__(self, data_container: DataContainer):
        """Initialize DataPrep with a DataContainer."""
        self.data_container = data_container
        self.parent_tabs = self.create_parent_tabs()

    def create_parent_tabs(self):
        """Create tabs based on the number of parents."""
        parent_map = self.data_container.input_parent_map
        tabs = {}

        # Initialize tabs for parents without children
        for key, val in self.data_container.input_unique_map.items():
            print(f"key, value: {key}, {val}")
            if key.endswith("_None"):
                tabs[self.data_container.input_unique_map[key]] = []

        # Populate tabs with children
        for child, parent in parent_map.items():
            print(f"Child: {child}, Parent: {parent}")
            if stla_nav[parent] in tabs:
                tabs[stla_nav[parent]].append(child)

        return tabs

    def generate_html_content(self):
        """Generate HTML content for plots."""
        plots_hash = {}

        for parent_key, children in self.parent_tabs.items():
            for child in children:
                input_key = self.data_container.input_reverse_unique_map[stla[child]]
                output_key = self.data_container.output_reverse_unique_map[stla[child]]

                # Extract prefixes and suffixes for plotting
                parent_prefix, child_suffix = input_key.split('_')
                parent_prefix_out, child_suffix_out = output_key.split('_')

                # Create scatter plot logic here...
                for input_data, output_data in zip(self.data_container.input_data, self.data_container.output_data):
                    fig = PlotlyCharts.create_scatter(
                        [len(input_data), len(input_data)],  # Adjust as necessary
                        [input_data, output_data],          # Adjust as necessary
                        title=f"Results for {child_suffix}",
                        # labels=[child_suffix, child_suffix_out],
                        # xaxis_title="Scan Index"
                    )

                    # Store the plot in the appropriate hash map
                    parent_none_key = f"{parent_prefix}_None"
                    if self.data_container.input_unique_map.get(parent_none_key) in plots_hash:
                        plots_hash[self.data_container.input_unique_map[parent_none_key]].append(fig)
                        print(f"Updated plots for: {self.data_container.input_unique_map[parent_none_key]}")
                    else:
                        plots_hash[self.data_container.input_unique_map[parent_none_key]] = [fig]
        # Create an instance of PlotGenerator with valid plots and generate HTML content
        plot_generator = PlotGenerator(plots_hash)
        return plot_generator.generate_html_content()
