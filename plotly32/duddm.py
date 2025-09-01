import h5py
import numpy as np

class HDF5Parser:
    def __init__(self, input_file, data_container, val_sig_map, scan_index):
        self.input_file = input_file
        self.data_container = data_container
        self.val_sig_map = val_sig_map
        self.val_sig_map_tenth = 0
        self.val_sig_map_unit = None
        self.grp_stack = [input_file]  # Use a list as a grp_stack for DFS
        self.scan_index = scan_index

    def parse(self):
        """Parse the HDF5 file using DFS to explore its structure."""
        while self.grp_stack:
            current_item = self.grp_stack.pop()
            print(f"Processing: {current_item}")

            if isinstance(current_item, h5py.Group):
                print(f"Found group: {current_item.name}")
                # if len(current_item.items()) == 2:
                #     keys_list = list(current_item.keys())
                #     current_key = keys_list[0]
                #     print(f"Current key: {current_key}")

                self._process_group(current_item)

        return self.data_container, self.val_sig_map

    def _process_group(self, group):
        """Process each group in the HDF5 file."""
        for name, obj in group.items():
            if isinstance(obj, h5py.Group):
                print(f"Adding group to stack: {obj.name}")
                self.grp_stack.append(obj)
            elif isinstance(obj, h5py.Dataset):
                print(f"Processing dataset: {name}")
                self._process_dataset(name, obj)

    def _process_dataset(self, name, dataset):
        """Process datasets based on their type and update structures accordingly."""
        for i in dataset:
            print(f"Dataset value: {i}")
            # Assuming data_container is structured correctly to hold this data
            if name not in self.val_sig_map:
                self.val_sig_map[name] = i
                print(f"Added to val_sig_map: {name} -> {i}")
            else:
                print(f"Duplicate found in val_sig_map for {name}")

            # Update data_container (assuming it's a list of lists)
            if len(self.data_container) > 0:
                self.data_container[-1].append(i)
                print(f"Updated data_container with: {i}")
