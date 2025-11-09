"""
KPI Processing Interfaces and Abstract Base Classes
Implements SOLID principles with proper abstractions
"""
from abc import ABC, abstractmethod
from typing import Dict, List, Any, Optional
from dataclasses import dataclass
from enum import Enum
import multiprocessing as mp

class KPIType(Enum):
    """Enumeration of supported KPI types"""
    ALIGNMENT = "alignment"
    TRACKER = "tracker" 
    DETECTION = "detection"

@dataclass
class KPIResult:
    """Data class for KPI processing results"""
    kpi_type: KPIType
    plots: List[Any]
    success: bool
    error_message: Optional[str] = None
    processing_time: Optional[float] = None
    metadata: Optional[Dict[str, Any]] = None

@dataclass
class KPITaskConfig:
    """Configuration for KPI processing tasks"""
    kpi_type: KPIType
    input_data: Any
    output_data: Any
    stream_name: str
    sensor: str
    temp_dir: str
    additional_params: Optional[Dict[str, Any]] = None

class IKPIProcessor(ABC):
    """Interface for KPI processors following Single Responsibility Principle"""
    
    @abstractmethod
    def can_process(self, stream_name: str) -> bool:
        """Check if processor can handle the given stream type"""
        pass
    
    @abstractmethod
    def process(self, config: KPITaskConfig) -> KPIResult:
        """Process KPI data and return results"""
        pass
    
    @abstractmethod
    def get_kpi_type(self) -> KPIType:
        """Get the KPI type this processor handles"""
        pass

class IDataPreparer(ABC):
    """Interface for data preparation following Single Responsibility Principle"""
    
    @abstractmethod
    def prepare_data(self, input_data: Any, output_data: Any, kpi_type: KPIType) -> Dict[str, Any]:
        """Prepare data for specific KPI type"""
        pass

class IKPIFactory(ABC):
    """Interface for KPI processor factory following Open/Closed Principle"""
    
    @abstractmethod
    def create_processor(self, kpi_type: KPIType) -> IKPIProcessor:
        """Create KPI processor for given type"""
        pass
    
    @abstractmethod
    def get_supported_types(self) -> List[KPIType]:
        """Get list of supported KPI types"""
        pass

class IMultiprocessingManager(ABC):
    """Interface for multiprocessing management"""
    
    @abstractmethod
    def process_kpis_parallel(self, tasks: List[KPITaskConfig]) -> List[KPIResult]:
        """Process multiple KPI tasks in parallel"""
        pass
    
    @abstractmethod
    def set_max_workers(self, max_workers: int) -> None:
        """Set maximum number of worker processes"""
        pass

class IStreamClassifier(ABC):
    """Interface for stream type classification following Single Responsibility Principle"""
    
    @abstractmethod
    def classify_stream(self, stream_name: str) -> List[KPIType]:
        """Classify stream and return applicable KPI types"""
        pass
