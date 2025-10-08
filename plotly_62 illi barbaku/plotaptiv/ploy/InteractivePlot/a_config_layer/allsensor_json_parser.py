"""
JSON parser for AllSensor configuration files.

This module provides functionality to parse JSON input files for all-sensor HDF files.
"""
import os
import json
from typing import Dict, List, Any, Optional

from InteractivePlot.interfaces.base_interfaces import IInputOutputParser

class AllsensorJSONParser(IInputOutputParser):
    """Parser for all-sensor JSON configuration files."""
    
    def __init__(self, json_file: str):
        """
        Initialize the AllSensor JSON parser.
        
        Args:
            json_file: Path to the JSON configuration file
        """
        self.json_file = json_file
        
    def get_io_map(self) -> Dict[str, str]:
        """
        Parse the JSON file to get input-output file mapping.
        
        Returns:
            Dictionary mapping input file paths to output file paths
        """
        if not os.path.exists(self.json_file):
            raise FileNotFoundError(f"JSON file not found: {self.json_file}")
            
        try:
            with open(self.json_file, 'r') as f:
                data = json.load(f)
                
            io_map = {}
            
            # Check for INPUT_HDF and OUTPUT_HDF arrays in the JSON
            if 'INPUT_HDF' in data and 'OUTPUT_HDF' in data:
                inputs = data['INPUT_HDF']
                outputs = data['OUTPUT_HDF']
                
                if len(inputs) != len(outputs):
                    print(f"Warning: Number of input files ({len(inputs)}) does not match number of output files ({len(outputs)})")
                
                # Create mapping of input to output files
                for i in range(min(len(inputs), len(outputs))):
                    input_file = inputs[i]
                    output_file = outputs[i]
                    
                    # Verify that input and output files exist
                    if not os.path.exists(input_file):
                        print(f"Warning: Input file does not exist: {input_file}")
                    if not os.path.exists(output_file):
                        print(f"Warning: Output file does not exist: {output_file}")
                    
                    io_map[input_file] = output_file
                
            return io_map
        except json.JSONDecodeError as e:
            raise ValueError(f"Error parsing JSON file: {e}")
        except Exception as e:
            raise ValueError(f"Error processing JSON file: {e}")
