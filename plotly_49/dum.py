import os
import h5py

# Configuration
input_path = "C:/Plastic/10030156_03_Analysis_Framework_Suite/ResimulationTool/SOURCE/ApplicationProjects/ResimHTMLReport/Interactiveplot/plotly/hdf_DB/hdf_all/output.h5"
output_dir = "C:/Users/ouymc2/Desktop/Project1"
os.makedirs(output_dir, exist_ok=True)  # Create output directory if missing
output_path = os.path.join(output_dir, os.path.basename(input_path))

def process_dataset(name, dataset, out_file):
    """Extract first 10 rows and save to output file"""
    data = dataset[:1500]  # Get the first 10 rows of the dataset
    if name in out_file:
        del out_file[name]  # Delete existing dataset if it exists
    out_file.create_dataset(name, data=data)  # Create new dataset with only 10 rows

def process_group(parent_name, group, out_file):
    """Recursively process HDF5 groups"""
    for key in group:
        item = group[key]
        full_path = f"{parent_name}/{key}" if parent_name else key
        
        if isinstance(item, h5py.Dataset):
            process_dataset(full_path, item, out_file)  # Process datasets
        elif isinstance(item, h5py.Group):
            out_file.create_group(full_path)  # Create group in output file
            process_group(full_path, item, out_file)  # Recursively process groups

# Main processing
with h5py.File(input_path, 'r') as in_file, \
     h5py.File(output_path, 'w') as out_file:
    
    for root_item in in_file:
        if isinstance(in_file[root_item], h5py.Dataset):
            process_dataset(root_item, in_file[root_item], out_file)  # Process root datasets
        else:
            out_file.create_group(root_item)  # Create root group in output file
            process_group(root_item, in_file[root_item], out_file)  # Process root groups
