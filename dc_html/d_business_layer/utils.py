import time
import logging
import os
from functools import wraps
from c_data_storage.interfaces import ILogger

def time_taken(func):
    """
    Decorator to measure and log the time taken by a function.
    Follows Single Responsibility Principle by focusing only on timing measurement.
    """
    @wraps(func)
    def wrapper(*args, **kwargs):
        start_time = time.time()
        result = func(*args, **kwargs)
        end_time = time.time()
        print(f"{func.__name__} took {end_time - start_time:.2f} seconds to execute")
        return result
    return wrapper

class LoggerSetup(ILogger):
    """
    Logger setup class implementing ILogger interface.
    Follows Single Responsibility Principle by focusing only on logging setup and operations.
    """
    
    def __init__(self, output_dir: str = "html"):
        """
        Initialize the logger setup.
        
        Args:
            output_dir: Directory for log files
        """
        self.output_dir = output_dir
        os.makedirs(output_dir, exist_ok=True)
        
        # Setup logging
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
            handlers=[
                logging.StreamHandler(),
                logging.FileHandler(os.path.join(output_dir, 'processing.log'))
            ]
        )
        self.logger = logging.getLogger(__name__)
    
    def info(self, message: str) -> None:
        """Log info message."""
        self.logger.info(message)
    
    def error(self, message: str) -> None:
        """Log error message."""
        self.logger.error(message)
    
    def warning(self, message: str) -> None:
        """Log warning message."""
        self.logger.warning(message)
    
    def log_to_both(self, message: str) -> None:
        """Log message to both console and file."""
        print(message)
        self.logger.info(message)
    
    def log_to_file_only(self, message: str) -> None:
        """Log message to file only."""
        self.logger.info(message) 