import os
import tempfile
import logging
import json
import gc
from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage
from InteractivePlot.e_presentation_layer.plotly_visualization import PlotlyCharts
# from KPI.b_presentation_layer.kpi_visualization import KpiVisualization
# from KPI.a_business_layer.alignment_matching_kpi import process_alignment_kpi
# from KPI.a_business_layer.tracker_matching_kpi import process_tracker_kpi
# from KPI.a_business_layer.detection_matching_kpi import process_detection_kpi

class KpiDataModel:
    """
    Class for generating KPI plots from DataModelStorage.
    This class processes data from DataModelStorage to generate KPI visualizations.
    """
    
    def __init__(self, input_data, output_data, sensor, stream_name):
        """
        Initialize the KPI data model with input and output data.
        
        Parameters:
            input_data: DataModelStorage instance containing input data
            output_data: DataModelStorage instance containing output data
            sensor: Name of the sensor being processed
            stream_name: Name of the stream being processed
        """
        self.input_data = input_data
        self.output_data = output_data
        self.sensor = sensor
        self.stream_name = stream_name
        self.signal_to_value_map_in = input_data._signal_to_value
        self.signal_to_value_map_out = output_data._signal_to_value
        
        # Create temporary directory for plot files
        self.temp_dir = tempfile.mkdtemp(prefix="kpi_plots_temp_")
        
        # Initialize plots list
        self.plots = []
        
        # Process KPIs based on stream type
        self._process_kpis()
    
    def _process_kpis(self):
        """
        Process KPIs based on the stream type.
        This method determines which KPI processor to use based on the stream name.
        """
        # try:
        if "ALIGNMENT_STREAM" in self.stream_name.upper():
            self.plots = self._process_alignment_kpi()
        elif "TRACKER" in self.stream_name.upper() or "ROT_OBJ" in self.stream_name.upper():
            self.plots = self._process_tracker_kpi()
        elif "DETECTION" in self.stream_name.upper() or "DET" in self.stream_name.upper():
            self.plots = self._process_detection_kpi()
        else:
            logging.warning(f"No KPI processor available for stream type: {self.stream_name}")
        # except Exception as e:
        #     logging.error(f"Error processing KPIs: {str(e)}")
        #     self.plots = []
    
    def _process_alignment_kpi(self):
        """
        Process alignment KPIs using the alignment_matching_kpi module.
        """
        # try:
            # Extract necessary data from DataModelStorage
        data_dict = self._prepare_data_for_alignment_kpi(self.input_data,self.output_data)
        
        
        # Call the alignment KPI processor
        return process_alignment_kpi(data_dict)
        # except Exception as e:
        #     logging.error(f"Error processing alignment KPIs: {str(e)}")
        #     return []
    
    # def _process_tracker_kpi(self):
    #     """
    #     Process tracker KPIs using the tracker_matching_kpi module.
    #     """
    #     try:
    #         # Extract necessary data from DataModelStorage
    #         input_data = self._prepare_data_for_tracker_kpi(self.input_data, self.output_data)
            
    #         # Call the tracker KPI processor
    #         return process_tracker_kpi(input_data, self.temp_dir)
    #     except Exception as e:
    #         logging.error(f"Error processing tracker KPIs: {str(e)}")
    #         return []
    
    # def _process_detection_kpi(self):
    #     """
    #     Process detection KPIs using the detection_matching_kpi module.
    #     """
    #     try:
    #         # Extract necessary data from DataModelStorage
    #         input_data = self._prepare_data_for_detection_kpi(self.input_data, self.output_data)
            
    #         # Call the detection KPI processor
    #         return process_detection_kpi(input_data, self.temp_dir)
    #     except Exception as e:
    #         logging.error(f"Error processing detection KPIs: {str(e)}")
    #         return []
    
    def _prepare_data_for_alignment_kpi(self, data_model_veh, data_model_out):
        """
        Extract and prepare data for alignment KPI processing.
        """
        # Extract relevant signals from data_model using DataModelStorage.get_data
        # This would be customized based on what alignment_matching_kpi expects
        return {
            'vacs_boresight_az_nominal': DataModelStorage.get_data(data_model_veh, data_model_out, 'vacs_boresight_az_nominal'),
            # 'vacs_boresight_az_kf_internal': DataModelStorage.get_data(data_model_veh, data_model_out, 'vacs_boresight_az_kf_internal'),
            # 'vacs_boresight_az_estimated': DataModelStorage.get_data(data_model_veh, data_model_out, 'vacs_boresight_az_estimated'),
            'vacs_boresight_el_nominal': DataModelStorage.get_data(data_model_veh, data_model_out, 'vacs_boresight_el_nominal'),
            # 'vacs_boresight_el_kf_internal': DataModelStorage.get_data(data_model_veh, data_model_out, 'vacs_boresight_el_kf_internal'),
            'vacs_boresight_el_estimated': DataModelStorage.get_data(data_model_veh, data_model_out, 'vacs_boresight_el_estimated'),
        }
    
    # def _prepare_data_for_tracker_kpi(self, data_model_veh, data_model_out):
    #     """
    #     Extract and prepare data for tracker KPI processing.
    #     """
    #     # Extract relevant signals using DataModelStorage.get_data
    #     # This would be customized based on what tracker_matching_kpi expects
    #     # tracker_data = {'scan_index': DataModelStorage.get_data(data_model_veh, data_model_out, 'scan_index')}
        
    #     # Add track-specific signals (trkID, vcs_xposn, etc.)
    #     max_tracks = 64  # Adjust based on your data model
    #     for i in range(max_tracks):
    #         for signal in ['trkID', 'vcs_xposn', 'vcs_yposn', 'vcs_xvel', 'vcs_yvel', 'vcs_heading', 
    #                       'len1', 'len2', 'wid1', 'wid2', 'f_moving']:
    #             signal_name = f"{signal}_{i}"
    #             if signal_name in data_model_veh._signal_to_value or signal_name in data_model_out._signal_to_value:
    #                 tracker_data[signal_name] = DataModelStorage.get_data(data_model_veh, data_model_out, signal_name)
        
    #     return tracker_data
    
    # def _prepare_data_for_detection_kpi(self, data_model_veh, data_model_out):
    #     """
    #     Extract and prepare data for detection KPI processing.
    #     """
    #     # Extract relevant signals using DataModelStorage.get_data
    #     # This would be customized based on what detection_matching_kpi expects
    #     detection_data = {
    #         'scan_index': DataModelStorage.get_data(data_model_veh, data_model_out, 'scan_index'),
    #         'num_af_det': DataModelStorage.get_data(data_model_veh, data_model_out, 'num_af_det')
    #     }
        
    #     # Add detection-specific signals (rdd_idx, ran, vel, etc.)
    #     max_detections = 768  # Adjust based on your data model
    #     for i in range(max_detections):
    #         for signal in ['rdd_idx', 'ran', 'vel', 'theta', 'phi', 'f_single_target', 
    #                       'f_superres_target', 'f_bistatic']:
    #             signal_name = f"{signal}_{i}"
    #             if signal_name in data_model_veh._signal_to_value or signal_name in data_model_out._signal_to_value:
    #                 detection_data[signal_name] = DataModelStorage.get_data(data_model_veh, data_model_out, signal_name)
        
        # return detection_data


# class KPIFactory:
   
#    """Factory to create KPI calculators by name."""

#    def create_kpi(self, name: str):
#     name = (name or "").lower()
#     return AlignmentKPI()
#         # raise ValueError(f"Unknown KPI type: {name}")
