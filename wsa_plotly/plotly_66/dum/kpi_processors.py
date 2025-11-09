"""
Concrete KPI Processor Implementations
Following Single Responsibility and Open/Closed Principles
"""
import logging
import time
from typing import Dict, Any, List
from .kpi_interfaces import IKPIProcessor, KPIType, KPIResult, KPITaskConfig

# Import actual KPI processing functions
from .alignment_matching_kpi import process_alignment_kpi

logger = logging.getLogger(__name__)

class AlignmentKPIProcessor(IKPIProcessor):
    """Processor for alignment KPI calculations"""
    
    def can_process(self, stream_name: str) -> bool:
        """Check if this processor can handle alignment streams"""
        return "ALIGNMENT_STREAM" in stream_name.upper()
    
    def get_kpi_type(self) -> KPIType:
        """Return the KPI type this processor handles"""
        return KPIType.ALIGNMENT
    
    def process(self, config: KPITaskConfig) -> KPIResult:
        """Process alignment KPI data"""
        start_time = time.time()
        
        try:
            # Prepare data for alignment KPI
            data_dict = self._prepare_alignment_data(config.input_data, config.output_data)
            
            # Process alignment KPI
            plots = process_alignment_kpi(data_dict)
            
            processing_time = time.time() - start_time
            
            return KPIResult(
                kpi_type=KPIType.ALIGNMENT,
                plots=plots,
                success=True,
                processing_time=processing_time,
                metadata={"stream_name": config.stream_name, "sensor": config.sensor}
            )
            
        except Exception as e:
            logger.error(f"Error processing alignment KPI: {str(e)}")
            return KPIResult(
                kpi_type=KPIType.ALIGNMENT,
                plots=[],
                success=False,
                error_message=str(e),
                processing_time=time.time() - start_time
            )
    
    def _prepare_alignment_data(self, input_data: Any, output_data: Any) -> Dict[str, Any]:
        """Prepare data for alignment KPI processing"""
        from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage
        
        return {
            'vacs_boresight_az_nominal': DataModelStorage.get_data(input_data, output_data, 'vacs_boresight_az_nominal'),
            'vacs_boresight_el_nominal': DataModelStorage.get_data(input_data, output_data, 'vacs_boresight_el_nominal'),
            'vacs_boresight_el_estimated': DataModelStorage.get_data(input_data, output_data, 'vacs_boresight_el_estimated'),
        }

class TrackerKPIProcessor(IKPIProcessor):
    """Processor for tracker KPI calculations"""
    
    def can_process(self, stream_name: str) -> bool:
        """Check if this processor can handle tracker streams"""
        return "TRACKER" in stream_name.upper() or "ROT_OBJ" in stream_name.upper()
    
    def get_kpi_type(self) -> KPIType:
        """Return the KPI type this processor handles"""
        return KPIType.TRACKER
    
    def process(self, config: KPITaskConfig) -> KPIResult:
        """Process tracker KPI data"""
        start_time = time.time()
        
        try:
            # Prepare data for tracker KPI
            data_dict = self._prepare_tracker_data(config.input_data, config.output_data)
            
            # Process tracker KPI (placeholder - actual implementation needed)
            plots = self._process_tracker_kpi_internal(data_dict)
            
            processing_time = time.time() - start_time
            
            return KPIResult(
                kpi_type=KPIType.TRACKER,
                plots=plots,
                success=True,
                processing_time=processing_time,
                metadata={"stream_name": config.stream_name, "sensor": config.sensor}
            )
            
        except Exception as e:
            logger.error(f"Error processing tracker KPI: {str(e)}")
            return KPIResult(
                kpi_type=KPIType.TRACKER,
                plots=[],
                success=False,
                error_message=str(e),
                processing_time=time.time() - start_time
            )
    
    def _prepare_tracker_data(self, input_data: Any, output_data: Any) -> Dict[str, Any]:
        """Prepare data for tracker KPI processing"""
        from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage
        
        tracker_data = {'scan_index': DataModelStorage.get_data(input_data, output_data, 'scan_index')}
        
        # Add track-specific signals (trkID, vcs_xposn, etc.)
        max_tracks = 64  # Adjust based on your data model
        for i in range(max_tracks):
            for signal in ['trkID', 'vcs_xposn', 'vcs_yposn', 'vcs_xvel', 'vcs_yvel', 'vcs_heading', 
                          'len1', 'len2', 'wid1', 'wid2', 'f_moving']:
                signal_name = f"{signal}_{i}"
                if signal_name in input_data._signal_to_value or signal_name in output_data._signal_to_value:
                    tracker_data[signal_name] = DataModelStorage.get_data(input_data, output_data, signal_name)
        
        return tracker_data
    
    def _process_tracker_kpi_internal(self, data_dict: Dict[str, Any]) -> List[Any]:
        """Internal tracker KPI processing - placeholder for actual implementation"""
        # This would call the actual tracker_matching_kpi.main() function
        # For now, returning empty list as placeholder
        logger.info("Processing tracker KPI data")
        return []

class DetectionKPIProcessor(IKPIProcessor):
    """Processor for detection KPI calculations"""
    
    def can_process(self, stream_name: str) -> bool:
        """Check if this processor can handle detection streams"""
        return "DETECTION" in stream_name.upper() or "DET" in stream_name.upper()
    
    def get_kpi_type(self) -> KPIType:
        """Return the KPI type this processor handles"""
        return KPIType.DETECTION
    
    def process(self, config: KPITaskConfig) -> KPIResult:
        """Process detection KPI data"""
        start_time = time.time()
        
        try:
            # Prepare data for detection KPI
            data_dict = self._prepare_detection_data(config.input_data, config.output_data)
            
            # Process detection KPI (placeholder - actual implementation needed)
            plots = self._process_detection_kpi_internal(data_dict)
            
            processing_time = time.time() - start_time
            
            return KPIResult(
                kpi_type=KPIType.DETECTION,
                plots=plots,
                success=True,
                processing_time=processing_time,
                metadata={"stream_name": config.stream_name, "sensor": config.sensor}
            )
            
        except Exception as e:
            logger.error(f"Error processing detection KPI: {str(e)}")
            return KPIResult(
                kpi_type=KPIType.DETECTION,
                plots=[],
                success=False,
                error_message=str(e),
                processing_time=time.time() - start_time
            )
    
    def _prepare_detection_data(self, input_data: Any, output_data: Any) -> Dict[str, Any]:
        """Prepare data for detection KPI processing"""
        from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage
        
        detection_data = {
            'scan_index': DataModelStorage.get_data(input_data, output_data, 'scan_index'),
            'num_af_det': DataModelStorage.get_data(input_data, output_data, 'num_af_det')
        }
        
        # Add detection-specific signals (rdd_idx, ran, vel, etc.)
        max_detections = 768  # Adjust based on your data model
        for i in range(max_detections):
            for signal in ['rdd_idx', 'ran', 'vel', 'theta', 'phi', 'f_single_target', 
                          'f_superres_target', 'f_bistatic']:
                signal_name = f"{signal}_{i}"
                if signal_name in input_data._signal_to_value or signal_name in output_data._signal_to_value:
                    detection_data[signal_name] = DataModelStorage.get_data(input_data, output_data, signal_name)
        
        return detection_data
    
    def _process_detection_kpi_internal(self, data_dict: Dict[str, Any]) -> List[Any]:
        """Internal detection KPI processing - placeholder for actual implementation"""
        # This would call the actual detection_matching_kpi.main() function
        # For now, returning empty list as placeholder
        logger.info("Processing detection KPI data")
        return []







# class KPIFactory:
#     """
#     Legacy KPI Factory - maintained for backwards compatibility.
#     New code should use the refactored KPIProcessorFactory.
#     """
    
#     def __init__(self):
#         # Import the refactored factory for delegation
#         from .kpi_factory_refactored import KPIProcessorFactory, KPIType
#         self.refactored_factory = KPIProcessorFactory()
        
#     def create_kpi(self, name: str):
#         """
#         Create KPI processor by name - delegates to refactored implementation.
#         """
#         name = (name or "").lower()
        
#         # Map legacy names to new KPI types
#         name_mapping = {
#             "alignment": "KPIType.ALIGNMENT",
#             "tracker": "KPIType.TRACKER", 
#             "detection": "KPIType.DETECTION"
#         }
        
#         if name in name_mapping:
#             from .kpi_factory_refactored import KPIType
#             kpi_type = getattr(KPIType, name.upper())
#             return self.refactored_factory.create_processor(kpi_type)
#         else:
#             raise ValueError(f"Unknown KPI type: {name}")
