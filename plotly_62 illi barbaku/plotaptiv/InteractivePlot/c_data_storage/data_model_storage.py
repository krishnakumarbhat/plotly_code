from typing import Dict, List, Optional, Any
from dataclasses import dataclass, field
import itertools

@dataclass
class SignalMetadata:
    """Metadata for a signal including its group relationships"""
    group_mappings: List[Dict[str, str]] = field(default_factory=list)
    value_key: Optional[str] = None

class DataModelStorage:
    """
    A hierarchical storage manager for scientific data that maintains bidirectional 
    mappings between signals and their values while preserving group relationships.
    """
    
    def __init__(self):
        self._value_to_signal_map: Dict[str, str] = {}
        self._signal_metadata: Dict[str, SignalMetadata] = {}
        self._scan_data: Dict[str, List] = {}
        self._hierarchy_counter = HierarchyCounter()
    
    def initialize_scan_indices(self, scan_indices: List[int]) -> None:
        """
        Initialize the data container with scan indices.
        
        Args:
            scan_indices: List of scan indices to initialize storage with
        """
        self._scan_data = {str(idx): [] for idx in scan_indices}
    
    def start_new_parent_group(self) -> None:
        """Reset hierarchy tracking when starting a new parent group."""
        self._hierarchy_counter.reset_child()

    def set_value(self, dataset: Any, scan_index: List[int], signal_name: str, grp_name: str) -> str:
        """
        Store a dataset value while maintaining hierarchical relationships.
        
        Args:
            dataset: The dataset to store
            scan_index: List of scan indices for data organization
            signal_name: Name of the signal
            grp_name: Name of the group this signal belongs to
            
        Returns:
            str: Generated unique key for the stored data
        """
        if self._hierarchy_counter.is_new_parent():
            return self._store_parent_data(dataset, signal_name, grp_name)
        return self._store_child_data(dataset, signal_name, grp_name)

    def _store_parent_data(self, dataset: Any, signal_name: str, grp_name: str) -> str:
        """Store data for a parent node in the hierarchy."""
        key = self._hierarchy_counter.get_parent_key()
        
        # Store data for each scan index
        for row, scan_idx in itertools.zip_longest(dataset, self._scan_data, fillvalue=[]):
            if not row:
                print(f"Warning: Missing data for signal '{signal_name}' in group '{grp_name}' at scan index {scan_idx}")
            self._scan_data[scan_idx].append([row])

        self._value_to_signal_map[key] = signal_name
        self._signal_metadata[signal_name] = SignalMetadata(value_key=key)
        return key

    def _store_child_data(self, dataset: Any, signal_name: str, grp_name: str) -> str:
        """Store data for a child node in the hierarchy."""
        key = self._hierarchy_counter.get_next_child_key()
        
        # Store data for each scan index
        for row, scan_idx in itertools.zip_longest(dataset, self._scan_data, fillvalue=[]):
            if not row:
                print(f"Warning: Missing data for signal '{signal_name}' in group '{grp_name}' at scan index {scan_idx}")
            self._scan_data[scan_idx][-1].append(row)

        self._value_to_signal_map[key] = signal_name
        
        if signal_name not in self._signal_metadata:
            self._signal_metadata[signal_name] = SignalMetadata(
                group_mappings=[{grp_name: key}]
            )
        else:
            self._signal_metadata[signal_name].group_mappings.append({grp_name: key})
        
        return key

    def get_signal_metadata(self, signal_name: str) -> Optional[SignalMetadata]:
        """
        Get metadata for a given signal.
        
        Args:
            signal_name: Name of the signal
            
        Returns:
            Optional[SignalMetadata]: Signal metadata if found
        """
        return self._signal_metadata.get(signal_name)
    
    def get_signal_data(self, signal_name: str, scan_index: int = 0) -> Optional[Any]:
        """
        Retrieve data for a given signal name and scan index.
        
        Args:
            signal_name: Name of the signal to retrieve
            scan_index: Index to retrieve data from (defaults to 0)
            
        Returns:
            Optional[Any]: The stored data if found
        """
        metadata = self._signal_metadata.get(signal_name)
        if not metadata:
            return None
            
        if metadata.group_mappings:
            # Handle hierarchical data with multiple group mappings
            result = []
            for mapping in metadata.group_mappings:
                for key in mapping.values():
                    parent_idx, child_idx = self._parse_hierarchy_key(key)
                    if child_idx is None:
                        result.extend(self._scan_data[scan_index][parent_idx - 1][0])
                    else:
                        result.extend(self._scan_data[scan_index][parent_idx - 1][child_idx])
            return result
        else:
            # Handle single value data
            parent_idx, child_idx = self._parse_hierarchy_key(metadata.value_key)
            if child_idx is None:
                return self._scan_data[scan_index][parent_idx - 1][0]
            return self._scan_data[scan_index][parent_idx - 1][child_idx]
    
    @staticmethod
    def _parse_hierarchy_key(key: str) -> tuple[int, Optional[int]]:
        """Parse a hierarchy key into parent and child indices."""
        parent_str, child_str = key.split('_')
        parent_idx = int(parent_str)
        child_idx = None if child_str == 'None' else int(child_str)
        return parent_idx, child_idx
    
    def clear(self) -> None:
        """Clear all stored data and reset counters."""
        self._value_to_signal_map.clear()
        self._signal_metadata.clear()
        self._scan_data.clear()
        self._hierarchy_counter.reset()


class HierarchyCounter:
    """Manages hierarchical counting for parent-child relationships in data storage."""
    
    def __init__(self):
        self.parent_count = 0
        self.child_count = -1
        
    def reset(self) -> None:
        """Reset both parent and child counters."""
        self.parent_count = 0
        self.child_count = -1
        
    def reset_child(self) -> None:
        """Reset only the child counter."""
        self.child_count = -1
        
    def is_new_parent(self) -> bool:
        """Check if we're starting a new parent group."""
        return self.child_count == -1
        
    def get_parent_key(self) -> str:
        """Get key for a new parent node."""
        self.parent_count += 1
        self.child_count += 1
        return f"{self.parent_count}_None"
        
    def get_next_child_key(self) -> str:
        """Get key for the next child node."""
        self.child_count += 1
        return f"{self.parent_count}_{self.child_count}"
