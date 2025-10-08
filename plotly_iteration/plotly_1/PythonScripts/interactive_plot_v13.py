import pandas as pd
import plotly.express as px
import os
from datetime import datetime
from front_end import html_template

class DataFetcher:
    """Class responsible for fetching and processing CSV data."""
    
    @staticmethod
    def read_csv_paths(file_path):
        """Read CSV file paths from logs.txt."""
        try:
            with open(file_path, 'r') as f:
                return [line.strip().strip('"') for line in f.readlines()]
        except Exception as e:
            print(f"Error reading CSV paths: {e}")
            return []

    @staticmethod
    def process_csv_file(csv_path):
        """Process a single CSV file and return processed data."""
        if Application.debug_mode:
            from test_web import Timer
            Timer.start('Process CSV File')
        
        df = pd.read_csv(csv_path)
        processed_data = {key: [] for key in ['Vehicle_Info', 'Timing_Info', 'Alignment', 'Detection', 'Tracker']}
        
        for i in range(5):  # From ran_0 to ran_4, vel_0 to vel_4, pow_0 to pow_4
            if all(f'{col}_{i}' in df.columns for col in ['ran', 'vel', 'pow']):
                temp_df = df[['scan_index', f'ran_{i}', f'vel_{i}', f'pow_{i}']]
                processed_data[list(processed_data.keys())[i]].append(temp_df)
        
        if Application.debug_mode:
            Timer.stop('Process CSV File')
        
        return {key: pd.concat(value, ignore_index=True) if value else pd.DataFrame() 
                for key, value in processed_data.items()}

class PlotGenerator:
    """Class responsible for generating plots based on fetched data."""
    
    def __init__(self, csv_path):
        self.processed_data = DataFetcher.process_csv_file(csv_path)
        if Application.debug_mode:
            from test_web import Timer
            Timer.start('Create Plots')
        
        self.plots = self.create_plots()
        
        if Application.debug_mode:
            Timer.stop('Create Plots')

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
                # # Animation Plot
                # plot_types[key][f'animation_{k}'] = px.scatter(
                #     self.processed_data[key],
                #     x=self.processed_data[key][f'{plot_t[k]}_{i}'].abs(),  # Take absolute value
                #     y=self.processed_data[key][f'{plot_t[(k + 1) % 3]}_{i}'].abs(),  # Take absolute value
                #     animation_frame='scan_index',
                #     size=self.processed_data[key][f'{plot_t[(k + 2) % 3]}_{i}'].abs(),  # Take absolute value
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

class Application:
    debug_mode = False
    
    def __init__(self):
        mode = input("Enter mode \n 1 to debug \n 2 to release: ").strip().lower()
        
        Application.debug_mode = mode != '2'

        if self.debug_mode:
            from test_web import Timer
            print("Debug mode activated.")
            Timer.start('Application Initialization')
            
        csv_paths = DataFetcher.read_csv_paths('logs.txt')
        
        for csv_path in csv_paths:
            csv_name = os.path.splitext(os.path.basename(csv_path))[0]
            self.plot_generator = PlotGenerator(csv_path)
            self.run(csv_name)
            
        if self.debug_mode:
            Timer.stop('Application Initialization')
    def run(self, csv_name):
        """Create the output directory and save the generated HTML content."""
        
        output_directory = 'html_folder'
        
        os.makedirs(output_directory, exist_ok=True)

        html_content = self.plot_generator.generate_html_content()
        output_filename = os.path.join(output_directory, f'plot_{csv_name}.html')
        
        with open(output_filename, 'w') as f:
            f.write(html_content)
            print(f"HTML file '{output_filename}' has been created.")

if __name__ == "__main__":
    app = Application()
    
    if app.debug_mode:
        from test_web import Timer, SeleniumTester
        Timer.start('Run Selenium Tests')
        
        # Use the second last HTML file created
        html_files = sorted(os.listdir('html_folder'))
        
        if len(html_files) >= 2:
            last_html_file_relative_path = os.path.join('html_folder', html_files[-2])
            last_html_file_absolute_path = os.path.abspath(last_html_file_relative_path)  # Convert to absolute path
            
            print(f"Absolute path to HTML file: {last_html_file_absolute_path}")  # Debugging output
            
            selenium_tester_instance = SeleniumTester(last_html_file_absolute_path)
            selenium_tester_instance.run_test()
        
        Timer.stop('Run Selenium Tests')
