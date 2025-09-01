import h5py
from typing import Any, List, Optional
from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage

class HDF5Parser:
    """
    A parser for HDF5 files that implements a depth-first traversal strategy to extract
    and organize hierarchical scientific data into a structured format.
    """
    
    def __init__(self, group: h5py.Group, storage: DataModelStorage, scan_index: List[int]):
        """
        Initialize the HDF5 Parser.
        
        Args:
            group: The HDF5 group to parse
            storage: Storage instance to save parsed data
            scan_index: List of scan indices for data organization
        """
        self.root_group = group
        self.storage = storage
        self.scan_index = scan_index
        self.grp_list: List[h5py.Group] = []  # Stack for DFS traversal
        self.datasets: List[tuple] = []  # Store datasets found in this group
        
    @classmethod
    def parse(cls, group: h5py.Group, storage: DataModelStorage, scan_index: List[int]) -> DataModelStorage:
        """
        Parse an HDF5 file structure using depth-first traversal.
        
        Args:
            group: The HDF5 group to parse
            storage: Storage instance to save parsed data
            scan_index: List of scan indices for data organization
            
        Returns:
            DataModelStorage: The storage instance with parsed data
        """
        parser = cls(group, storage, scan_index)
        parser.parse_grp(parser.root_group)
        return storage

    def parse_grp(self, group: h5py.Group) -> None:
        """
        Recursively parse an HDF5 group, process its datasets, and store data.
        
        Args:
            group: The HDF5 group to parse
        """
        # Process datasets and child groups in a single pass to reduce memory overhead
        datasets = []
        child_groups = []
        
        # Single loop through items to classify them
        for item_name, item in group.items():
            if isinstance(item, h5py.Dataset):
                datasets.append((item_name, item))
            elif isinstance(item, h5py.Group) and item_name not in ("Stream_Hdr"):
                child_groups.append(item)
        
        # Process all datasets in batch to reduce method call overhead
        for dataset_name, dataset in datasets:
            self.storage.set_value(
                dataset,
                self.scan_index,
                signal_name=dataset_name,
                grp_name=group.name.split('/')[-1]
            )
        
        # Process child groups
        for current_group in child_groups:
            self.parse_grp(current_group)
            
        
