import os
import psutil
import multiprocessing as mp
from data_fetcher import DataFetcher
from plot_generator import PlotGenerator

class Application:
    def save_html(self, output_directory: str, hdf_name: str, html_content: str) -> None:
        """Save the generated HTML content to a file."""
        output_filename = os.path.join(output_directory, f'{hdf_name}.html')
        
        with open(output_filename, 'w') as f:
            f.write(html_content)
            print(f"HTML file '{output_filename}' has been created.")

    def process_file(self, hdf_name: str, output_directory: str) -> None:
        """Process a single HDF5 file and save its plot as HTML."""
        plot_generator = PlotGenerator(hdf_name)
        html_content = plot_generator.generate_html_content()
        hdf_base_name = os.path.splitext(os.path.basename(hdf_name))[0]
        self.save_html(output_directory, hdf_base_name, html_content)
        
    def run(self) -> None:
        """Main method to run the application."""
        output_directory = 'html_folder'
        os.makedirs(output_directory, exist_ok=True)

        # Creating shared queue for communication between processes
        self.hdf_queue = mp.Manager().Queue()
        fetcher = DataFetcher(self.hdf_queue)
        fetcher_process = mp.Process(target=fetcher.run)
        fetcher_process.start()

        # Determine number of processes to use based on CPU cores
        physical_cores = psutil.cpu_count(logical=False)
        cpu_utilization = psutil.cpu_percent(interval=1)
        free_cores = physical_cores - (physical_cores * (cpu_utilization / 100))
        num_processes_to_use = max(1, min(int(free_cores) - 1, 10))
        print(f"Available cores: {free_cores}, Processes to use: {num_processes_to_use}")
        
        self.process_files_in_parallel(output_directory, num_processes_to_use)

    def process_files_in_parallel(self, output_directory: str, num_processes: int) -> None:
        """Process HDF files in parallel using a pool of workers."""
        with mp.Pool(processes=num_processes) as pool:
            try:
                while True:
                    hdf_name = self.hdf_queue.get()  # Wait for data from the queue
                    pool.apply_async(self.process_file, args=(hdf_name, output_directory))
                    
            except KeyboardInterrupt:
                print("Shutting down receiver...")
            
            finally:
                pool.close()
                pool.join()

if __name__ == "__main__":
    # Timer.start('Application as whole')
    app = Application()
    app.run()
    # Timer.stop('Application as whole')

    # Debug mode section for running Selenium tests
    if hasattr(app, 'debug_mode') and DataFetcher.debug_mode:
        from timer import Timer
        
        Timer.start('Run Selenium Tests')
        
        html_files = sorted(os.listdir('html_folder'))
        
        if len(html_files) >= 2:  # Ensure there are at least two files to test
            last_html_file_relative_path = os.path.join('html_folder', html_files[-2])
            last_html_file_absolute_path = os.path.abspath(last_html_file_relative_path)
            print(f"Absolute path to HTML file: {last_html_file_absolute_path}")
            
            from test_web import SeleniumTester
            selenium_tester_instance = SeleniumTester(last_html_file_absolute_path)
            selenium_tester_instance.run_test()
        
        Timer.stop('Run Selenium Tests')
