import h5py
import numpy as np
import os
import logging
from typing import Dict, List, Any

class DataDumper:
    """
    Utility class for dumping specific datasets from HDF files to text files.
    Extracts data from Tracker_Information/OLP and OSI_Ground_Truth/Object datasets.
    """
    
    def __init__(self, hdf_file_path: str, output_dir: str = "html"):
        """
        Initialize the DataDumper.
        
        Args:
            hdf_file_path: Path to the HDF file
            output_dir: Directory to save output files
        """
        self.hdf_file_path = hdf_file_path
        self.output_dir = output_dir
        self.logger = logging.getLogger(__name__)
        
        # Ensure output directory exists
        os.makedirs(output_dir, exist_ok=True)
        
    def dump_tracker_data(self) -> bool:
        """
        Dump Tracker_Information/OLP datasets to Track.txt
        
        Returns:
            bool: True if successful, False otherwise
        """
        try:
            with h5py.File(self.hdf_file_path, 'r') as f:
                if 'Tracker_Information' not in f or 'OLP' not in f['Tracker_Information']:
                    self.logger.warning("Tracker_Information/OLP not found in HDF file")
                    return False
                
                olp_group = f['Tracker_Information']['OLP']
                tracker_datasets = ['vcs_accel_x', 'vcs_accel_y', 'vcs_pos_x', 'vcs_pos_y', 'vcs_vel_x', 'vcs_vel_y']
                
                # Check if all required datasets exist
                missing_datasets = [ds for ds in tracker_datasets if ds not in olp_group]
                if missing_datasets:
                    self.logger.warning(f"Missing datasets in Tracker_Information/OLP: {missing_datasets}")
                
                # Extract data
                data_dict = {}
                for dataset_name in tracker_datasets:
                    if dataset_name in olp_group:
                        data_dict[dataset_name] = olp_group[dataset_name][()]
                    else:
                        # Create empty array with same shape as other datasets
                        if data_dict:
                            first_key = list(data_dict.keys())[0]
                            data_dict[dataset_name] = np.zeros_like(data_dict[first_key])
                        else:
                            data_dict[dataset_name] = np.array([])
                
                # Save to file
                output_file = os.path.join(self.output_dir, "Track.txt")
                self._save_data_to_file(data_dict, output_file, "Tracker Information - OLP")
                
                self.logger.info(f"Successfully dumped Tracker_Information/OLP data to {output_file}")
                return True
                
        except Exception as e:
            self.logger.error(f"Error dumping Tracker_Information data: {e}")
            return False
    
    def dump_osi_data(self) -> bool:
        """
        Dump OSI_Ground_Truth/Object datasets to OSI track.txt
        
        Returns:
            bool: True if successful, False otherwise
        """
        try:
            with h5py.File(self.hdf_file_path, 'r') as f:
                if 'OSI_Ground_Truth' not in f or 'Object' not in f['OSI_Ground_Truth']:
                    self.logger.warning("OSI_Ground_Truth/Object not found in HDF file")
                    return False
                
                object_group = f['OSI_Ground_Truth']['Object']
                osi_datasets = ['length', 'speed', 'vcs_lat_accel', 'vcs_lat_posn', 'vcs_lat_vel', 
                               'vcs_long_accel', 'vcs_long_posn', 'vcs_long_vel']
                
                # Check if all required datasets exist
                missing_datasets = [ds for ds in osi_datasets if ds not in object_group]
                if missing_datasets:
                    self.logger.warning(f"Missing datasets in OSI_Ground_Truth/Object: {missing_datasets}")
                
                # Extract data
                data_dict = {}
                for dataset_name in osi_datasets:
                    if dataset_name in object_group:
                        data_dict[dataset_name] = object_group[dataset_name][()]
                    else:
                        # Create empty array with same shape as other datasets
                        if data_dict:
                            first_key = list(data_dict.keys())[0]
                            data_dict[dataset_name] = np.zeros_like(data_dict[first_key])
                        else:
                            data_dict[dataset_name] = np.array([])
                
                # Save to file
                output_file = os.path.join(self.output_dir, "OSI track.txt")
                self._save_data_to_file(data_dict, output_file, "OSI Ground Truth - Object")
                
                self.logger.info(f"Successfully dumped OSI_Ground_Truth/Object data to {output_file}")
                return True
                
        except Exception as e:
            self.logger.error(f"Error dumping OSI_Ground_Truth data: {e}")
            return False
    
    def _save_data_to_file(self, data_dict: Dict[str, np.ndarray], output_file: str, header: str):
        """
        Save data dictionary to a text file with proper formatting.
        
        Args:
            data_dict: Dictionary containing dataset names and their data
            output_file: Path to output file
            header: Header text for the file
        """
        with open(output_file, 'w') as f:
            # Write header
            f.write(f"# {header}\n")
            f.write(f"# Generated from: {self.hdf_file_path}\n")
            f.write(f"# Number of datasets: {len(data_dict)}\n")
            f.write("#" + "="*80 + "\n\n")
            
            # Write dataset information
            for dataset_name, data in data_dict.items():
                f.write(f"# Dataset: {dataset_name}\n")
                f.write(f"# Shape: {data.shape}\n")
                f.write(f"# Data type: {data.dtype}\n")
                f.write(f"# Data:\n")
                
                # Write data in a readable format
                if len(data.shape) == 2:
                    # For 2D data, write each row
                    for i in range(data.shape[0]):
                        row_data = data[i]
                        # Filter out zero values for cleaner output
                        non_zero_indices = np.nonzero(row_data)[0]
                        if len(non_zero_indices) > 0:
                            f.write(f"Row {i}: ")
                            for j in non_zero_indices:
                                f.write(f"Col{j}={row_data[j]:.6f} ")
                            f.write("\n")
                        else:
                            f.write(f"Row {i}: All zeros\n")
                else:
                    # For 1D data or other shapes
                    f.write(f"{data}\n")
                
                f.write("\n" + "-"*50 + "\n\n")
    
    def dump_all_data(self) -> Dict[str, bool]:
        """
        Dump both Tracker_Information and OSI_Ground_Truth data.
        
        Returns:
            Dict[str, bool]: Dictionary with results for each dump operation
        """
        results = {}
        results['tracker'] = self.dump_tracker_data()
        results['osi'] = self.dump_osi_data()
        return results 