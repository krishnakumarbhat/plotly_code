from typing import List, Dict, Any
from InteractivePlot.d_business_layer.data_prep import DataPrep
from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage
import logging
import time


class DataProcessor:
    """
    Handles the processing of parsed data through DataPrep.
    This class decouples the data processing logic from the parsing logic,
    providing better separation of concerns and maintainability.
    """
    
    def __init__(self, output_dir=None):
        """
        Initialize the data processor.
        
        Args:
            output_dir: Directory to save HTML reports
        """
        self.output_dir = output_dir
        self.processed_data = []
    
    def process_sensor_stream_data(
        self,
        input_data: DataModelStorage,
        output_data: DataModelStorage,
        html_name: str,
        sensor: str,
        stream: str,
        base_name: str,
        base_name_out: str,
        generate_html: bool = True
    ) -> DataPrep:
        """
        Process a single sensor-stream combination through DataPrep.
        
        Args:
            input_data: Input data storage containing signal data
            output_data: Output data storage containing processed signal data
            html_name: Name for the HTML file to be generated
            sensor: Sensor identifier
            stream: Stream identifier
            base_name: Base name from input file
            base_name_out: Base name from output file
            generate_html: Whether to generate HTML file
            
        Returns:
            DataPrep: The processed DataPrep object
        """
        if input_data._data_container or output_data._data_container:
            start_time = time.time()
            
            data_prep = DataPrep(
                input_data,
                output_data,
                html_name,
                sensor,
                stream,
                base_name,
                base_name_out,
                self.output_dir,
                generate_html=generate_html,
            )
            
            end_time = time.time()
            logging.debug(
                f"Time taken by dataprep processing {end_time - start_time} in {stream}"
            )
            
            self.processed_data.append(data_prep)
            return data_prep
        
        return None
    
    def get_processed_data(self) -> List[DataPrep]:
        """
        Get all processed DataPrep objects.
        
        Returns:
            List[DataPrep]: List of all processed DataPrep objects
        """
        return self.processed_data
    
    def clear_processed_data(self):
        """
        Clear the processed data list to free memory.
        """
        self.processed_data.clear()