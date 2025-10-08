import h5py
from typing import Any, List, Dict, Optional
from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage

class HDF5Parser:
    """
    A parser for HDF5 files that implements a depth-first traversal strategy to extract
    and organize hierarchical scientific data into a structured format.
    """
    
    def __init__(self, root_group: h5py.Group, data_storage: DataModelStorage, scan_indices: List[int]):
        """
        Initialize the HDF5 parser with necessary components.
        
        Args:
            root_group: Root HDF5 group to start parsing from
            data_storage: Storage manager for parsed data
            scan_indices: List of scan indices for data organization
        """
        self.root_group = root_group
        self.data_storage = data_storage
        self.scan_indices = scan_indices
        self.pending_groups: List[h5py.Group] = []  # Groups pending processing in DFS
        self.current_datasets: List[tuple[str, h5py.Dataset]] = []  # Datasets in current group
        
    @classmethod
    def parse(cls, root_group: h5py.Group, data_storage: DataModelStorage, 
              scan_indices: List[int]) -> DataModelStorage:
        """
        Parse an HDF5 file structure using depth-first traversal.
        
        Args:
            root_group: Root HDF5 group to parse
            data_storage: Storage manager for parsed data
            scan_indices: List of scan indices for data organization
            
        Returns:
            DataModelStorage: Storage instance containing parsed data
        """
        parser = cls(root_group, data_storage, scan_indices)
        parser._traverse_group(parser.root_group)
        return data_storage

    def _traverse_group(self, current_group: h5py.Group) -> None:
        """
        Traverse an HDF5 group recursively, extracting datasets and maintaining hierarchy.
        
        Args:
            current_group: The HDF5 group currently being processed
        """
        # Process all items in current group
        for item_name, item in current_group.items():
            if isinstance(item, h5py.Dataset):
                self.current_datasets.append((item_name, item))
            elif isinstance(item, h5py.Group) and item_name != "Stream_Hdr":
                self.pending_groups.append(item)
        
        # Process all datasets in current group
        self._process_datasets(current_group)
        
        # Continue DFS traversal
        while self.pending_groups:
            next_group = self.pending_groups.pop(0)  # Process groups in order
            self._traverse_group(next_group)
    
    def _process_datasets(self, current_group: h5py.Group) -> None:
        """
        Process and store all datasets found in the current group.
        
        Args:
            current_group: The group whose datasets are being processed
        """
        while self.current_datasets:
            dataset_name, dataset = self.current_datasets.pop(0)
            self.data_storage.set_value(
                dataset=dataset,
                scan_index=self.scan_indices,
                signal_name=dataset_name,
                grp_name=current_group.name
            )
            
        
