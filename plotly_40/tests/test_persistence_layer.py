import pytest
from InteractivePlot.b_persistence_layer.hdf_processor_factory import HdfProcessorFactory
import h5py
import numpy as np
import tempfile
import os

@pytest.fixture
def sample_hdf_file():
    with tempfile.NamedTemporaryFile(suffix='.h5', delete=False) as f:
        with h5py.File(f.name, 'w') as hdf:
            hdf.create_dataset('test_data', data=np.array([1, 2, 3, 4, 5]))
    yield f.name
    os.unlink(f.name)

def test_hdf_processor_factory_creation():
    input_output_map = {
        "inputs": {"sensor1": "path/to/sensor1"},
        "outputs": {"plot1": "path/to/plot1"}
    }
    factory = HdfProcessorFactory(input_output_map, "HDF5")
    assert factory is not None

def test_hdf_file_reading(sample_hdf_file):
    with h5py.File(sample_hdf_file, 'r') as hdf:
        data = hdf['test_data'][:]
        assert len(data) == 5
        assert np.array_equal(data, np.array([1, 2, 3, 4, 5]))

def test_invalid_hdf_file():
    with pytest.raises(Exception):
        with h5py.File('nonexistent.h5', 'r') as hdf:
            pass
