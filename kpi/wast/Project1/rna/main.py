import os

def create_folders(base_path, num_folders, sub_folders_per_folder):
    # Create the base path if it doesn't exist
    if not os.path.exists(base_path):
        os.makedirs(base_path)

    for i in range(1, num_folders + 1):
        # Create the main folder
        main_folder = os.path.join(base_path, f'Folder_{i}')
        os.makedirs(main_folder)

        for j in range(1, sub_folders_per_folder + 1):
            # Create subfolders within the main folder
            sub_folder = os.path.join(main_folder, f'Subfolder_{j}')
            os.makedirs(sub_folder)

# Define parameters
base_directory = 'BaseFolder'  # Change this to your desired base directory
num_folders = 7                 # Number of main folders
sub_folders_per_folder = 2      # Number of subfolders in each main folder

# Create the folders
create_folders(base_directory, num_folders, sub_folders_per_folder)

print(f'{num_folders} folders with {sub_folders_per_folder} subfolders each created in "{base_directory}".')