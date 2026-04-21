import time
import logging
import os
from functools import wraps

def time_taken(func):
    """Decorator to measure and log the time taken by a function."""
    @wraps(func)
    def wrapper(*args, **kwargs):
        start_time = time.time()
        result = func(*args, **kwargs)
        end_time = time.time()
        print(f"{func.__name__} took {end_time - start_time:.2f} seconds to execute")
        return result
    return wrapper

class LoggerSetup:
    """Simple logger setup class."""
    
    def __init__(self, output_dir="html"):
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
    
    def log_to_both(self, message):
        """Log message to both console and file."""
        print(message)
        self.logger.info(message)
    
    def log_to_file_only(self, message):
        """Log message to file only."""
        self.logger.info(message) 