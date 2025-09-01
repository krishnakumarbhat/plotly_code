import h5py
from collections import deque

# Create a list to store datasets found in this group
datasets = []
stack_grp = deque()
def print_hdf5_structure(group, indent=0):
    """Print the structure of an HDF5 group with proper indentation."""
    # Print the current group name with indentation
    print("  " * indent + f"Group: {group.name}")

    # Iterate through all items in the group
    for item_name in group:
        item = group[item_name]
        if isinstance(item, h5py.Dataset):
            # Store dataset details in the list
            datasets.append(f"- Dataset: {item_name}, Shape: {item.shape}")
        elif isinstance(item, h5py.Group):
            # Add child groups to the stack for later processing
            stack_grp.append(item)
    
    # Print all datasets found in this group
    while datasets:
        dataset_info = datasets.pop(0)  # Use pop(0) to maintain order
        print(f"{dataset_info} with parent {group.name}")

    # Recursively print the structure of child groups
    while stack_grp:
        child_group = stack_grp.popleft()  # Use popleft() for FIFO order
        print_hdf5_structure(child_group, indent + 1)  # Increase indentation for nested groups

def access_hdf5(file_path):
    """Open the HDF5 file and start printing its structure."""
    with h5py.File(file_path, 'r') as f:
        hdf_file = f["data"]
        print_hdf5_structure(hdf_file)

# Example usage
access_hdf5(r'C:\Users\ouymc2\Desktop\plotly31\hdf_DB\hdf_per\inputFR.h5')
