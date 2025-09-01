import h5py
from collections import deque

def bfs_access_hdf5(file_path):
    with h5py.File(file_path, 'r') as f:
        queue = deque([f])  # Start with the root group
        while queue:
            current_group = queue.popleft()
            print(f"Accessing group: {current_group.name}")
            for key in current_group.keys():
                item = current_group[key]
                if isinstance(item, h5py.Dataset):
                    print(f" - Found dataset: {item.name}, shape: {item.shape}")
                elif isinstance(item, h5py.Group):
                    print(f" - Found subgroup: {item.name}")
                    queue.append(item)  # Add sub-groups to the queue

# Example usage
bfs_access_hdf5(r'C:\Users\ouymc2\Desktop\plotly31\hdf_DB\hdf_per\inputFL.h5')
