from typing import Dict, Any, Optional
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from pathlib import Path

@dataclass
class XmlConfig:
    """Configuration data parsed from XML"""
    hdf_file_type: str
    json_config_file: str
    input_directory: str
    output_directory: str
    metadata: Dict[str, Any]

class XmlConfigParser:
    """Parser for XML configuration files with validation and error handling"""
    
    REQUIRED_ELEMENTS = [
        'HDF_FILE_TYPE',
        'JSON_CONFIG_FILE',
        'INPUT_DIRECTORY',
        'OUTPUT_DIRECTORY'
    ]
    
    def __init__(self, config_path: str):
        """
        Initialize XML configuration parser.
        
        Args:
            config_path: Path to XML configuration file
        """
        self.config_path = Path(config_path)
        self.tree: Optional[ET.ElementTree] = None
        self.root: Optional[ET.Element] = None
        
    def parse(self) -> XmlConfig:
        """
        Parse and validate XML configuration file.
        
        Returns:
            Parsed configuration data
            
        Raises:
            ValueError: If configuration file is invalid or missing required elements
        """
        try:
            self._load_xml()
            self._validate_structure()
            return self._extract_config()
        except Exception as e:
            raise ValueError(f"Failed to parse XML configuration: {str(e)}")
    
    def _load_xml(self) -> None:
        """Load XML file and get root element."""
        try:
            self.tree = ET.parse(self.config_path)
            self.root = self.tree.getroot()
        except ET.ParseError as e:
            raise ValueError(f"Invalid XML format: {str(e)}")
        except FileNotFoundError:
            raise ValueError(f"Configuration file not found: {self.config_path}")
    
    def _validate_structure(self) -> None:
        """Validate XML structure and required elements."""
        if not self.root:
            raise ValueError("XML root element not found")
            
        missing_elements = [
            elem for elem in self.REQUIRED_ELEMENTS
            if self.root.find(elem) is None
        ]
        
        if missing_elements:
            raise ValueError(
                f"Missing required configuration elements: {', '.join(missing_elements)}"
            )
    
    def _extract_config(self) -> XmlConfig:
        """Extract configuration data from validated XML."""
        if not self.root:
            raise ValueError("XML root element not found")
            
        # Extract required elements
        config = XmlConfig(
            hdf_file_type=self._get_element_text('HDF_FILE_TYPE'),
            json_config_file=self._get_element_text('JSON_CONFIG_FILE'),
            input_directory=self._get_element_text('INPUT_DIRECTORY'),
            output_directory=self._get_element_text('OUTPUT_DIRECTORY'),
            metadata=self._extract_metadata()
        )
        
        # Validate paths exist
        self._validate_paths(config)
        
        return config
    
    def _get_element_text(self, element_name: str) -> str:
        """
        Get text content of an XML element.
        
        Args:
            element_name: Name of the XML element
            
        Returns:
            Text content of the element
            
        Raises:
            ValueError: If element is missing or empty
        """
        if not self.root:
            raise ValueError("XML root element not found")
            
        element = self.root.find(element_name)
        if element is None:
            raise ValueError(f"Required element not found: {element_name}")
            
        text = element.text
        if not text or not text.strip():
            raise ValueError(f"Empty value for required element: {element_name}")
            
        return text.strip()
    
    def _extract_metadata(self) -> Dict[str, Any]:
        """Extract optional metadata from XML."""
        if not self.root:
            raise ValueError("XML root element not found")
            
        metadata = {}
        metadata_elem = self.root.find('METADATA')
        
        if metadata_elem is not None:
            for child in metadata_elem:
                metadata[child.tag.lower()] = child.text.strip() if child.text else ''
                
        return metadata
    
    def _validate_paths(self, config: XmlConfig) -> None:
        """
        Validate that all file and directory paths exist.
        
        Args:
            config: Parsed configuration data
            
        Raises:
            ValueError: If any required path does not exist
        """
        # Check JSON config file exists
        json_path = Path(config.json_config_file)
        if not json_path.is_file():
            raise ValueError(f"JSON configuration file not found: {json_path}")
        
        # Check directories exist
        input_dir = Path(config.input_directory)
        if not input_dir.is_dir():
            raise ValueError(f"Input directory not found: {input_dir}")
            
        output_dir = Path(config.output_directory)
        if not output_dir.is_dir():
            raise ValueError(f"Output directory not found: {output_dir}")
            
    @staticmethod
    def create_default_config(output_path: str) -> None:
        """
        Create a default XML configuration file.
        
        Args:
            output_path: Path to save the default configuration
        """
        root = ET.Element('CONFIG')
        
        # Add required elements
        ET.SubElement(root, 'HDF_FILE_TYPE').text = 'HDF_WITH_ALLSENSOR'
        ET.SubElement(root, 'JSON_CONFIG_FILE').text = 'config.json'
        ET.SubElement(root, 'INPUT_DIRECTORY').text = 'input'
        ET.SubElement(root, 'OUTPUT_DIRECTORY').text = 'output'
        
        # Add metadata section
        metadata = ET.SubElement(root, 'METADATA')
        ET.SubElement(metadata, 'VERSION').text = '1.0'
        ET.SubElement(metadata, 'DESCRIPTION').text = 'Default configuration'
        
        # Write to file with proper formatting
        tree = ET.ElementTree(root)
        tree.write(output_path, encoding='utf-8', xml_declaration=True)
