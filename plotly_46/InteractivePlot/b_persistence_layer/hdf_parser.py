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
        self._cache = {}  # Cache for parsed groups
        self._visited = set()  # Track visited groups to prevent cycles

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
        parser._parse_recursive(group)
        return storage

    def _parse_recursive(self, group: h5py.Group) -> None:
        """
        Efficient recursive parsing of HDF5 groups with caching.
        
        Args:
            group: The HDF5 group to parse
        """
        group_path = group.name
        if group_path in self._visited:
            return
        
        self._visited.add(group_path)
        
        # Process datasets first
        for name, item in group.items():
            if isinstance(item, h5py.Dataset):
                self._process_dataset(name, item)
            elif isinstance(item, h5py.Group) and name != "Stream_Hdr":
                self._parse_recursive(item)

    def _process_dataset(self, dataset_name: str, dataset: h5py.Dataset) -> None:
        """
        Process a single dataset and store it efficiently.
        
        Args:
            dataset_name: Name of the dataset
            dataset: HDF5 dataset object
        """
        group_name = self.root_group.name.split('/')[-1]
        cache_key = f"{group_name}/{dataset_name}"
        
        if cache_key not in self._cache:
            self._cache[cache_key] = True
            self.storage.set_value(
                dataset,
                self.scan_index,
                signal_name=dataset_name,
                grp_name=group_name
            )
