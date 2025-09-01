import h5py
import itertools

class HDF5Parser:
    """Parser for HDF5 files that extracts datasets into a structured format."""
    
    def __init__(self, group, data_container, scan_index):
        self.root_group = group
        self.data_container = data_container
        self.val_sig_map = {}
        self.sig_val_map = {}

        self.scan_index = scan_index
        # Internal state variables
        self.val_sig_map_tenth = 0
        self.val_sig_map_unit = 0
        self.grp_list = []  # Stack for DFS traversal
        self.datasets = []# Store datasets found in this group
        
    
    @classmethod
    def parse(cls, group, data_container,scan_index):
        """Parse the HDF5 file using DFS to explore its structure."""
        
        parser = cls(group, data_container,scan_index)
        parser.parse_grp(parser.root_group)
        return parser.data_container, parser.val_sig_map,parser.sig_val_map

    def parse_grp(self, group):
        """
        Recursively parse an HDF5 group, process its datasets, and update mapping dictionaries.
        """

        # leave the root group and record the group info in our maps.
        if group != self.root_group:
            global current_group
            print(f"Group: {group.name}")
            self.val_sig_map_unit = None
            self.val_sig_map[f"{self.val_sig_map_tenth}_{self.val_sig_map_unit}"] = [group.name]
            
            # will add the if condtion if we have duplcate grp too
            # if group.name in self.sig_val_map:
            # self.sig_val_map[group.name].append({current_group: f"{self.val_sig_map_tenth}_{self.val_sig_map_unit}"})
            # else:

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
                if "stream_hdr" in item_name:
                    pass
                else:
                    self.grp_list.append(item)
        
        # Process all datasets found in this group
        while self.datasets:
            dataset_info = self.datasets.pop(0)  # Use pop(0) to maintain order
            dataset_name = dataset_info[0]
            dataset = dataset_info[1]
            
            # Update value signal map with dataset name
            self.val_sig_map[f"{self.val_sig_map_tenth}_{self.val_sig_map_unit}"] = [dataset_name]
            
            for row, scanidx in itertools.zip_longest(dataset, self.data_container, fillvalue=[]):
                if row is not None:
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
