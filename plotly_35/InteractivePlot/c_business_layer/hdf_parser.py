import h5py
import numpy as np
import itertools

class HDF5Parser:
    """Parser for HDF5 files that extracts datasets into a structured format."""
    
    def __init__(self, group, data_container,val_sig_map ,sig_val_map , scan_index):
        self.group = group
        self.data_container = data_container
        self.val_sig_map = {}
        self.sig_val_map = {}
        self.val_sig_map_tenth = 0
        self.val_sig_map_unit = 0
        self.grp_list = []  # Stack for DFS traversal
        self.datasets = []# Store datasets found in this group
        self.scan_index = scan_index
    
    @classmethod
    def parse(cls, group, data_container, val_sig_map, sig_val_map ,scan_index):
        """Parse the HDF5 file using DFS to explore its structure."""

        parser_instance = cls(group, data_container,val_sig_map ,sig_val_map,scan_index)
        parser_instance.parse_grp(group)
        return parser_instance.data_container, parser_instance.val_sig_map,parser_instance.sig_val_map

    def parse_grp(self, group):
        """Parse a single HDF5 group and extract datasets."""
        global current_group
        print(f"Group: {group.name}")
        self.val_sig_map_unit = None
        self.val_sig_map[f"{self.val_sig_map_tenth}_{self.val_sig_map_unit}"] = [group.name]
        # if group.name in self.sig_val_map:
        
        # self.sig_val_map[group.name].append({current_group: f"{self.val_sig_map_tenth}_{self.val_sig_map_unit}"})
        # else:
        # will add the if condtion if we have duplcate grp too
        self.sig_val_map[group.name]= f"{self.val_sig_map_tenth}_{self.val_sig_map_unit}"
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
            if dataset_name in self.sig_val_map:
                self.sig_val_map[dataset_name].append({current_group: f"{self.val_sig_map_tenth}_{self.val_sig_map_unit}"})
            else:
                self.sig_val_map[dataset_name]= f"{self.val_sig_map_tenth}_{self.val_sig_map_unit}"

            for row, scanidx in itertools.zip_longest(dataset, self.data_container, fillvalue=[]):
                rows_with_scanidx.append((row, scanidx))

            # Determine maximum row length
            max_row_length = 0
            for row, _ in rows_with_scanidx:
                if row is not None:
                    max_row_length = max(max_row_length, len(row))

            # Process and pad rows before stacking
            for row, scanidx in rows_with_scanidx:
                if row is not None and len(row) > 0:
                    # Pad the row if its length is less than the maximum length
                    if len(row) < max_row_length:
                        pad_width = max_row_length - len(row)
                        row = np.pad(row, (0, pad_width), mode='constant', constant_values=0)  # Pad with zeros
                    N = len(row)
                    if self.data_container[scanidx].size == 0:
                        self.data_container[scanidx] = row
                        continue
                    if self.val_sig_map_unit == 0:
                            # row_reshaped = row.reshape(0, N)
                        self.data_container[scanidx] = np.dstack([self.data_container[scanidx],row])   
                    else:
                        self.data_container[scanidx] = np.vstack([self.data_container[scanidx],row])  
                        # self.data_container[scanidx][-1].append(row)
                else:
                    # Handle missing data by creating a padded row
                    row = np.zeros(max_row_length)  # Create a row of zeros with max length
                    if self.val_sig_map_unit == 0:
                        if self.data_container[scanidx].size == 0:
                            self.data_container[scanidx] = np.empty((0, max_row_length, 0))  #empty((1, N,0))
                            # row_reshaped = row.reshape(1, N, 1)
                        self.data_container[scanidx] = np.dstack([self.data_container[scanidx], row])
                    else:
                        self.data_container[scanidx] = np.vstack([self.data_container[scanidx], row])
                    print(f"Missing data in {dataset_name} in {group.name} in scanindex = {scanidx}")

            self.val_sig_map_unit += 1

        # Process child groups recursively
        while self.grp_list:
            current_group = self.grp_list.pop(0)
            self.val_sig_map_tenth += 1
            self.parse_grp(current_group)
