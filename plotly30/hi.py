import logging
from pathlib import Path
from typing import Optional, Dict, Any
from dataclasses import dataclass

from a_config_layer.xml_config_parser import XmlConfigParser
from a_config_layer.json_parser_factory import JSONParserFactory
from b_db_layer.hdf_processor_factory import HdfProcessorFactory

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

@dataclass
class ProcessorConfig:
    """Configuration class for MainProcessor."""
    xml_file: Optional[str]
    json_file: Optional[str]
    per_sensor_json_file: Optional[str]

    def validate(self) -> None:
        """Validate configuration files exist."""
        for file_path in [self.xml_file, self.json_file, self.per_sensor_json_file]:
            if file_path and not Path(file_path).exists():
                raise FileNotFoundError(f"Configuration file not found: {file_path}")

class MainProcessor:
    """
    Main processor class that orchestrates the processing of configuration files
    and HDF data processing.
    
    Attributes:
        config (ProcessorConfig): Configuration containing paths to required files
    """
    
    def __init__(
        self,
        xml_file: Optional[str] = None,
        json_file: Optional[str] = None,
        per_sensor_json_file: Optional[str] = None
    ) -> None:
        """
        Initialize MainProcessor with configuration files.
        
        Args:
            xml_file: Path to XML configuration file
            json_file: Path to JSON configuration file
            per_sensor_json_file: Path to per-sensor JSON configuration file
        """
        self.config = ProcessorConfig(xml_file, json_file, per_sensor_json_file)
        self.logger = logging.getLogger(__name__)

    def _parse_config(self) -> tuple[str, Dict[str, Any]]:
        """
        Parse configuration files and return necessary data.
        
        Returns:
            tuple containing hdf_file_type and input_output_map
            
        Raises:
            ValueError: If parsing fails
        """
        try:
            self.config.validate()
            self.logger.info("Starting configuration parsing")
            
            config_parser = XmlConfigParser(self.config.xml_file)
            hdf_file_type = config_parser.parse()
            self.logger.debug(f"Parsed HDF file type: {hdf_file_type}")
            
            json_parser = JSONParserFactory.create_parser(
                hdf_file_type,
                self.config.json_file,
                self.config.per_sensor_json_file
            )
            input_output_map = json_parser.get_io_map()
            self.logger.debug("Successfully created input/output mapping")
            
            return hdf_file_type, input_output_map
            
        except Exception as e:
            self.logger.error(f"Error parsing configuration: {str(e)}")
            raise ValueError(f"Configuration parsing failed: {str(e)}") from e

    def run(self) -> None:
        """
        Execute the main processing workflow.
        
        Raises:
            RuntimeError: If processing fails
        """
        try:
            self.logger.info("Starting processing workflow")
            
            hdf_file_type, input_output_map = self._parse_config()
            
            factory = HdfProcessorFactory(input_output_map, hdf_file_type)
            factory.process()
            
            self.logger.info("Processing completed successfully")
            
        except Exception as e:
            self.logger.error(f"Processing failed: {str(e)}")
            raise RuntimeError(f"Processing failed: {str(e)}") from e

if __name__ == "__main__":
    try:
        processor = MainProcessor(
            xml_file=r"ConfigInteractivePlots.xml",
            json_file=r"InputsInteractivePlot.json",
            per_sensor_json_file=r"InputsPerSensorInteractivePlot.json"
        )
        processor.run()
    except Exception as e:
        logger.error(f"Application failed: {str(e)}")
        raise
