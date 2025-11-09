from abc import ABC, abstractmethod
from typing import Dict, Type
from c_data_storage.interfaces import IDataExtractor, ILogger
from d_business_layer.data_extractor import HDFDataExtractor

class DataExtractorFactory:
    """
    Factory for creating different types of data extractors.
    Follows Factory pattern and Open/Closed Principle.
    """
    
    _extractors: Dict[str, Type[IDataExtractor]] = {
        'hdf': HDFDataExtractor,
        # Can easily add more extractors here:
        # 'csv': CSVDataExtractor,
        # 'json': JSONDataExtractor,
        # 'xml': XMLDataExtractor,
    }
    
    @classmethod
    def create_extractor(cls, extractor_type: str, logger: ILogger) -> IDataExtractor:
        """
        Create a data extractor of the specified type.
        
        Args:
            extractor_type: Type of extractor to create ('hdf', 'csv', etc.)
            logger: Logger instance for the extractor
            
        Returns:
            IDataExtractor: Configured extractor instance
            
        Raises:
            ValueError: If extractor type is not supported
        """
        if extractor_type not in cls._extractors:
            supported_types = list(cls._extractors.keys())
            raise ValueError(f"Unsupported extractor type: {extractor_type}. Supported types: {supported_types}")
        
        extractor_class = cls._extractors[extractor_type]
        return extractor_class(logger)
    
    @classmethod
    def register_extractor(cls, extractor_type: str, extractor_class: Type[IDataExtractor]) -> None:
        """
        Register a new extractor type.
        
        Args:
            extractor_type: Name of the extractor type
            extractor_class: Class implementing IDataExtractor
        """
        cls._extractors[extractor_type] = extractor_class
    
    @classmethod
    def get_supported_types(cls) -> list:
        """
        Get list of supported extractor types.
        
        Returns:
            List of supported extractor type names
        """
        return list(cls._extractors.keys()) 