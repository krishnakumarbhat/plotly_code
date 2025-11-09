import time
import functools
import logging
import os
import sys

def time_taken(func):
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        start_time = time.time()
        result = func(*args, **kwargs)
        end_time = time.time()
        logging.info(f"Function {func.__name__} took {end_time - start_time:.4f} seconds to execute.")
        return result
    return wrapper

class LoggerSetup:
    def __init__(self, output_dir):
        self.output_dir = output_dir
        self.logger = None
        self.setup_logging()

    def setup_logging(self):
        os.makedirs(self.output_dir, exist_ok=True)
        logs_file = os.path.join(self.output_dir, "logs.txt")

        self.logger = logging.getLogger()
        self.logger.setLevel(logging.DEBUG)

        for handler in self.logger.handlers[:]:
            self.logger.removeHandler(handler)

        file_handler = logging.FileHandler(logs_file, mode="w")
        file_handler.setLevel(logging.DEBUG)
        file_formatter = logging.Formatter("%(asctime)s - %(levelname)s: %(message)s")
        file_handler.setFormatter(file_formatter)

        console_handler = logging.StreamHandler(sys.stdout)
        console_handler.setLevel(logging.INFO)
        console_formatter = logging.Formatter("%(message)s")
        console_handler.setFormatter(console_formatter)

        self.logger.addHandler(file_handler)
        self.logger.addHandler(console_handler)

        logging.info(f"Logging initialized. Log file: {logs_file}")
        logging.info("Interactive html version 1.0")

    def log_to_both(self, message):
        self.logger.info(message)

    def log_to_file_only(self, message):
        self.logger.debug(message)

    def log_to_console_only(self, message):
        file_handler = self.logger.handlers[0]
        self.logger.removeHandler(file_handler)
        self.logger.info(message)
        self.logger.addHandler(file_handler)