import h5py
import os
from collections import deque

class PreRun:
    """
    PreRun performs pre-parsing of the provided input and output HDF5 files for a specified sensor.
    It attempts to retrieve any missing data from the input file by comparing it to the output file.
    """

    def __init__(self, input_file, output_file):
        """
        Initialize the PreRun instance.

        Args:
            input_file (str): Path to the input HDF5 file.
            output_file (str): Path to the output HDF5 file.
        """
        self.input_file = input_file
        self.output_file = output_file

        # Lists to store names of datasets and groups.
        self.datasetsin = []
        self.datasetsout = []
        self.grp_listin = []
        self.grp_listout = []

        self.missing_data = None
        self.sensor_list = None
        self.streams = None
        self.get_missing_data()

    def get_missing_data(self):
        """
        Compares the data groups of the specified sensor in the input and output HDF5 files.
        It constructs lists of dataset names and group names and then computes what is missing.
        
        Returns:
            A dictionary with keys 'missing_groups' and 'missing_datasets' that contain the names
            of groups and datasets present in the output file but missing in the input file.
            Also returns the sensor_list and streams.
        """
        level1_groups_in = []
        level2_groups_in = []
        level1_groups_out = []
        level2_groups_out = []
        
        # Function to traverse an HDF5 file and collect data in a single pass
        def traverse_hdf(file_path, datasets_list, grp_list, level1_groups, level2_groups):
            with h5py.File(file_path, 'r') as hdf_file:
                queue = deque([(hdf_file, '')])
                while queue:
                    group, prefix = queue.popleft()
                    for name, item in group.items():
                        full_name = f"{prefix}/{name}" if prefix else name
                        if isinstance(item, h5py.Dataset):
                            datasets_list.append(full_name)
                        elif isinstance(item, h5py.Group):
                            if not prefix:
                                level1_groups.append(name)
                            elif prefix in level1_groups:
                                level2_groups.append(name)
                            queue.append((item, full_name))
                            grp_list.append(full_name)
        
        # Traverse input and output files in parallel (still sequential execution but combined logic)
        traverse_hdf(self.input_file, self.datasetsin, self.grp_listin, level1_groups_in, level2_groups_in)
        traverse_hdf(self.output_file, self.datasetsout, self.grp_listout, level1_groups_out, level2_groups_out)

        # Compute missing groups and datasets using set operations
        missing_groups_in = set(self.grp_listout) - set(self.grp_listin)
        missing_groups_out = set(self.grp_listin) - set(self.grp_listout)
        missing_datasets_in = set(self.datasetsout) - set(self.datasetsin)
        missing_datasets_out = set(self.datasetsin) - set(self.datasetsout)
        
        # Union operations for final missing elements
        missing_groups = missing_groups_in.union(missing_groups_out)
        missing_datasets = missing_datasets_in.union(missing_datasets_out)
        
        # Combine results into a dictionary
        self.missing_data = {
            'missing_groups': list(missing_groups),
            'missing_datasets': list(missing_datasets)
        }

        # Get the sensor_list and streams - use set intersection for efficiency
        self.sensor_list = list(set(level1_groups_in) & set(level1_groups_out))
        self.streams = list(set(level2_groups_in) & set(level2_groups_out))

        return self.missing_data, self.sensor_list, self.streams, level1_groups_in, level1_groups_out, level2_groups_in, level2_groups_out