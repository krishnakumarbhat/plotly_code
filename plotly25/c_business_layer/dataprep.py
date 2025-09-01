# dataprep.py

from b_db_layer.data_model import DataContainer
from b_db_layer.plotlyapi import PlotlyCharts
from b_db_layer.plotgenrator import PlotGenerator

def prepare_data_and_generate_plots(data_container: DataContainer):
    # Extract data from the DataContainer
    parsed_data = data_container.parsed_data
    uni_map = data_container.uni_map
    rev_uni_map = data_container.rev_uni_map

    # Prepare data for plotting
    x_data = []
    y_data = []
    
    # Example: Creating scatter plots for each pair in uni_map
    for key, value in uni_map.items():
        if 'ScanIndex' in value:  # Assuming ScanIndex is part of the keys
            x_data.append(parsed_data[key])
            y_data.append(parsed_data[rev_uni_map[value]])

    # Generate plots using PlotlyCharts
    for x, y in zip(x_data, y_data):
        scatter_plot = PlotlyCharts.create_scatter(x=x, y=y, title=f'Scatter Plot of {key} vs {value}')
        plot_generator = PlotGenerator('example_hdf_file')  # Replace with actual HDF file name
        plot_generator.plots['Scatter Plot'][f'{key} vs {value}'] = scatter_plot

    # Generate HTML content for the plots
    html_content = plot_generator.generate_html_content()
    
    # Save HTML file (specify your output directory)
    output_directory = './output'
    plot_generator.save_html(output_directory, 'example_hdf_file', html_content)

