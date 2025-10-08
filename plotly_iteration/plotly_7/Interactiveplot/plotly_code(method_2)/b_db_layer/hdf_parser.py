import h5py
import numpy as np
from collections import deque
from b_db_layer.data_mapper import stla, stla_nav

class HDF5Parser:
    def __init__(self, input_file):
        self.input_file = input_file
        self.data_structure = {}
        self.queue = deque([input_file])
        self.index_list = []
        self.unique_map = {}
        self.reverse_unique_map = {}
        self.unique_index = 0
        self.sub_index = 0
        self.parent_map = {}
        self.total_scans = 0
        self.lenud = 0

    def parse(self):
        """Parse the HDF5 file using BFS to explore its structure."""
        while self.queue:
            current_item = self.queue.popleft()
            if isinstance(current_item, h5py.Group):
                self._process_group(current_item)
        return self.data_structure, self.unique_map, self.reverse_unique_map ,self.parent_map ,self.lenud

    def _process_group(self, group):
        """Process each group in the HDF5 file."""
        temp_list = []
        
        for name, obj in group.items():
            if name in stla:
                if stla[name] == "Header" or stla[name] == "ScanIndex":
                    self._process_dataset(name, obj)
                    self.queue.append(obj)
                else:
                    temp_list.append((name, obj))

            if name in stla_nav:
                # print(f"Found name in stla_nav: {name}")
                parent_key = f"{self.unique_index}_None"
                self.unique_map[parent_key] = stla_nav[name]
                self.reverse_unique_map[stla_nav[name]] = parent_key
                # print(f"Updated unique maps: {self.unique_map}, {self.reverse_unique_map}")
                self.unique_index += 1

        if temp_list:
            self.queue.extend(temp_list)

        for name, obj in group.items():
            if (name in stla or name in stla_nav) and \
               (stla.get(name) not in ["Header", "ScanIndex"] and 
                stla_nav.get(name) not in ["Header", "ScanIndex"]):
                if name in stla_nav:
                    for item in obj:
                        self.parent_map[item] = name
                self._process_dataset(name, obj)
                self.queue.append(obj)

    def _process_dataset(self, name, dataset):
        """Process datasets based on their type and update structures accordingly."""
        if name in stla:
            if isinstance(dataset, h5py.Group):
                print(f"Group found inside stla: {name}")
            elif isinstance(dataset, h5py.Dataset):
                if stla[name] == "ScanIndex":
                    self._initialize_scan_index(dataset)
                else:
                    self._update_data_structure(name, dataset)

    def _initialize_scan_index(self, dataset):
        """Initialize the scan index dataset."""
        self.index_list = dataset[:-1]
        leftmost_index, rightmost_index = self.index_list[0], self.index_list[-1]
        
        # print(f"Scan indices: {self.index_list}")
        self.total_scans = rightmost_index - leftmost_index
        
        for i in range(len(self.index_list) - 1):
            if self.index_list[i] + 1 != self.index_list[i + 1]:
                print(f"Missing scan index: {self.index_list[i] + 1}.")
        
        for index in self.index_list:
            self.data_structure[f"{index}"] = np.empty(shape=(0,), dtype=dataset.dtype)
        
        # print(f"Data structure initialized: {self.data_structure}")

    def _update_data_structure(self, name, dataset):
        """Update the data structure with values from the dataset."""
        if name in self.parent_map:
            parent_key = self.reverse_unique_map[stla_nav[self.parent_map[name]]]
            parent_prefix, _ = parent_key.split('_')
            self.lenud = len(dataset)
            # need to fix this
            if len(dataset) == len(self.index_list):
                
                for a, index in zip(range(len(dataset)), self.index_list):
                    print(f"Index: {index}, Current Value: {dataset[a]}")
                    current_value = dataset[a]
                    self.data_structure[str(index)] = np.append(self.data_structure.get(str(index), np.array([])), current_value)
                
            unique_key = f"{parent_prefix}_{self.sub_index}"
            self.sub_index += 1
            self.unique_map[unique_key] = stla[name]
            self.reverse_unique_map[stla[name]] = unique_key
        else:
            print("Parent not found; need to add data structure for it.")
