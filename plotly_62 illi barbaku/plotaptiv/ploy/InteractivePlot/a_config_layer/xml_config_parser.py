"""
XML configuration parser for Interactive Plot System.

This module provides functionality to parse XML configuration files.
"""
import os
import xml.etree.ElementTree as ET
from typing import Dict, List, Any, Optional

from InteractivePlot.interfaces.base_interfaces import IConfigParser

class XmlConfigParser(IConfigParser):
    """Parser for XML configuration files."""
    
    def __init__(self, xml_file: str):
        """
        Initialize the XML parser.
        
        Args:
            xml_file: Path to the XML configuration file
        """
        self.xml_file = xml_file
        self.hdf_source_selection = None
        self.hdf_file_type = None
        self.plot_mode = {}
        self.which_costumer =""
        
    def parse(self) -> str:
        """
        Parse the XML configuration file.
        
        Returns:
            String indicating the HDF file type
        """
        if not os.path.exists(self.xml_file):
            raise FileNotFoundError(f"XML configuration file not found: {self.xml_file}")
            
        try:
            tree = ET.parse(self.xml_file)
            root = tree.getroot()
            
            # Look for the HDF_FILE element
            for element in root.findall(".//HDF_FILE"):
                return element.text.strip()
                
            # If not found, raise an exception
            raise ValueError("HDF_FILE not found in XML configuration file")
        except ET.ParseError as e:
            raise ValueError(f"Error parsing XML configuration file: {e}")
        except Exception as e:
            raise ValueError(f"Error processing XML configuration file: {e}")

    def get_hdf_source_selection(self):
        return self.hdf_source_selection

    def get_hdf_file_type(self):
        return self.hdf_file_type

    def get_plot_mode(self):
        return self.plot_mode
