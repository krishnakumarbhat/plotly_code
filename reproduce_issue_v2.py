
import unittest
import sys
import os
import numpy as np
from unittest.mock import MagicMock

# Add project root to path
sys.path.append('/media/pope/projecteo/github_proj/plotly_code')

from KPI.c_business_layer.detection_matching_kpi import DetectionMappingKPIHDF
from KPI.b_data_storage.kpi_data_model_storage import KPI_DataModelStorage

class TestRealDetectionMatching(unittest.TestCase):
    def setUp(self):
        # Mock the Data Storage
        self.mock_data = MagicMock()
        
        # Setup Data Containers with keys (Scan Indices)
        # Veh: 10, 11, 12
        # Sim: 11, 12, 13
        self.veh_keys = [10, 11, 12]
        self.sim_keys = [11, 12, 13]
        
        # Create explicit mocks for streams
        self.input_stream_det = MagicMock()
        self.input_stream_det._data_container.keys.return_value = self.veh_keys
        
        self.output_stream_det = MagicMock()
        self.output_stream_det._data_container.keys.return_value = self.sim_keys

        self.input_stream_rdd = MagicMock()
        self.output_stream_rdd = MagicMock()
        
        # Configure the main mock to return these when accessed
        # We need to handle dictionary access on self.mock_data
        # self.mock_data is a MagicMock. 
        # We can configure __getitem__ side effect or return value.
        
        def get_stream(key):
            if key == 'DETECTION_STREAM':
                return {'input': self.input_stream_det, 'output': self.output_stream_det}
            if key == 'RDD_STREAM':
                return {'input': self.input_stream_rdd, 'output': self.output_stream_rdd}
            return MagicMock()
            
        self.mock_data.__getitem__.side_effect = get_stream
        
        # Data Setup
        # Veh Scan 10 (idx 0): 1 det
        # Veh Scan 11 (idx 1): 2 dets
        # Veh Scan 12 (idx 2): 3 dets
        self.veh_num_af_det = [np.array([1]), np.array([2]), np.array([3])]
        
        # Sim Scan 11 (idx 0): 2 dets
        # Sim Scan 12 (idx 1): 3 dets
        # Sim Scan 13 (idx 2): 4 dets
        self.sim_num_af_det = [np.array([2]), np.array([3]), np.array([4])]

        
        # RDD Indices
        # Veh Scan 11 (idx 1) -> 2 dets -> RDD indices [0, 1]
        # Sim Scan 11 (idx 0) -> 2 dets -> RDD indices [0, 1]
        
        # We need to mock KPI_DataModelStorage.get_value
        # Since it's a static method, we mock it on the class or patch it.
        
    def test_matching_logic(self):
        # Patch KPI_DataModelStorage.get_value
        original_get_value = KPI_DataModelStorage.get_value
        
        def mock_get_value(storage, param):
            # Determine if input or output based on storage object identity or some property
            # Here we cheat a bit and check if it's the input or output mock object
            is_input = (storage == self.mock_data['DETECTION_STREAM']['input'] or 
                       storage == self.mock_data['RDD_STREAM']['input'])
            
            if param == 'num_af_det':
                return (self.veh_num_af_det if is_input else self.sim_num_af_det), "success"
            
            # For RDD params, return dummy arrays of correct length
            if param in ['rdd1_rindx', 'rdd1_dindx', 'rdd2_range', 'rdd2_range_rate', 'rdd1_num_detect']:
                # Return list of lists
                if is_input:
                    # Veh: [ [0], [0,1], [0,1,2] ]
                    return [[x for x in range(i+1)] for i in range(3)], "success"
                else:
                    # Sim: [ [0,1], [0,1,2], [0,1,2,3] ]
                    return [[x for x in range(i+2)] for i in range(3)], "success"
            
            # For Detection params
            if param == 'rdd_idx':
                # Map 1:1 for simplicity
                if is_input:
                     return [[x for x in range(i+1)] for i in range(3)], "success"
                else:
                     return [[x for x in range(i+2)] for i in range(3)], "success"
            
            # For Threshold params (ran, vel, theta, phi)
            # We want them to match for the common scans
            # Common Scans: 11, 12
            # Veh Scan 11 is idx 1. Sim Scan 11 is idx 0.
            # Veh Scan 12 is idx 2. Sim Scan 12 is idx 1.
            
            # Let's just return zeros for everything so they match perfectly
            if param in ['ran', 'vel', 'theta', 'phi']:
                if is_input:
                    return [[0.0]*(i+1) for i in range(3)], "success"
                else:
                    return [[0.0]*(i+2) for i in range(3)], "success"
            
            return [], "failed"

        KPI_DataModelStorage.get_value = mock_get_value
        
        try:
            processor = DetectionMappingKPIHDF(self.mock_data, "test_sensor")
            
            # Run processing
            processor.process_rdd_matching()
            
            results = processor.kpi_results
            
            print("\nResults:")
            print(f"Matches: {results['matching_accuracy']['matches']}")
            print(f"Total Detections: {results['matching_accuracy']['total_detections']}")
            print(f"Per Scan Matches: {results['per_scan_matches']}")
            print(f"Per Scan Indices: {results['per_scan_scanindex']}")
            
            # Verification
            # Common Scans are 11 and 12.
            # Scan 11: Veh has 2 dets. Sim has 2 dets. Should match 2.
            # Scan 12: Veh has 3 dets. Sim has 3 dets. Should match 3.
            # Total Matches = 5.
            
            self.assertEqual(results['matching_accuracy']['matches'], 5)
            self.assertIn(11, results['per_scan_scanindex'])
            self.assertIn(12, results['per_scan_scanindex'])
            
        finally:
            KPI_DataModelStorage.get_value = original_get_value

if __name__ == '__main__':
    unittest.main()
