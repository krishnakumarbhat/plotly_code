from .data_store import DataStore
import h5py
import numpy as np
import itertools
class HDF5Parser:
    """Parser for HDF5 files that extracts datasets into a structured format."""
    
    def __init__(self, group, data_container, , scan_index):
        self.group = group
        self.data_container = data_container
        self.val_sig_map = {}
        self.sig_val_map = {}
        self.val_sig_map_tenth = 0
        self.val_sig_map_unit = 0
        self.grp_list = []  # Stack for DFS traversal
        self.datasets = []  # Store datasets found in this group
        self.scan_index = scan_index
    
    @classmethod
    def parse(cls, group, data_container, val_sig_map, scan_index):
        """Parse the HDF5 file using DFS to explore its structure."""
        
        parser_instance = cls(group, data_container, scan_index)
        parser_instance.parse_grp(group)
        return parser_instance.data_container, parser_instance.val_sig_map

    def parse_grp(self, group):
        """Parse a single HDF5 group and extract datasets."""
        
        print(f"Group: {group.name}")
        self.val_sig_map_unit = None
        self.val_sig_map[f"{self.val_sig_map_tenth}_{self.val_sig_map_unit}"] = [group.name]
        
        # Reset unit counter for new group parsing
        self.val_sig_map_unit = 0  
        # Traverse items in the current group

        for item_name in group:
            item = group[item_name]
            if isinstance(item, h5py.Dataset):
                # Store dataset details in the list
                self.datasets.append((item_name, item))
            elif isinstance(item, h5py.Group):
                # Add child groups to the stack for later processing
                self.grp_list.append(item)

        # Process all datasets found in this group
        while self.datasets:
            dataset_info = self.datasets.pop(0)  # Use pop(0) to maintain order
            dataset_name = dataset_info[0]
            dataset = dataset_info[1]
            
            # Update value signal map with dataset name
            self.val_sig_map[f"{self.val_sig_map_tenth}_{self.val_sig_map_unit}"] = [dataset_name]
            
            for row, scanidx in itertools.zip_longest(dataset, self.data_container, fillvalue=[]):
                if row is not None and len(row) > 0:
                    if self.val_sig_map_unit == 0:
                        self.data_container[scanidx].append([row])
                    else:
                        self.data_container[scanidx][-1].append(row)
                else:
                    if self.val_sig_map_unit == 0:
                        self.data_container[scanidx].append([row])
                    else:
                        self.data_container[scanidx][-1].append(row)
                    print(f"Missing data in {dataset_name} in {group.name} in scanindex = {scanidx}")

            self.val_sig_map_unit += 1

        # Process child groups recursively
        while self.grp_list:
            current_group = self.grp_list.pop(0)
            self.val_sig_map_tenth += 1
            self.parse_grp(current_group)
