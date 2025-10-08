"""
Parallel processing implementation for Interactive Plot System.
This module provides multiprocessing capabilities to speed up data processing.
"""
import multiprocessing as mp
from concurrent.futures import ProcessPoolExecutor, ThreadPoolExecutor
from typing import Dict, List, Any, Callable, Tuple, Optional
import os
import time
import functools

from InteractivePlot.interfaces.base_interfaces import IProgressReporter, IProgressObserver


# Define standalone functions for multiprocessing to avoid pickling issues
def _process_with_progress_wrapper(args):
    """Wrapper function for processing a file with progress tracking."""
    file_path, file_idx, total_files, process_func = args
    result = process_func(file_path)
    progress = (file_idx + 1) / total_files * 100
    return result, progress, file_idx, file_path


def _process_sensor_wrapper(args):
    """Wrapper function for processing a sensor with progress tracking."""
    sensor, sensor_idx, total_sensors, process_func, sensor_data = args
    start_time = time.time()
    result = process_func(sensor, sensor_data)
    elapsed_time = time.time() - start_time
    progress = (sensor_idx + 1) / total_sensors * 100
    return sensor, result, progress, sensor_idx, elapsed_time


def _generate_report_wrapper(args):
    """Wrapper function for generating a report with progress tracking."""
    data, report_idx, total_reports, generate_func = args
    visualizations, output_file, output_dir = data
    start_time = time.time()
    result = generate_func(visualizations, output_file, output_dir)
    elapsed_time = time.time() - start_time
    progress = (report_idx + 1) / total_reports * 100
    return result, progress, report_idx, output_file, elapsed_time


class ParallelProcessor:
    """
    Handles parallel processing of data using multiprocessing.
    
    This class provides methods to parallelize various processing tasks
    to improve overall performance.
    """
    
    def __init__(self, max_workers: Optional[int] = None):
        """
        Initialize the parallel processor.
        
        Args:
            max_workers: Maximum number of worker processes. If None, uses CPU count.
        """
        self.max_workers = max_workers or mp.cpu_count()
        
    def process_files_parallel(self, 
                              files: List[str], 
                              process_func: Callable[[str], Any],
                              progress_reporter: Optional[IProgressReporter] = None) -> List[Any]:
        """
        Process multiple files in parallel.
        
        Args:
            files: List of file paths to process
            process_func: Function to apply to each file
            progress_reporter: Optional progress reporter
            
        Returns:
            List of results from processing each file
        """
        results = []
        total_files = len(files)
        
        # Create arguments for each file
        args_list = [
            (file_path, idx, total_files, process_func)
            for idx, file_path in enumerate(files)
        ]
        
        # Use a regular multiprocessing Pool to avoid the pickling issue
        with mp.Pool(processes=self.max_workers) as pool:
            # Process files in parallel
            for result, progress, idx, file_path in pool.imap_unordered(_process_with_progress_wrapper, args_list):
                if progress_reporter:
                    progress_reporter.notify_observers(
                        progress=progress,
                        message=f"Processed file {idx + 1}/{total_files}: {os.path.basename(file_path)}"
                    )
                results.append(result)
                    
        return results
        
    def process_sensors_parallel(self,
                               sensors: List[str],
                               sensor_data: Dict[str, Any],
                               process_func: Callable[[str, Dict[str, Any]], Any],
                               progress_reporter: Optional[IProgressReporter] = None) -> Dict[str, Any]:
        """
        Process multiple sensors in parallel.
        
        Args:
            sensors: List of sensor names
            sensor_data: Dictionary containing data for all sensors
            process_func: Function to process each sensor
            progress_reporter: Optional progress reporter
            
        Returns:
            Dictionary mapping sensors to processed results
        """
        results = {}
        total_sensors = len(sensors)
        
        # If we only have one sensor or a small number, process sequentially to avoid overhead
        if total_sensors <= 1 or self.max_workers <= 1:
            for idx, sensor in enumerate(sensors):
                start_time = time.time()
                result = process_func(sensor, sensor_data)
                elapsed_time = time.time() - start_time
                
                if progress_reporter:
                    progress = (idx + 1) / total_sensors * 100
                    progress_reporter.notify_observers(
                        progress=progress,
                        message=f"Processed sensor {idx + 1}/{total_sensors}: {sensor} ({elapsed_time:.2f}s)"
                    )
                results[sensor] = result
            return results
            
        # Create arguments for each sensor
        args_list = [
            (sensor, idx, total_sensors, process_func, sensor_data)
            for idx, sensor in enumerate(sensors)
        ]
        
        # Use sequential processing to avoid pickling issues
        for sensor, idx in zip(sensors, range(len(sensors))):
            start_time = time.time()
            result = process_func(sensor, sensor_data)
            elapsed_time = time.time() - start_time
            
            if progress_reporter:
                progress = (idx + 1) / total_sensors * 100
                progress_reporter.notify_observers(
                    progress=progress,
                    message=f"Processed sensor {idx + 1}/{total_sensors}: {sensor} ({elapsed_time:.2f}s)"
                )
            results[sensor] = result
                    
        return results
        
    def generate_reports_parallel(self,
                               report_data: List[Tuple[Dict[str, Any], str, str]],
                               generate_func: Callable[[Dict[str, Any], str, str], str],
                               progress_reporter: Optional[IProgressReporter] = None) -> List[str]:
        """
        Generate multiple reports in parallel.
        
        Args:
            report_data: List of tuples containing (visualizations, output_file, output_dir)
            generate_func: Function to generate each report
            progress_reporter: Optional progress reporter
            
        Returns:
            List of paths to generated reports
        """
        results = []
        total_reports = len(report_data)
        
        # If only a few reports, process sequentially
        for idx, data in enumerate(report_data):
            visualizations, output_file, output_dir = data
            start_time = time.time()
            result = generate_func(visualizations, output_file, output_dir)
            elapsed_time = time.time() - start_time
            
            if progress_reporter:
                progress = (idx + 1) / total_reports * 100
                progress_reporter.notify_observers(
                    progress=progress,
                    message=f"Generated report {idx + 1}/{total_reports}: {output_file} ({elapsed_time:.2f}s)"
                )
            results.append(result)
                    
        return results 