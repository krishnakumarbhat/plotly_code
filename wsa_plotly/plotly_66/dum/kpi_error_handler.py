"""
Comprehensive Error Handling and Recovery for KPI Processing
Implements robust error handling, retry mechanisms, and graceful degradation
"""
import logging
import traceback
import functools
import time
from typing import Dict, Any, Optional, Callable, List, Type
from dataclasses import dataclass
from enum import Enum
import multiprocessing as mp

logger = logging.getLogger(__name__)

class ErrorSeverity(Enum):
    """Error severity levels for KPI processing"""
    LOW = "low"          # Non-critical errors, processing can continue
    MEDIUM = "medium"    # Significant errors, may affect results
    HIGH = "high"        # Critical errors, processing should stop
    FATAL = "fatal"      # Unrecoverable errors

@dataclass
class KPIError:
    """Structured error information for KPI processing"""
    error_type: str
    error_message: str
    severity: ErrorSeverity
    timestamp: float
    task_id: Optional[str] = None
    stack_trace: Optional[str] = None
    recovery_suggestion: Optional[str] = None
    metadata: Optional[Dict[str, Any]] = None

class KPIErrorHandler:
    """Centralized error handling for KPI processing"""
    
    def __init__(self, max_retries: int = 3, retry_delay: float = 1.0):
        self.max_retries = max_retries
        self.retry_delay = retry_delay
        self.errors: List[KPIError] = []
        
    def record_error(self, error: Exception, severity: ErrorSeverity = ErrorSeverity.MEDIUM, 
                    task_id: Optional[str] = None, metadata: Optional[Dict[str, Any]] = None) -> KPIError:
        """Record an error with structured information"""
        
        kpi_error = KPIError(
            error_type=type(error).__name__,
            error_message=str(error),
            severity=severity,
            timestamp=time.time(),
            task_id=task_id,
            stack_trace=traceback.format_exc(),
            recovery_suggestion=self._get_recovery_suggestion(error),
            metadata=metadata or {}
        )
        
        self.errors.append(kpi_error)
        
        # Log based on severity
        if severity == ErrorSeverity.FATAL:
            logger.critical(f"FATAL ERROR in {task_id}: {error}")
        elif severity == ErrorSeverity.HIGH:
            logger.error(f"HIGH severity error in {task_id}: {error}")
        elif severity == ErrorSeverity.MEDIUM:
            logger.warning(f"MEDIUM severity error in {task_id}: {error}")
        else:
            logger.info(f"LOW severity error in {task_id}: {error}")
            
        return kpi_error
    
    def _get_recovery_suggestion(self, error: Exception) -> str:
        """Provide recovery suggestions based on error type"""
        error_type = type(error).__name__
        
        suggestions = {
            "MemoryError": "Reduce data size or increase available memory",
            "FileNotFoundError": "Verify input file paths and permissions",
            "KeyError": "Check data model for required signal names",
            "ValueError": "Validate input data format and ranges", 
            "ImportError": "Ensure all required dependencies are installed",
            "TimeoutError": "Increase processing timeout or reduce data complexity",
            "PermissionError": "Check file and directory permissions",
            "ConnectionError": "Verify network connectivity and service availability"
        }
        
        return suggestions.get(error_type, "Check logs for detailed error information")
    
    def get_error_summary(self) -> Dict[str, Any]:
        """Get summary of all recorded errors"""
        if not self.errors:
            return {"total_errors": 0, "by_severity": {}, "recent_errors": []}
        
        severity_counts = {}
        for error in self.errors:
            severity_counts[error.severity.value] = severity_counts.get(error.severity.value, 0) + 1
        
        # Get recent errors (last 10)
        recent_errors = [
            {
                "type": error.error_type,
                "message": error.error_message,
                "severity": error.severity.value,
                "task_id": error.task_id,
                "timestamp": error.timestamp
            }
            for error in self.errors[-10:]
        ]
        
        return {
            "total_errors": len(self.errors),
            "by_severity": severity_counts,
            "recent_errors": recent_errors,
            "has_fatal_errors": any(e.severity == ErrorSeverity.FATAL for e in self.errors),
            "has_critical_errors": any(e.severity in [ErrorSeverity.HIGH, ErrorSeverity.FATAL] for e in self.errors)
        }

def retry_on_failure(max_retries: int = 3, delay: float = 1.0, 
                    backoff_factor: float = 2.0, 
                    exceptions: tuple = (Exception,)):
    """Decorator for retry logic with exponential backoff"""
    
    def decorator(func: Callable) -> Callable:
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            last_exception = None
            current_delay = delay
            
            for attempt in range(max_retries + 1):
                try:
                    return func(*args, **kwargs)
                except exceptions as e:
                    last_exception = e
                    
                    if attempt == max_retries:
                        logger.error(f"Function {func.__name__} failed after {max_retries} retries: {e}")
                        raise e
                    
                    logger.warning(f"Attempt {attempt + 1} failed for {func.__name__}: {e}. Retrying in {current_delay}s...")
                    time.sleep(current_delay)
                    current_delay *= backoff_factor
            
            raise last_exception
        
        return wrapper
    return decorator

def safe_execution(func: Callable, *args, error_handler: Optional[KPIErrorHandler] = None, 
                  task_id: Optional[str] = None, default_return=None, **kwargs):
    """Execute function with comprehensive error handling"""
    
    if error_handler is None:
        error_handler = KPIErrorHandler()
    
    try:
        return func(*args, **kwargs)
    except MemoryError as e:
        error_handler.record_error(e, ErrorSeverity.HIGH, task_id)
        logger.error(f"Memory error in {task_id}: {e}")
        return default_return
    except (FileNotFoundError, PermissionError) as e:
        error_handler.record_error(e, ErrorSeverity.MEDIUM, task_id)
        logger.warning(f"File system error in {task_id}: {e}")
        return default_return
    except (KeyError, ValueError, TypeError) as e:
        error_handler.record_error(e, ErrorSeverity.MEDIUM, task_id)
        logger.warning(f"Data error in {task_id}: {e}")
        return default_return
    except ImportError as e:
        error_handler.record_error(e, ErrorSeverity.HIGH, task_id)
        logger.error(f"Import error in {task_id}: {e}")
        return default_return
    except Exception as e:
        error_handler.record_error(e, ErrorSeverity.HIGH, task_id)
        logger.error(f"Unexpected error in {task_id}: {e}")
        return default_return

class ProcessSafetyManager:
    """Manages process safety and cleanup for multiprocessing"""
    
    def __init__(self, timeout_seconds: int = 300):
        self.timeout_seconds = timeout_seconds
        self.active_processes: Dict[str, mp.Process] = {}
    
    def register_process(self, task_id: str, process: mp.Process) -> None:
        """Register a process for monitoring"""
        self.active_processes[task_id] = process
        logger.debug(f"Registered process {process.pid} for task {task_id}")
    
    def monitor_process(self, task_id: str) -> Dict[str, Any]:
        """Monitor process health and status"""
        if task_id not in self.active_processes:
            return {"status": "not_found"}
        
        process = self.active_processes[task_id]
        
        return {
            "status": "alive" if process.is_alive() else "terminated",
            "pid": process.pid,
            "exitcode": process.exitcode
        }
    
    def terminate_process_safely(self, task_id: str, force_kill_after: float = 5.0) -> bool:
        """Safely terminate a process with graceful shutdown"""
        if task_id not in self.active_processes:
            return True
        
        process = self.active_processes[task_id]
        
        if not process.is_alive():
            return True
        
        try:
            # Try graceful termination first
            process.terminate()
            process.join(timeout=force_kill_after)
            
            # Force kill if still alive
            if process.is_alive():
                logger.warning(f"Force killing process {process.pid} for task {task_id}")
                process.kill()
                process.join(timeout=2.0)
            
            return not process.is_alive()
            
        except Exception as e:
            logger.error(f"Error terminating process for task {task_id}: {e}")
            return False
        finally:
            if task_id in self.active_processes:
                del self.active_processes[task_id]
    
    def cleanup_all_processes(self) -> Dict[str, bool]:
        """Clean up all registered processes"""
        results = {}
        
        for task_id in list(self.active_processes.keys()):
            results[task_id] = self.terminate_process_safely(task_id)
        
        return results

class GracefulDegradation:
    """Implements graceful degradation strategies for KPI processing"""
    
    @staticmethod
    def reduce_data_complexity(data: Dict[str, Any], reduction_factor: float = 0.5) -> Dict[str, Any]:
        """Reduce data complexity for memory-constrained processing"""
        reduced_data = {}
        
        for key, value in data.items():
            if isinstance(value, (list, tuple)) and len(value) > 100:
                # Sample data to reduce memory usage
                sample_size = int(len(value) * reduction_factor)
                step = len(value) // sample_size
                reduced_data[key] = value[::step]
                logger.info(f"Reduced {key} from {len(value)} to {len(reduced_data[key])} samples")
            else:
                reduced_data[key] = value
        
        return reduced_data
    
    @staticmethod
    def fallback_processing_mode(original_config: Dict[str, Any]) -> Dict[str, Any]:
        """Create fallback configuration for reduced processing"""
        fallback_config = original_config.copy()
        
        # Reduce worker count
        fallback_config['max_workers'] = min(2, fallback_config.get('max_workers', 1))
        
        # Reduce memory usage
        fallback_config['memory_limit_mb'] = fallback_config.get('memory_limit_mb', 1000) * 0.5
        
        # Increase timeouts
        fallback_config['timeout_seconds'] = fallback_config.get('timeout_seconds', 300) * 2
        
        logger.info("Switched to fallback processing mode with reduced resource usage")
        return fallback_config
