import os
import tempfile
import logging
import json
import gc
from typing import Optional, List, Dict, Any
from KPI.b_data_storage.kpi_data_model_storage import KPI_DataModelStorage
from KPI.c_business_layer.alignment_matching_kpi import process_alignment_kpi
from KPI.c_business_layer.detection_matching_kpi import process_detection_kpi
from KPI.c_business_layer.tracker_matching_kpi import process_tracker_kpi
import concurrent.futures

class KpiDataModel:
    """
    Class for generating KPI plots from KPI_DataModelStorage.
    This class processes data from KPI_DataModelStorage to generate KPI visualizations.
    """
    
    def __init__(self, data: Dict[str, KPI_DataModelStorage], sensor: str):
        """
        Initialize the KPI data model with input and output data.
        
        Parameters:
            data: Dictionary containing 'input' and 'output' KPI_DataModelStorage instances
            sensor: Name of the sensor being processed
        """
        self.data = data
        self.sensor = sensor

        # Create temporary directory for plot files
        self.temp_dir = tempfile.mkdtemp(prefix="kpi_plots_temp_")
        
        # Initialize plots list
        self.plots = []
        
        # Process KPIs based on stream type
        self._process_kpis()
    
    def _process_kpis(self):
        """
        Process KPIs in parallel using multiprocessing
        Runs alignment, tracker, and detection KPIs concurrently
        """
        import multiprocessing as mp
        
        # Determine which KPIs to process based on stream name
        tasks = [
            # ("alignment", self._process_alignment_kpi),
            # ("tracker", self._process_tracker_kpi),
            ("detection", self._process_detection_kpi)
        ]
            
        if not tasks:
            logging.warning(f"No KPI processors available for stream: {self.stream_name}")
            self.plots = []
            return
            
        # Process tasks in parallel
        self.plots = []
        with concurrent.futures.ProcessPoolExecutor(max_workers=len(tasks)) as executor:
            futures = {executor.submit(func): name for name, func in tasks}
            for future in concurrent.futures.as_completed(futures):
                try:
                    self.plots.extend(future.result())
                except Exception as e:
                    logging.error(f"Error processing {futures[future]} KPI: {e}")
    
    def _process_alignment_kpi(self):
        """
        Process alignment KPIs using the alignment_matching_kpi module.
        """
        try:
            from KPI.c_business_layer.alignment_matching_kpi import process_alignment_kpi
            
            # Process alignment KPI for each sensor
            results = []
            for sensor_id, stream_name in self._get_alignment_streams():
                result = process_alignment_kpi(
                    self.data, 
                    sensor_id,
                    stream_name
                )
                results.append(result)
            
            return results
        except Exception as e:
            logging.error(f"Error processing alignment KPIs: {str(e)}")
            return []

    def _process_detection_kpi(self):
        """
        Process detection KPIs using the detection_matching_kpi module.
        """
        try:
            from KPI.c_business_layer.detection_matching_kpi import process_detection_kpi
            
            # Process detection KPI for each sensor
            results = []
            result = process_detection_kpi(
                    self.data,
                    self.sensor,
                )
            results.append(result)
            
            return results
        except Exception as e:
            logging.error(f"Error processing detection KPIs: {str(e)}")
            return []

    def _process_tracker_kpi(self):
        """
        Process tracker KPIs using the tracker_matching_kpi module.
        """
        try:
            from tracker_matching_kpi import process_tracker_kpi
            
            # Process tracker KPI for each sensor
            results = []
            for sensor_id, stream_name in self._get_tracker_streams():
                result = process_tracker_kpi(
                    self.input_data,
                    self.output_data,
                    sensor_id,
                    stream_name
                )
                results.append(result)
            
            return results
        except Exception as e:
            logging.error(f"Error processing tracker KPIs: {str(e)}")
            return []

    def _get_alignment_streams(self):
        """Get alignment streams from config"""
        return [("FC", "alignment"), ("FL", "alignment"), ("FR", "alignment")]

    def _get_detection_streams(self):
        """Get detection streams from config"""
        return [("FC", "detection"), ("FL", "detection"), ("FR", "detection")]

    def _get_tracker_streams(self):
        """Get tracker streams from config"""
        return [("FC", "tracker"), ("FL", "tracker"), ("FR", "tracker")]
