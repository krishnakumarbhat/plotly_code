import pytest
from InteractivePlot.main import MainProcessor

def test_main_processor_initialization():
    processor = MainProcessor("test.xml", "test.json")
    assert processor.xml_file == "test.xml"
    assert processor.json_file == "test.json"

def test_main_processor_with_real_files(temp_xml_file, temp_json_file):
    processor = MainProcessor(temp_xml_file, temp_json_file)
    try:
        processor.run()
    except Exception as e:
        pytest.fail(f"MainProcessor.run() raised {e} unexpectedly!")

def test_main_processor_without_files():
    processor = MainProcessor()
    assert processor.xml_file is None
    assert processor.json_file is None

def test_main_processor_with_invalid_files():
    processor = MainProcessor("nonexistent.xml", "nonexistent.json")
    with pytest.raises(Exception):
        processor.run()
