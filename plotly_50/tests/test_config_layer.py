import pytest
from InteractivePlot.a_config_layer.xml_config_parser import XmlConfigParser
from InteractivePlot.a_config_layer.json_parser_factory import JSONParserFactory

def test_xml_config_parser(temp_xml_file):
    parser = XmlConfigParser(temp_xml_file)
    file_type = parser.parse()
    assert file_type == "HDF5"

def test_json_parser_factory(temp_json_file):
    parser = JSONParserFactory.create_parser("HDF5", temp_json_file)
    io_map = parser.get_io_map()
    assert isinstance(io_map, dict)
    assert "inputs" in io_map
    assert "outputs" in io_map

def test_invalid_xml_file():
    with pytest.raises(Exception):
        parser = XmlConfigParser("nonexistent.xml")
        parser.parse()

def test_invalid_json_file():
    with pytest.raises(Exception):
        parser = JSONParserFactory.create_parser("HDF5", "nonexistent.json")
        parser.get_io_map()
