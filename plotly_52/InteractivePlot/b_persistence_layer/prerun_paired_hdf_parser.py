import h5py
import os
from collections import deque

class PreRunPaired:
    """
    PreRunPaired performs pre-parsing of the provided paired HDF5 files.
    It analyzes each input-output pair independently, not requiring common streams across all files.
    """

    def __init__(self, input_files, output_files):
        """
        Initialize the PreRunPaired instance.

        Args:
            input_files (list): List of paths to input HDF5 files
            output_files (list): List of paths to output HDF5 files
        """
        self.input_files = input_files
        self.output_files = output_files
        
        # Check that input and output files have the same length
        if len(input_files) != len(output_files):
            raise ValueError("Number of input files must match number of output files")
        
        # Dictionary to hold sensors and streams for each pair
        # Structure: { pair_index: { 'sensors': [], 'streams': {} } }
        self.pair_data = {}
        
        self.analyze_files()

    def get_groups_and_datasets(self, file_path):
        """
        Extracts groups and datasets from an HDF5 file.
        
        Args:
            file_path (str): Path to the HDF5 file.
            
        Returns:
            tuple: (level1_groups, level2_groups_by_parent, datasets)
        """
        level1_groups = []
        level2_groups_by_parent = {}
        datasets = []
        
        try:
            with h5py.File(file_path, 'r') as hdf_file:
                # Get top-level groups (sensors)
                for key in hdf_file.keys():
                    if key != "Header" and key != "data" and isinstance(hdf_file[key], h5py.Group):
                        level1_groups.append(key)
                        level2_groups_by_parent[key] = []
                        
                        # Get second-level groups (streams) for each sensor
                        for stream in hdf_file[key].keys():
                            if isinstance(hdf_file[key][stream], h5py.Group):
                                level2_groups_by_parent[key].append(stream)
                                
                                # Collect datasets in each stream
                                for dataset in hdf_file[key][stream]:
                                    if isinstance(hdf_file[key][stream][dataset], h5py.Dataset):
                                        datasets.append(f"{key}/{stream}/{dataset}")
        except Exception as e:
            print(f"Error analyzing {file_path}: {str(e)}")
            
        return level1_groups, level2_groups_by_parent, datasets

    def analyze_files(self):
        """
        Analyzes each pair of files independently to find sensors and streams.
        Each input file is only compared with its corresponding output file.
        """
        # Process each pair of files
        for i, (input_file, output_file) in enumerate(zip(self.input_files, self.output_files)):
            # Initialize data structure for this pair
            self.pair_data[i] = {
                'sensors': [],
                'streams': {},
                'input_file': input_file,
                'output_file': output_file
            }
            
            # Get structure from input and output files
            input_groups, input_streams, input_datasets = self.get_groups_and_datasets(input_file)
            output_groups, output_streams, output_datasets = self.get_groups_and_datasets(output_file)
            
            # Find common sensors between input and output
            input_sensors = set(input_groups)
            output_sensors = set(output_groups)
            common_sensors = list(input_sensors & output_sensors)
            self.pair_data[i]['sensors'] = common_sensors
            
            # Find streams for each sensor in this pair
            for sensor in common_sensors:
                input_sensor_streams = set(input_streams.get(sensor, []))
                output_sensor_streams = set(output_streams.get(sensor, []))
                common_streams = input_sensor_streams & output_sensor_streams
                
                if common_streams:
                    self.pair_data[i]['streams'][sensor] = list(common_streams)
            
            # Print summary for this pair
            print(f"\nFile Pair {i+1} Analysis:")
            print(f"  Input: {os.path.basename(input_file)}")
            print(f"  Output: {os.path.basename(output_file)}")
            
            if common_sensors:
                print(f"  Found {len(common_sensors)} sensors: {', '.join(common_sensors)}")
                
                total_streams = sum(len(streams) for streams in self.pair_data[i]['streams'].values())
                print(f"  Found {total_streams} streams")
                
                for sensor, streams in self.pair_data[i]['streams'].items():
                    print(f"    - {sensor}: {len(streams)} streams: {', '.join(streams)}")
            else:
                print("  No common sensors found in this pair")
