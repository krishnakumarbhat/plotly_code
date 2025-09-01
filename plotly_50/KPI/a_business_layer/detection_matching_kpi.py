from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage
from KPI.b_presentation_layer.kpi_visualization import KpiVisualization
import numpy as np

class KpiDataModel:
    def __init__(self, input_data, output_data, signal_to_value_map_in, signal_to_value_map_out):
        self.input_data = input_data
        self.output_data = output_data
        self.signal_to_value_map_in = signal_to_value_map_in
        self.signal_to_value_map_out = signal_to_value_map_out
        self.plots = self.create_html(input_data, output_data, signal_to_value_map_in, signal_to_value_map_out)

    def create_html(self, input_data, output_data, signal_to_value_map_in, signal_to_value_map_out):
        """
        Generate KPI plots from input and output data.
        
        Parameters:
            input_data: Input data storage
            output_data: Output data storage
            signal_to_value_map_in: Mapping of signals to values for input data
            signal_to_value_map_out: Mapping of signals to values for output data
        
        Returns:
            dict: Dictionary containing KPI plots
        """
        plots = {}
        
        # Get the rdd_idx data
        # rdd_data_in = input_data.get_value_from_data_container('rdd_idx')
        # rdd_data_out = output_data.get_value_from_data_container('rdd_idx')
        
        # Create a scatter plot for KPI
        # First, prepare the data in the format expected by KpiVisualization
        data_records = {
            'I': [],  # Mismatched input values
            'O': [],  # Mismatched output values
            'M': []   # Matching values
        }
        
        # For demonstration, let's create some dummy data
        # In a real implementation, you would compare rdd_data_in and rdd_data_out
        # and populate the data_records dictionary with actual matches and mismatches
        
        # Create dummy scan indices and values
        scan_indices = range(100)
        input_values = np.random.normal(0, 1, 100)
        output_values = input_values + np.random.normal(0, 0.1, 100)  # Add some noise
        
        # Create matching pairs (for demonstration)
        for idx, (in_val, out_val) in enumerate(zip(input_values, output_values)):
            if abs(in_val - out_val) < 0.5:  # Consider as match if difference is small
                data_records['M'].append([idx, in_val])
            else:
                data_records['I'].append([idx, in_val])
                data_records['O'].append([idx, out_val])
        
        # Create KPI scatter plot
        scatter_plot = KpiVisualization.kpi_scatter_plot(
            data_records,
            signal_name="Detection Matching",
            sensor_position="KPI"
        )
        
        # Create KPI pie chart
        pie_chart = KpiVisualization.kpi_pie_chart(
            data_records,
            signal_name="Detection Matching",
            sensor_position="KPI"
        )
        
        # Add both plots to the plots dictionary
        plots = [scatter_plot, pie_chart]
        
        return plots