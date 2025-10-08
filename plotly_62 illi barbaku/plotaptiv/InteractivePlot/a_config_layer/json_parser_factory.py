from typing import Dict, Type
from abc import ABC, abstractmethod

class JsonParser(ABC):
    """Abstract base class for JSON parsers"""
    
    @abstractmethod
    def parse(self, json_data: Dict) -> Dict:
        """
        Parse JSON data according to specific format requirements.
        
        Args:
            json_data: Raw JSON data to parse
            
        Returns:
            Parsed data in standardized format
        """
        pass

class JsonParserFactory:
    """Factory for creating appropriate JSON parser instances"""
    
    def __init__(self):
        self._parsers: Dict[str, Type[JsonParser]] = {}
    
    def register_parser(self, parser_type: str, parser_class: Type[JsonParser]) -> None:
        """
        Register a parser class for a specific parser type.
        
        Args:
            parser_type: Identifier for the parser type
            parser_class: Parser class to register
        """
        self._parsers[parser_type] = parser_class
    
    def create_parser(self, parser_type: str) -> JsonParser:
        """
        Create a parser instance for the specified type.
        
        Args:
            parser_type: Type of parser to create
            
        Returns:
            Instance of the requested parser
            
        Raises:
            ValueError: If parser type is not registered
        """
        parser_class = self._parsers.get(parser_type)
        if not parser_class:
            raise ValueError(f"No parser registered for type: {parser_type}")
        return parser_class()
