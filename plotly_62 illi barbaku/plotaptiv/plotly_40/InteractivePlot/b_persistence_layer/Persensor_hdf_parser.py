import h5py
import os
from InteractivePlot.d_business_layer.data_prep import DataPrep
from InteractivePlot.b_persistence_layer import hdf_parser
from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage

class PersensorHdfParser:
    """Parser for HDF5 files based on an address map and customer type."""
    
    def __init__(self, address_map, output_dir=None):
        self.address_map = address_map
        self.html_name = ""
        self.output_dir = output_dir or "html"  # Default to "html" if not provided
        self.input_storage = DataModelStorage()
        self.output_storage = DataModelStorage()
    
    def generate_html_name(self, input_file, output_file):
        """
        Generate HTML filename by keeping the input name and removing common parts from output name.
        
        Args:
            input_file: Path to input HDF file
            output_file: Path to output HDF file
            
        Returns:
            str: HTML filename
        """
        input_name = os.path.basename(input_file).split('.')[0]
        output_name = os.path.basename(output_file).split('.')[0]
        
        # Find the common part and remove it
        unique_part = output_name
        # Check for common suffix (like "_out" or similar)
        for i in range(1, min(len(input_name), len(output_name))):
            if input_name.endswith(output_name[:i]) or output_name.startswith(input_name[-i:]):
                unique_part = output_name[i:]
                break
        
        # If we have a unique part, combine it with input name, otherwise use both
        if unique_part and unique_part != output_name:
            html_name = f"{input_name}_{unique_part}.html"
        else:
            html_name = f"{input_name}_{output_name}.html"
            
        return html_name

    def parse(self):
        """Parse input and output HDF5 files based on the address map."""
        
        total_files = len(self.address_map)
        processed_files = 0
        
        for input_file, output_file in self.address_map.items():
            processed_files += 1
            progress = (processed_files / total_files) * 100
            print(f"\nProcessing file pair ({processed_files}/{total_files}) [{progress:.1f}%]:")
            print(f"  Input: {os.path.basename(input_file)}")
            print(f"  Output: {os.path.basename(output_file)}")
            
            # Process input file
            print("  Reading input HDF file...", end="", flush=True)
            start_time = os.times().system
            with h5py.File(input_file, 'r') as hdf_file:
                scan_index = hdf_file["Header/ScanIndex"][()]
                data_group = hdf_file["data"]
                self.input_storage.initialize(scan_index)
                hdf_parser.HDF5Parser.parse(data_group, self.input_storage, scan_index)
            end_time = os.times().system
            print(f" Done ({end_time-start_time:.2f}s)")
            
            # Process output file
            print("  Reading output HDF file...", end="", flush=True)
            start_time = os.times().system
            with h5py.File(output_file, 'r') as hdf_file_out:
                scan_index = hdf_file_out["Header/ScanIndex"][()]
                data_group = hdf_file_out["data"]
                self.output_storage.initialize(scan_index)
                hdf_parser.HDF5Parser.parse(data_group, self.output_storage, scan_index)
            end_time = os.times().system
            print(f" Done ({end_time-start_time:.2f}s)")

            # Generate HTML name using the new method
            self.html_name = self.generate_html_name(input_file, output_file)
            
            print(f"  Generating report: {self.html_name}...", end="", flush=True)
            start_time = os.times().system
            DataPrep(
                self.input_storage,
                self.output_storage,
                self.html_name,
                self.output_dir
            )
            end_time = os.times().system
            print(f" Done ({end_time-start_time:.2f}s)")
