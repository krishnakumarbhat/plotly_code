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
        with h5py.File(self.input_file, 'r') as hdf_file_in:
            with h5py.File(self.output_file, 'r') as hdf_file_out:
                # Use BFS to traverse the groups and datasets in the input file
                queue = deque([(hdf_file_in, '')])
                level1_groups_in = []
                level2_groups_in = []
                while queue:
                    group, prefix = queue.popleft()
                    for name, item in group.items():
                        full_name = f"{prefix}/{name}" if prefix else name
                        if isinstance(item, h5py.Dataset):
                            self.datasetsin.append(full_name)
                        elif isinstance(item, h5py.Group):
                            if not prefix:
                                level1_groups_in.append(name)
                            elif prefix in level1_groups_in:
                                level2_groups_in.append(name)
                            queue.append((item, full_name))
                            self.grp_listin.append(full_name)

                # Use BFS to traverse the groups and datasets in the output file
                queue = deque([(hdf_file_out, '')])
                level1_groups_out = []
                level2_groups_out = []
                while queue:
                    group, prefix = queue.popleft()
                    for name, item in group.items():
                        full_name = f"{prefix}/{name}" if prefix else name
                        if isinstance(item, h5py.Dataset):
                            self.datasetsout.append(full_name)
                        elif isinstance(item, h5py.Group):
                            if not prefix:
                                level1_groups_out.append(name)
                            elif prefix in level1_groups_out:
                                level2_groups_out.append(name)
                            queue.append((item, full_name))
                            self.grp_listout.append(full_name)

        # Compute missing groups and datasets:
        missing_groups_in = set(self.grp_listout) - set(self.grp_listin)
        missing_groups_out = set(self.grp_listin) - set(self.grp_listout)
        missing_datasets_in = set(self.datasetsout) - set(self.datasetsin)
        missing_datasets_out = set(self.datasetsin)-set(self.datasetsout)
        missing_groups = missing_groups_in.union(missing_groups_out)
        missing_datasets = missing_datasets_in.union(missing_datasets_out)
        # Combine results into a dictionary, or adjust as needed
        self.missing_data = {
            'missing_groups': list(missing_groups),
            'missing_datasets': list(missing_datasets)
        }

        # Get the sensor_list and streams
        self.sensor_list = [name for name in level1_groups_in if name in level1_groups_out]

        self.streams = [name for name in set(level2_groups_in) & set(level2_groups_out)]

        return self.missing_data, self.sensor_list, self.streams, level1_groups_in, level1_groups_out, level2_groups_in, level2_groups_out
