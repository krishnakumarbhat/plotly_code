
import h5py

def compress_hdf(input_file, output_file):
    # Open the input HDF file in read mode
    with h5py.File(input_file, 'r') as input_hdf:
        # Create a new HDF file in write mode with compression
        with h5py.File(output_file, 'w') as output_hdf:
            # Function to recursively copy datasets and groups
            def copy_group(source, destination):
                for key in source:
                    item = source[key]
                    if isinstance(item, h5py.Group):
                        # Create a new group in the destination
                        new_group = destination.create_group(key)
                        # Recursively copy the group
                        copy_group(item, new_group)
                    elif isinstance(item, h5py.Dataset):
                        # Copy the dataset with compression
                        destination.create_dataset(
                            key,
                            data=item[...],  # Read the data
                            compression='gzip',  # Use gzip compression
                            compression_opts=9,  # Maximum compression
                            chunks=True  # Enable chunking
                        )

            # Start copying from the root group
            copy_group(input_hdf, output_hdf)

# Example usage
input_hdf_file ='C:/git/10030156_03_Analysis_Framework_Suite/ResimulationTool/SOURCE/ApplicationProjects/ResimHTMLReport/Interactiveplot/plotly/hdf_DB/hdf_all/R607_out.h5'
output_hdf_file_name = 'R607_out.h5'  # Desired output file name
compress_hdf(input_hdf_file, output_hdf_file_name)
