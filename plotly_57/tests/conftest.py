import sys
import os

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

import pytest
import json
import tempfile

@pytest.fixture
def sample_xml_config():
    return """<?xml version="1.0" encoding="UTF-8"?>
    <Configuration>
        <FileType>HDF5</FileType>
    </Configuration>"""

@pytest.fixture
def sample_json_config():
    return {
        "inputs": {
            "sensor1": "path/to/sensor1",
            "sensor2": "path/to/sensor2"
        },
        "outputs": {
            "plot1": "path/to/plot1"
        }
    }

@pytest.fixture
def temp_xml_file(sample_xml_config):
    with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.xml') as f:
        f.write(sample_xml_config)
    yield f.name
    os.unlink(f.name)

@pytest.fixture
def temp_json_file(sample_json_config):
    with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.json') as f:
        json.dump(sample_json_config, f)
    yield f.name
    os.unlink(f.name)
