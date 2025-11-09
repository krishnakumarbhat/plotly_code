import os
import numpy as np
from typing import Dict, List
from c_data_storage.interfaces import IDataFormatter, IDataStorage, ILogger

class DataModelStorageFormatter(IDataFormatter):
    """
    Data formatter for DataModelStorage format.
    Follows Single Responsibility Principle by focusing only on data formatting and file output.
    """
    
    def __init__(self, logger: ILogger):
        """
        Initialize the data formatter.
        
        Args:
            logger: Logger instance for logging operations
        """
        self.logger = logger
    
    def format_data(self, storage: IDataStorage, output_file: str, header: str) -> bool:
        """
        Format and save data to file.
        
        Args:
            storage: Data storage instance containing the data
            output_file: Path to output file
            header: Header text for the file
            
        Returns:
            True if formatting was successful, False otherwise
        """
        try:
            # Ensure output directory exists
            os.makedirs(os.path.dirname(output_file), exist_ok=True)
            
            with open(output_file, 'w') as f:
                # Write header
                f.write(f"# {header}\n")
                f.write(f"# DataModelStorage Format\n")
                f.write("#" + "="*80 + "\n\n")
                
                # Write storage structure information
                self._write_storage_info(f, storage)
                
                # Write signal mappings
                self._write_signal_mappings(f, storage)
                
                # Write data container structure
                self._write_data_container_info(f, storage)
                
                # Write detailed data for each scan index
                self._write_detailed_data(f, storage)
                
                # Write summary statistics
                self._write_summary_statistics(f, storage)
                
                # Write signal information
                self._write_signal_information(f, storage)
            
            self.logger.info(f"Successfully formatted data to {output_file}")
            return True
            
        except Exception as e:
            self.logger.error(f"Error formatting data: {e}")
            return False
    
    def _write_storage_info(self, file, storage: IDataStorage) -> None:
        """Write storage structure information."""
        file.write("# DataModelStorage Structure:\n")
        file.write(f"# - Number of scan indices: {len(storage.get_data_container())}\n")
        file.write(f"# - Number of signals: {len(storage.get_signal_to_value())}\n")
        file.write(f"# - Parent counter: {storage.get_parent_counter()}\n")
        file.write(f"# - Child counter: {storage.get_child_counter()}\n")
        file.write(f"# - Stream name: {storage.get_stream_name()}\n\n")
    
    def _write_signal_mappings(self, file, storage: IDataStorage) -> None:
        """Write signal mappings."""
        file.write("# Signal Mappings:\n")
        file.write("# _signal_to_value:\n")
        for signal_name, value in storage.get_signal_to_value().items():
            file.write(f"#   {signal_name}: {value}\n")
        file.write("\n")
        
        file.write("# _value_to_signal:\n")
        for value, signal_name in storage.get_value_to_signal().items():
            file.write(f"#   {value}: {signal_name}\n")
        file.write("\n")
    
    def _write_data_container_info(self, file, storage: IDataStorage) -> None:
        """Write data container structure information."""
        file.write("# Data Container Structure:\n")
        scan_indices = sorted(storage.get_data_container().keys())
        file.write(f"# Scan indices: {scan_indices}\n\n")
    
    def _write_detailed_data(self, file, storage: IDataStorage) -> None:
        """Write detailed data for each scan index."""
        for scan_idx in sorted(storage.get_data_container().keys()):
            file.write(f"# Scan Index {scan_idx}:\n")
            data_list = storage.get_data_container()[scan_idx]
            
            if not data_list:
                file.write(f"#   Empty data\n")
            else:
                for group_idx, group_data in enumerate(data_list):
                    file.write(f"#   Group {group_idx}:\n")
                    if isinstance(group_data, list):
                        for item_idx, item in enumerate(group_data):
                            if isinstance(item, np.ndarray):
                                # Show non-zero values for cleaner output
                                non_zero_indices = np.nonzero(item)[0]
                                if len(non_zero_indices) > 0:
                                    file.write(f"#     Item {item_idx}: ")
                                    for idx in non_zero_indices:
                                        file.write(f"Col{idx}={item[idx]:.6f} ")
                                    file.write("\n")
                                else:
                                    file.write(f"#     Item {item_idx}: All zeros\n")
                            else:
                                file.write(f"#     Item {item_idx}: {item}\n")
                    else:
                        file.write(f"#     {group_data}\n")
            file.write("\n")
    
    def _write_summary_statistics(self, file, storage: IDataStorage) -> None:
        """Write summary statistics."""
        file.write("# Summary Statistics:\n")
        total_data_points = sum(len(data_list) for data_list in storage.get_data_container().values())
        file.write(f"# - Total data points: {total_data_points}\n")
        
        if storage.get_data_container():
            avg_points = total_data_points / len(storage.get_data_container())
            file.write(f"# - Average data points per scan: {avg_points:.2f}\n")
        else:
            file.write(f"# - Average data points per scan: 0.00\n")
        file.write("\n")
    
    def _write_signal_information(self, file, storage: IDataStorage) -> None:
        """Write signal information."""
        file.write("# Signal Information:\n")
        for signal_name in storage.get_signal_to_value().keys():
            if signal_name in storage.get_value_to_signal().values():
                file.write(f"# - {signal_name}: Available\n")
            else:
                file.write(f"# - {signal_name}: Not found in value mappings\n") 