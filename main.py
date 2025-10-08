import os
import shutil
import subprocess

# --- Configuration ---
# The path to the 'github' subdirectory
github_dir = "github"

# You may need to specify the branch if not 'main' or 'master'
git_branch = "main"

# --- Functions ---
def run_git_command(command):
    """Executes a git command and returns success status."""
    try:
        subprocess.run(
            command,
            check=True,
            text=True,
            capture_output=True,
            shell=True
        )
        print(f"✅ Success: {' '.join(command)}")
        return True
    except subprocess.CalledProcessError as e:
        print(f"❌ Error during command: {' '.join(e.cmd)}")
        print(f"  Error message: {e.stderr}")
        return False

# --- Main Script ---
if __name__ == "__main__":
    current_working_dir = os.getcwd()
    github_path = os.path.join(current_working_dir, github_dir)

    if not os.path.isdir(github_path):
        print(f"❌ Error: The '{github_dir}' directory does not exist.")
        exit()

    print("Starting automated Git push process...")
    
    # Get all 'plotly' folders from the github subdirectory
    plotly_dirs = [d for d in os.listdir(github_path) if d.startswith('plotly') and os.path.isdir(os.path.join(github_path, d))]
    
    # Sort them numerically for a clean commit history
    import re
    plotly_dirs.sort(key=lambda x: int(re.sub(r'plotly_?', '', x)))

    for folder_name in plotly_dirs:
        source_folder_path = os.path.join(github_path, folder_name)
        
        print(f"\nProcessing '{folder_name}'...")
        
        # Define the destination path (the current working directory)
        destination_path = os.path.join(current_working_dir, folder_name)

        try:
            # Move the folder from the 'github' subdirectory to the current directory
            print(f"📂 Moving '{source_folder_path}' to '{destination_path}'...")
            shutil.move(source_folder_path, destination_path)

            # Change to the repository root to run git commands
            os.chdir(current_working_dir)
            
            # Remove the now empty 'github' folder if it is empty to avoid pushing empty directories
            if not os.listdir(github_path):
                print(f"📁 The '{github_dir}' folder is now empty. Removing it.")
                os.rmdir(github_path)
            
            # Add the folder to the staging area
            add_command = f"git add {folder_name}"
            if not run_git_command(add_command):
                continue
            
            # Create the commit message
            commit_message = f"Pushed {folder_name}"
            commit_command = f'git commit -m "{commit_message}"'
            if not run_git_command(commit_command):
                continue
            
            # Push the commit to the remote repository
            push_command = f"git push origin {git_branch}"
            if not run_git_command(push_command):
                continue
                
            print(f"✅ Successfully pushed '{folder_name}'.")

        except shutil.Error as e:
            print(f"❌ Error moving folder '{folder_name}': {e}")
            continue
        except Exception as e:
            print(f"❌ An unexpected error occurred: {e}")
            continue

    print("\nScript execution finished.")