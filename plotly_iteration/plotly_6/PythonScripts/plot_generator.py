import plotly.express as px
from front_end import html_template
import tables
import pandas as pd
from data_fetcher import DataFetcher

class PlotGenerator:
    """Class responsible for generating plots based on fetched data."""
    
    def __init__(self, hdf_name: str):
        
        self.hdf_name = hdf_name
        self.processed_data = self.process_hdf_file(hdf_name)
	
        if DataFetcher.debug_mode:
            from test_web import Timer
            Timer.start('Create Plots')
        
        self.plots = self.create_plots()
        
        if DataFetcher.debug_mode:
            Timer.stop('Create Plots')
            
    # @staticmethod
    # def read_hdf_with_pytables(hdf_name):
    #     """Read data from an HDF5 file using PyTables."""
    #     with tables.open_file(hdf_name, mode='r') as h5file:
    #         print(h5file)
    #         return h5file.root.data.read() 
            
    # @staticmethod
    # def read_hdf_with_pytables(hdf_name):
    #     # Open the HDF5 file
    #     with tables.open_file(hdf_name, mode='r') as h5file:
    #         # Assuming the data is stored in a table or array format
    #         # You may need to adjust 'data' to the actual path in your HDF5 file
    #         data = h5file.root.data.read()  # Change 'data' to your dataset name
            
    #         # If you want to convert it to a DataFrame-like structure, you can use numpy
    #         return data
            
    @staticmethod
    def process_hdf_file(hdf_name: str):
        """Process a single HDF5 file and return processed data."""
        
        if DataFetcher.debug_mode:
            from test_web import Timer
            Timer.start('Process HDF File')
        
        # df = PlotGenerator.read_hdf_with_pytables(hdf_name)  
        df = pd.read_hdf(hdf_name)
        processed_data = {key: [] for key in ['Vehicle_Info', 'Timing_Info', 'Alignment', 'Detection', 'Tracker']}
        
        for i in range(5):  # From ran_0 to ran_4, vel_0 to vel_4, pow_0 to pow_4
            if all(f'{col}_{i}' in df.columns for col in ['ran', 'vel', 'pow']):
                temp_df = df[['scan_index', f'ran_{i}', f'vel_{i}', f'pow_{i}']]
                processed_data[list(processed_data.keys())[i]].append(temp_df)
        
        if DataFetcher.debug_mode:
            Timer.stop('Process HDF File')
        
        return {key: pd.concat(value, ignore_index=True) if value else pd.DataFrame() 
                for key, value in processed_data.items()}
        
    def create_plots(self):
        """Create plots for each category of data."""
        plot_types = {}
        plot_t = ["vel", "ran", "pow"]
        
        for i, key in enumerate(self.processed_data.keys()):
            plot_types[key] = {}
            for k in range(len(plot_t)):
                plot_types[key][f'scatter_{k}'] = px.scatter(
                    self.processed_data[key], 
                    x=f'{plot_t[k]}_{i}', 
                    y=f'{plot_t[(k + 1) % 3]}_{i}', 
                    title=f'Scatter Plot of {key} {i}'
                )
                plot_types[key][f'box_{k}'] = px.box(
                    self.processed_data[key], 
                    y=f'{plot_t[k]}_{i}', 
                    title=f'Box Plot of {key} {i}'
                )
                plot_types[key][f'bar_{k}'] = px.bar(
                    self.processed_data[key], 
                    x='scan_index', 
                    y=f'{plot_t[k]}_{i}', 
                    title=f'Bar Plot of {key} {i}'
                )
                plot_types[key][f'pie_{k}'] = px.pie(
                    self.processed_data[key], 
                    names=f'{plot_t[k]}_{i}', 
                    values=f'{plot_t[(k + 1) % 3]}_{i}', 
                    title=f'Pie Plot of {key} {i}'
                )
                
                # Animation Plot
                # plot_types[key][f'animation_{k}'] = px.scatter(
                #     self.processed_data[key],
                #     x=self.processed_data[key][f'{plot_t[k]}_{i}'].abs(),
                #     y=self.processed_data[key][f'{plot_t[(k + 1) % 3]}_{i}'].abs(),
                #     animation_frame='scan_index',
                #     size=self.processed_data[key][f'{plot_t[(k + 2) % 3]}_{i}'].abs(),
                #     title=f"Animated Bubble Chart for {key} {i}"
                # )
                
        return plot_types

    def generate_html_content(self):
        """Generate HTML content for the plots."""
        tabs_html = ''.join(f'<div class="tab" onclick="showTab(\'{key.lower().replace(" ", "-")}\')">{key}</div>' for key in self.plots.keys() if self.plots[key])

        content_html = ''.join(
            f'<div id="{key.lower().replace(" ", "-")}" class="tab-content">'
            f'  <table style="width:100%;">'
            + ''.join(
                f'<tr>'
                + ''.join(
                    f'<td style="width:50%;">{plot.to_html(full_html=False, include_plotlyjs=False)}</td>'
                    if idx % 2 == 0 else
                    f'<td style="width:50%;">{plot.to_html(full_html=False, include_plotlyjs=False)}</td></tr><tr>'
                    for idx, plot in enumerate(self.plots[key].values())
                )
                + '</tr>'
            )
            + '</table></div>'
            for key in self.plots.keys() if self.plots[key]
        )
        
        final_html = html_template.replace("{{TABS}}", tabs_html).replace("{{CONTENT}}", content_html)

        return final_html
