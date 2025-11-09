"""
Refactored KPI Factory with SOLID principles and design patterns
Includes Factory Pattern, Strategy Pattern, and Multiprocessing support
"""
import logging
import multiprocessing as mp
from typing import Dict, List, Any, Optional
from concurrent.futures import ProcessPoolExecutor, as_completed
import time

from .kpi_interfaces import (
    IKPIFactory, IKPIProcessor, IMultiprocessingManager, IStreamClassifier,
    KPIType, KPIResult, KPITaskConfig
)
from .kpi_processors import AlignmentKPIProcessor, TrackerKPIProcessor, DetectionKPIProcessor

logger = logging.getLogger(__name__)

class StreamClassifier(IStreamClassifier):
    """Classifies stream names to determine applicable KPI types"""
    
    def __init__(self):
        self._classification_rules = {
            KPIType.ALIGNMENT: lambda name: "ALIGNMENT_STREAM" in name.upper(),
            KPIType.TRACKER: lambda name: "TRACKER" in name.upper() or "ROT_OBJ" in name.upper(),
            KPIType.DETECTION: lambda name: "DETECTION" in name.upper() or "DET" in name.upper()
        }
    
    def classify_stream(self, stream_name: str) -> List[KPIType]:
        """Classify stream and return applicable KPI types"""
        applicable_types = []
        
        for kpi_type, rule in self._classification_rules.items():
            if rule(stream_name):
                applicable_types.append(kpi_type)
        
        return applicable_types

class KPIProcessorFactory(IKPIFactory):
    """Factory for creating KPI processors following Open/Closed Principle"""
    
    def __init__(self):
        self._processors = {
            KPIType.ALIGNMENT: AlignmentKPIProcessor,
            KPIType.TRACKER: TrackerKPIProcessor,
            KPIType.DETECTION: DetectionKPIProcessor
        }
    
    def create_processor(self, kpi_type: KPIType) -> IKPIProcessor:
        """Create KPI processor for given type"""
        processor_class = self._processors.get(kpi_type)
        if processor_class is None:
            raise ValueError(f"No processor available for KPI type: {kpi_type}")
        
        return processor_class()
    
    def get_supported_types(self) -> List[KPIType]:
        """Get list of supported KPI types"""
        return list(self._processors.keys())
    
    def register_processor(self, kpi_type: KPIType, processor_class: type) -> None:
        """Register a new processor type (supports extension)"""
        self._processors[kpi_type] = processor_class

def _process_kpi_task_worker(task_config: KPITaskConfig) -> KPIResult:
    """Worker function for multiprocessing KPI tasks"""
    try:
        factory = KPIProcessorFactory()
        processor = factory.create_processor(task_config.kpi_type)
        return processor.process(task_config)
    except Exception as e:
        logger.error(f"Error in KPI worker process: {e}")
        return KPIResult(
            kpi_type=task_config.kpi_type,
            plots=[],
            success=False,
            error_message=str(e)
        )

class MultiprocessingKPIManager(IMultiprocessingManager):
    """Manages parallel processing of KPI tasks"""
    
    def __init__(self, max_workers: Optional[int] = None):
        self.max_workers = max_workers or mp.cpu_count()
        logger.info(f"Initialized KPI manager with {self.max_workers} workers")
    
    def set_max_workers(self, max_workers: int) -> None:
        """Set maximum number of worker processes"""
        self.max_workers = max_workers
        logger.info(f"Updated max workers to {max_workers}")
    
    def process_kpis_parallel(self, tasks: List[KPITaskConfig]) -> List[KPIResult]:
        """Process multiple KPI tasks in parallel"""
        if not tasks:
            return []
        
        logger.info(f"Starting parallel processing of {len(tasks)} KPI tasks")
        start_time = time.time()
        
        results = []
        
        with ProcessPoolExecutor(max_workers=self.max_workers) as executor:
            # Submit all tasks
            future_to_task = {
                executor.submit(_process_kpi_task_worker, task): task 
                for task in tasks
            }
            
            # Collect results as they complete
            for future in as_completed(future_to_task):
                task = future_to_task[future]
                try:
                    result = future.result()
                    results.append(result)
                    
                    if result.success:
                        logger.info(f"Completed {result.kpi_type.value} KPI in {result.processing_time:.2f}s")
                    else:
                        logger.error(f"Failed {result.kpi_type.value} KPI: {result.error_message}")
                        
                except Exception as e:
                    logger.error(f"Exception in KPI task {task.kpi_type.value}: {e}")
                    results.append(KPIResult(
                        kpi_type=task.kpi_type,
                        plots=[],
                        success=False,
                        error_message=str(e)
                    ))
        
        total_time = time.time() - start_time
        successful_tasks = sum(1 for r in results if r.success)
        logger.info(f"Completed {successful_tasks}/{len(tasks)} KPI tasks in {total_time:.2f}s")
        
        return results

class RefactoredKpiDataModel:
    """
    Refactored KPI Data Model using SOLID principles and design patterns
    - Single Responsibility: Each component has one clear purpose
    - Open/Closed: Easy to extend with new KPI types
    - Liskov Substitution: All processors implement the same interface
    - Interface Segregation: Separate interfaces for different concerns
    - Dependency Inversion: Depends on abstractions, not concretions
    """
    
    def __init__(self, input_data, output_data, sensor: str, stream_name: str, 
                 temp_dir: str, max_workers: Optional[int] = None):
        """Initialize the refactored KPI data model"""
        self.input_data = input_data
        self.output_data = output_data
        self.sensor = sensor
        self.stream_name = stream_name
        self.temp_dir = temp_dir
        
        # Initialize components following Dependency Inversion Principle
        self.stream_classifier: IStreamClassifier = StreamClassifier()
        self.processor_factory: IKPIFactory = KPIProcessorFactory()
        self.multiprocessing_manager: IMultiprocessingManager = MultiprocessingKPIManager(max_workers)
        
        self.plots = []
        self.kpi_results: List[KPIResult] = []
        
        # Process KPIs
        self._process_kpis()
    
    def _process_kpis(self) -> None:
        """Process KPIs using multiprocessing and SOLID design"""
        logger.info(f"Processing KPIs for stream: {self.stream_name}")
        
        # Classify stream to determine applicable KPI types
        applicable_kpi_types = self.stream_classifier.classify_stream(self.stream_name)
        
        if not applicable_kpi_types:
            logger.warning(f"No KPI processors available for stream type: {self.stream_name}")
            return
        
        # Create task configurations for each applicable KPI type
        tasks = []
        for kpi_type in applicable_kpi_types:
            task_config = KPITaskConfig(
                kpi_type=kpi_type,
                input_data=self.input_data,
                output_data=self.output_data,
                stream_name=self.stream_name,
                sensor=self.sensor,
                temp_dir=self.temp_dir
            )
            tasks.append(task_config)
        
        # Process all tasks in parallel
        logger.info(f"Processing {len(tasks)} KPI tasks in parallel")
        self.kpi_results = self.multiprocessing_manager.process_kpis_parallel(tasks)
        
        # Collect all plots from successful results
        self.plots = []
        for result in self.kpi_results:
            if result.success:
                self.plots.extend(result.plots)
            else:
                logger.error(f"KPI processing failed for {result.kpi_type.value}: {result.error_message}")
    
    def get_processing_summary(self) -> Dict[str, Any]:
        """Get summary of KPI processing results"""
        total_tasks = len(self.kpi_results)
        successful_tasks = sum(1 for r in self.kpi_results if r.success)
        total_time = sum(r.processing_time or 0 for r in self.kpi_results)
        
        return {
            "total_tasks": total_tasks,
            "successful_tasks": successful_tasks,
            "failed_tasks": total_tasks - successful_tasks,
            "total_processing_time": total_time,
            "total_plots": len(self.plots),
            "stream_name": self.stream_name,
            "sensor": self.sensor
        }
    
    def get_failed_tasks(self) -> List[KPIResult]:
        """Get list of failed KPI processing tasks"""
        return [result for result in self.kpi_results if not result.success]
    
    def add_custom_processor(self, kpi_type: KPIType, processor_class: type) -> None:
        """Add a custom KPI processor (supports Open/Closed Principle)"""
        self.processor_factory.register_processor(kpi_type, processor_class)
        logger.info(f"Registered custom processor for {kpi_type.value}")

# Legacy compatibility layer
class KpiDataModel(RefactoredKpiDataModel):
    """Legacy compatibility layer for existing code"""
    
    def __init__(self, input_data, output_data, sensor, stream_name, max_workers: Optional[int] = None):
        # Create temporary directory for backwards compatibility
        import tempfile
        temp_dir = tempfile.mkdtemp(prefix="kpi_plots_temp_")
        
        super().__init__(input_data, output_data, sensor, stream_name, temp_dir, max_workers)
    
    def _process_alignment_kpi(self):
        """Legacy method - now handled by multiprocessing"""
        alignment_results = [r for r in self.kpi_results if r.kpi_type == KPIType.ALIGNMENT]
        return alignment_results[0].plots if alignment_results and alignment_results[0].success else []
    
    def _process_tracker_kpi(self):
        """Legacy method - now handled by multiprocessing"""
        tracker_results = [r for r in self.kpi_results if r.kpi_type == KPIType.TRACKER]
        return tracker_results[0].plots if tracker_results and tracker_results[0].success else []
    
    def _process_detection_kpi(self):
        """Legacy method - now handled by multiprocessing"""
        detection_results = [r for r in self.kpi_results if r.kpi_type == KPIType.DETECTION]
        return detection_results[0].plots if detection_results and detection_results[0].success else []
