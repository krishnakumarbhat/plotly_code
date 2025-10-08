import os
import time
import psutil
import csv
from datetime import datetime
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC

class Timer:
    """Singleton class for tracking execution time and resource usage."""

    _instance = None
    timings = {}

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super(Timer, cls).__new__(cls)
        return cls._instance
    
    @classmethod
    def start(cls, function_name):
        """Start timing a function."""
        start_time = time.time()
        init_mem_usage = psutil.Process().memory_info().rss / (1024 * 1024)  # Memory in MB
        init_cpu_usage = psutil.cpu_percent(interval=None)
        date_now = datetime.now().strftime('%Y-%m-%d')
        time_now = datetime.now().strftime('%H:%M:%S')

        cls.timings[function_name] = {
            'start_time': start_time,
            'date': date_now,
            'time': time_now,
            'duration': None,
            'init_mem_usage': init_mem_usage,
            'init_cpu_usage': init_cpu_usage,
            'end_mem_usage': None,
            'end_cpu_usage': None,
            'memory_usage': None,
            'cpu_usage': None,
            'browser_memory_usage': None
        }

    @classmethod
    def stop(cls, function_name, browser_pid=None):
        """Stop the timer and record duration and resource usage."""
        if function_name in cls.timings:
            end_time = time.time()
            timing_info = cls.timings[function_name]
            timing_info['duration'] = end_time - timing_info['start_time']
            end_cpu_usage = psutil.cpu_percent()
            end_mem_usage = psutil.Process().memory_info().rss / (1024 * 1024)

            timing_info['end_cpu_usage'] = end_cpu_usage
            timing_info['end_mem_usage'] = end_mem_usage
            timing_info['cpu_usage'] = end_cpu_usage - timing_info['init_cpu_usage']
            timing_info['memory_usage'] = end_mem_usage - timing_info['init_mem_usage']
            if browser_pid is not None:
                try:
                    browser_process = psutil.Process(browser_pid)
                    timing_info['browser_memory_usage'] = browser_process.memory_info().rss / (1024 * 1024)
                except psutil.NoSuchProcess:
                    timing_info['browser_memory_usage'] = None

    @classmethod
    def save_runtime_log(cls):
        """Save the runtime log to a CSV file, appending data if the file exists."""
        file_exists = os.path.isfile('runtime_log.csv')
        
        with open('runtime_log.csv', 'a', newline='') as csvfile:
            fieldnames = ['Function Name', 'Date', 'Time', 
                          'Duration (s)', 
                          'End Memory Usage (MB)', 
                          'Initial Memory Usage (MB)',  
                          'CPU Usage (%)',  
                          'Browser Memory Usage (MB)']
            
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)

            if not file_exists:
                writer.writeheader()

            for func, data in cls.timings.items():
                # for _ in range(5):  # Create 5 rows for each function entry
                    writer.writerow({
                        'Function Name': func,
                        'Date': data['date'],
                        'Time': data['time'],
                        'Duration (s)': data['duration'],
                        'End Memory Usage (MB)': data['end_mem_usage'],
                        'Initial Memory Usage (MB)': data['init_mem_usage'],
                        'CPU Usage (%)': data['cpu_usage'],
                        'Browser Memory Usage (MB)': data['browser_memory_usage']
                    })

    @classmethod
    def print_timings(cls):
        """Print all recorded timings."""
        for function, data in cls.timings.items():
            # duration = data.get('duration', 0)
            # cpu_usage = data.get('cpu_usage', 0)
            # browser_memory_usage = data.get('browser_memory_usage', 0)
            
            duration = data['duration'] if data['duration'] is not None else 0  # Default to 0 if None
            cpu_usage = data['cpu_usage'] if data['cpu_usage'] is not None else 0  # Default to 0 if None
            browser_memory_usage = data['browser_memory_usage'] if data['browser_memory_usage'] is not None else 0  # Default to 0 if None
            
            
            print(f"Function: {function}, Duration: {duration:.2f}s, "
                  f"CPU Usage: {cpu_usage}%, "
                  f"Browser Memory Usage: {browser_memory_usage} MB")
            
class SeleniumTester:
    """Class for testing HTML files using Selenium."""
    def __init__(self, html_file_path):
        """Initialize SeleniumTester with the path to the HTML file."""
        self.html_file_path = html_file_path
        self.driver = None 
        self.timer = Timer()  # Use singleton Timer instance

    def initialize_driver(self):
        """Initialize Chrome WebDriver with options."""
        options = webdriver.ChromeOptions()
        
        options.add_argument('--headless')
        options.add_argument('--no-sandbox')
        options.add_argument('--disable-dev-shm-usage')
        
        self.driver = webdriver.Chrome(options=options)
        
        return self.driver.service.process.pid  

    def open_html_file(self):
        """Open the HTML file in the browser and measure load time."""
        
        browser_pid = self.initialize_driver()  # this gets PID when driver is initialized
        
        Timer.start('Open Browser')
        
        print(f"Opening HTML file at: file:///{self.html_file_path}")  # Debugging output
        self.driver.get(f'file:///{self.html_file_path}')
        
        try:
            # this  Wait's until tab elements are present before proceeding.
            WebDriverWait(self.driver, 20).until(EC.presence_of_element_located((By.CLASS_NAME, "tab")))  # Increased timeout
            print("Tabs found successfully.")  # Debugging output
        except Exception as e:
            print(f"Timeout while waiting for tabs to load: {e}")  # Debugging output
            raise
        
        Timer.stop('Open Browser', browser_pid)

    def measure_tab_navigation_time(self):
        """Measure time taken to navigate through all tabs."""
        tab_elements = self.driver.find_elements(By.CLASS_NAME, "tab")
        
        for tab in tab_elements:
            tab_id = tab.text.lower().replace(" ", "-")
            
            Timer.start(f'Navigate to {tab.text}')
            
            tab.click()

            # this Wait's until the corresponding content is visible.
            WebDriverWait(self.driver, 10).until(EC.visibility_of_element_located((By.ID, tab_id)))
            
            Timer.stop(f'Navigate to {tab.text}')  

            # this Log memory usage and time into CSV
            timing_info = Timer.timings[f'Navigate to {tab.text}']
            with open('runtime_log.csv', 'a', newline='') as csvfile:
                fieldnames = ['Function Name', 'Duration (s)', 'Memory Usage (MB)', 'CPU Usage (%)']
                writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
                writer.writerow({
                    'Function Name': f'Navigate to {tab.text}',
                    'Duration (s)': timing_info['duration'],
                    'Memory Usage (MB)': timing_info['memory_usage'],
                    'CPU Usage (%)': timing_info['cpu_usage']
                })

    def run_test(self):
        """Run the full Selenium test sequence."""
        try:
            # this is to Open HTML file and measure navigation time.
            self.open_html_file()
            self.measure_tab_navigation_time()
            
        finally:
            # this Ensure driver quits after tests.
            if self.driver:
                self.driver.quit()

        Timer.print_timings()  
        Timer.save_runtime_log()

if __name__ == "__main__":
    tester = SeleniumTester("path_to_your_html_file.html")  # if you want to run this indepently then give this input
    tester.run_test()
