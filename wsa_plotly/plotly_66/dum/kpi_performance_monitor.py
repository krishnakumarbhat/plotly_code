"""
Performance Monitoring and Optimization for KPI Processing
Provides memory management, process monitoring, and performance metrics
"""
import psutil
import time
import gc
import logging
from typing import Dict, Any, Optional, List
from dataclasses import dataclass
from contextlib import contextmanager
import resource
import threading

logger = logging.getLogger(__name__)

@dataclass
class PerformanceMetrics:
    """Data class for performance metrics"""
    start_time: float
    end_time: Optional[float] = None
    cpu_usage_start: float = 0.0
    cpu_usage_end: float = 0.0
    memory_usage_start: float = 0.0
    memory_usage_end: float = 0.0
    peak_memory: float = 0.0
    processing_time: float = 0.0
    
    def calculate_metrics(self):
        """Calculate derived performance metrics"""
        if self.end_time:
            self.processing_time = self.end_time - self.start_time
        return {
            "processing_time": self.processing_time,
            "cpu_usage_delta": self.cpu_usage_end - self.cpu_usage_start,
            "memory_usage_delta": self.memory_usage_end - self.memory_usage_start,
            "peak_memory_mb": self.peak_memory / (1024 * 1024),
            "memory_efficiency": self.memory_usage_start / max(self.peak_memory, 1)
        }

class PerformanceMonitor:
    """Monitors performance metrics during KPI processing"""
    
    def __init__(self):
        self.process = psutil.Process()
        self.metrics: Dict[str, PerformanceMetrics] = {}
        self._monitoring_active = False
        self._monitor_thread: Optional[threading.Thread] = None
        self._peak_memory = 0.0
    
    def start_monitoring(self, task_id: str) -> PerformanceMetrics:
        """Start monitoring performance for a specific task"""
        metrics = PerformanceMetrics(
            start_time=time.time(),
            cpu_usage_start=self.process.cpu_percent(),
            memory_usage_start=self.process.memory_info().rss
        )
        self.metrics[task_id] = metrics
        
        # Start continuous monitoring for peak memory
        self._start_peak_memory_monitoring()
        
        logger.debug(f"Started performance monitoring for task: {task_id}")
        return metrics
    
    def stop_monitoring(self, task_id: str) -> Optional[PerformanceMetrics]:
        """Stop monitoring and calculate final metrics"""
        if task_id not in self.metrics:
            logger.warning(f"No monitoring started for task: {task_id}")
            return None
        
        metrics = self.metrics[task_id]
        metrics.end_time = time.time()
        metrics.cpu_usage_end = self.process.cpu_percent()
        metrics.memory_usage_end = self.process.memory_info().rss
        metrics.peak_memory = self._peak_memory
        
        self._stop_peak_memory_monitoring()
        
        performance_data = metrics.calculate_metrics()
        logger.info(f"Task {task_id} performance: {performance_data}")
        
        return metrics
    
    def _start_peak_memory_monitoring(self):
        """Start monitoring peak memory usage in background thread"""
        if not self._monitoring_active:
            self._monitoring_active = True
            self._monitor_thread = threading.Thread(target=self._monitor_peak_memory, daemon=True)
            self._monitor_thread.start()
    
    def _stop_peak_memory_monitoring(self):
        """Stop peak memory monitoring"""
        self._monitoring_active = False
        if self._monitor_thread and self._monitor_thread.is_alive():
            self._monitor_thread.join(timeout=1.0)
    
    def _monitor_peak_memory(self):
        """Monitor peak memory usage continuously"""
        while self._monitoring_active:
            try:
                current_memory = self.process.memory_info().rss
                self._peak_memory = max(self._peak_memory, current_memory)
                time.sleep(0.1)  # Check every 100ms
            except Exception as e:
                logger.error(f"Error monitoring peak memory: {e}")
                break
    
    def get_system_resources(self) -> Dict[str, Any]:
        """Get current system resource usage"""
        return {
            "cpu_count": psutil.cpu_count(),
            "memory_total": psutil.virtual_memory().total,
            "memory_available": psutil.virtual_memory().available,
            "memory_percent": psutil.virtual_memory().percent,
            "cpu_percent": psutil.cpu_percent(interval=1),
            "load_average": psutil.getloadavg() if hasattr(psutil, 'getloadavg') else None
        }

class MemoryOptimizer:
    """Optimizes memory usage during KPI processing"""
    
    def __init__(self, memory_threshold_mb: float = 1000.0):
        self.memory_threshold_bytes = memory_threshold_mb * 1024 * 1024
        self.process = psutil.Process()
        
    def check_memory_usage(self) -> Dict[str, Any]:
        """Check current memory usage"""
        memory_info = self.process.memory_info()
        return {
            "rss_mb": memory_info.rss / (1024 * 1024),
            "vms_mb": memory_info.vms / (1024 * 1024),
            "threshold_mb": self.memory_threshold_bytes / (1024 * 1024),
            "exceeds_threshold": memory_info.rss > self.memory_threshold_bytes
        }
    
    def optimize_memory(self, force: bool = False) -> Dict[str, Any]:
        """Optimize memory usage if threshold exceeded or forced"""
        memory_before = self.process.memory_info().rss
        
        if memory_before > self.memory_threshold_bytes or force:
            logger.info("Optimizing memory usage...")
            
            # Force garbage collection
            collected = gc.collect()
            
            # Clear any temporary caches
            import matplotlib.pyplot as plt
            plt.clf()
            plt.close('all')
            
            memory_after = self.process.memory_info().rss
            memory_freed = memory_before - memory_after
            
            result = {
                "memory_before_mb": memory_before / (1024 * 1024),
                "memory_after_mb": memory_after / (1024 * 1024),
                "memory_freed_mb": memory_freed / (1024 * 1024),
                "objects_collected": collected,
                "optimization_applied": True
            }
            
            logger.info(f"Memory optimization results: {result}")
            return result
        
        return {
            "optimization_applied": False,
            "memory_mb": memory_before / (1024 * 1024)
        }

@contextmanager
def performance_context(task_id: str, monitor: Optional[PerformanceMonitor] = None, 
                       optimizer: Optional[MemoryOptimizer] = None):
    """Context manager for performance monitoring and optimization"""
    if monitor is None:
        monitor = PerformanceMonitor()
    if optimizer is None:
        optimizer = MemoryOptimizer()
    
    # Start monitoring
    monitor.start_monitoring(task_id)
    
    try:
        yield monitor, optimizer
    finally:
        # Stop monitoring and optimize memory
        metrics = monitor.stop_monitoring(task_id)
        optimizer.optimize_memory()
        
        if metrics:
            performance_data = metrics.calculate_metrics()
            logger.info(f"Performance summary for {task_id}: {performance_data}")

class ProcessPoolOptimizer:
    """Optimizes process pool configuration based on system resources"""
    
    @staticmethod
    def calculate_optimal_workers(task_count: int, memory_per_task_mb: float = 200.0) -> int:
        """Calculate optimal number of workers based on system resources and task requirements"""
        
        # Get system resources
        cpu_count = psutil.cpu_count()
        memory_gb = psutil.virtual_memory().total / (1024 * 1024 * 1024)
        
        # Calculate based on CPU
        cpu_based = min(cpu_count, task_count)
        
        # Calculate based on memory
        available_memory_mb = memory_gb * 1024 * 0.8  # Use 80% of available memory
        memory_based = int(available_memory_mb / memory_per_task_mb)
        
        # Take the minimum to ensure we don't overload system
        optimal_workers = max(1, min(cpu_based, memory_based, task_count))
        
        logger.info(f"Calculated optimal workers: {optimal_workers} "
                   f"(CPU: {cpu_based}, Memory: {memory_based}, Tasks: {task_count})")
        
        return optimal_workers
    
    @staticmethod
    def get_process_pool_config(task_count: int) -> Dict[str, Any]:
        """Get optimized process pool configuration"""
        optimal_workers = ProcessPoolOptimizer.calculate_optimal_workers(task_count)
        
        return {
            "max_workers": optimal_workers,
            "mp_context": "spawn" if psutil.WINDOWS else "fork",
            "initializer": None,  # Can be set to initialize workers
            "initargs": (),
        }
