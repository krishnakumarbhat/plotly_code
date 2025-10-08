import json
import math
import os.path
import re
import subprocess
import warnings
from pathlib import Path
from typing import Dict, Iterable, Optional

import pandas as pd


class AsperaConnect:
    """
    Wrapper for Aspera CLI to perform commonly used operations from python code
    To make use of this class Aspera CLI tool needs to be downloaded from IBM website
    or HPC Support sharepoint: https://hpc-apps.aptiv.com/KB
    Unzip the folder in your selected location and add cli/bin folder to system path
    """

    def __init__(self, netid: str,
                 password: str,
                 host_address: str = "aspera-emea.aptiv.com"):
        """
        AsperaConnect object initializer
        Args:
            netid: username used for aspera login
            password: password used for aspera login
            host_address: host address server (usually 'aspera-emea.aptiv.com' or 'aspera-na.aptiv.com')
        """
        self.netid = netid
        self.password = password
        self.host_address = host_address

    def download(self, source: str,
                 destination: str,
                 timeout: int = None) -> str:
        """
        Download selected source file or directory from aspera to local destination directory
        Args:
            source: hpcc path of folder or file to download
            destination: local directory to save to
            timeout: max time in seconds to perform the operation
        """
        filename = os.path.basename(source)
        file_path = os.path.join(destination, filename)
        if os.path.isfile(file_path):
            print(f"Download skipped: {filename} already exists in {destination}")
        else:
            Path(destination).mkdir(exist_ok=True, parents=True)
            command = ["aspera", "shares", f"download", "-i", f"--host={self.host_address}", f"--source={source}",
                       f"--destination={destination}", f"--password={self.password}", f"--user={self.netid}"]
            print(f'Downloading: {source} to: {destination}')
            try:
                subprocess.run(command, shell=True, stdout=subprocess.PIPE, timeout=timeout, check=True)
            except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as e:
                e_pass_hidden = re.sub(r"(?<=--password=).+?'", "*****", str(e))
                warnings.warn(f'File download did not complete successfully: {source}\n\t{e_pass_hidden}')
            if not os.path.isfile(file_path):
                file_path = None
        return file_path

    def upload(self, source: str,
               destination: str,
               timeout: int = None) -> None:
        """
        Upload file or directory from local path to destination on aspera
        Args:
            source: local path of file or folder to be uploaded
            destination: folder on aspera to upload file/folder to
            timeout: max time in seconds to perform the operation
        """
        command = ["aspera", "shares", f"upload", "-i", f"--host={self.host_address}", f"--source={source}",
                   f"--destination={destination}", f"--password={self.password}", f"--username={self.netid}"]
        print(f'Uploading: {source} to: {destination}')
        try:
            subprocess.run(command, shell=True, stdout=subprocess.PIPE, timeout=timeout, check=True)
        except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as e:
            e_pass_hidden = re.sub(r"(?<=--password=).+?'", "*****", str(e))
            warnings.warn(f'File upload did not complete successfully: {source}\n\t{e_pass_hidden}')

    def browse(self, directory: Iterable[str],
               recursive_depth: int = 1,
               result: Optional[pd.DataFrame] = None) -> pd.DataFrame:
        """
        List files and directories in aspera directory
        Args:
            directory: list of directories to be explored
            recursive_depth: list files and folders recursively up to this level, by default (with recursive_depth
                                 parameter set to 1) only base directory contents are listed
            result: used only internally for recursive function execution
        Returns:
            DataFrame containing list of files and folders found and their basic properties
        """
        if result is None:
            result = pd.DataFrame()
        if isinstance(directory, str):
            directory = [directory]

        if recursive_depth > 0:
            for directory in directory:
                command = ["aspera", "shares", f"browse", "-i", f"--host={self.host_address}", f"--path={directory}",
                           f"--password={self.password}", f"--user={self.netid}", "-j"]
                print(f'Exploring directory: {directory}')

                try:
                    process = subprocess.run(command, shell=True, stdout=subprocess.PIPE, timeout=10, check=True)
                    stdout_message = process.stdout.decode('utf-8')
                    stdout_message_json = self._extract_json_message(stdout_message)
                    all_elements = pd.DataFrame(stdout_message_json['items'])
                except (subprocess.CalledProcessError, subprocess.TimeoutExpired):
                    all_elements = pd.DataFrame()
                    warnings.warn(f'Directory contents could not be retrieved: {directory}')

                if not all_elements.empty:
                    directories_local = all_elements[all_elements['type'] == 'directory']
                    result = result.append(all_elements)
                    result = self.browse(directories_local.path,
                                         recursive_depth=recursive_depth - 1,
                                         result=result)
        return result

    def find_and_download(self, filenames: Iterable[str],
                          root_dir: str,
                          destination_dir: str,
                          recursive_depth: int = math.inf,
                          only_get_ext: Iterable[str] = ['']) -> pd.DataFrame:
        """
        Find files recursively starting from root_dir and download them into a single local directory
        Args:
            filenames: filenames (or parts of filenames) to be downloaded
            root_dir: aspera directory to begin recursive search from
            destination_dir: local directory to download files to
            recursive_depth: max depth of recursive search
            only_get_ext: specify file extensions that should be downloaded, by default all extensions are accepted
        Returns:
            DataFrame listing downloaded files, their aspera locations and basic properties
        """
        if isinstance(filenames, str):
            filenames = [filenames]

        all_elements = self.browse(directory=[root_dir], recursive_depth=recursive_depth)
        filename_filter = all_elements['basename'].apply(
            lambda x: any(filename in os.path.splitext(x)[0] for filename in filenames))
        ext_filter = all_elements['basename'].apply(
            lambda x: any(ext in os.path.splitext(x)[1] for ext in only_get_ext))
        files_to_download = all_elements[filename_filter & ext_filter]
        print(f"Number of files found: {len(files_to_download)}")
        for aspera_filepath in files_to_download['path']:
            self.download(source=aspera_filepath, destination=destination_dir)

        return files_to_download

    def _extract_json_message(self, message: str) -> Dict:
        """
        Find JSON message in a string and return as dictionary
        Args:
            message: string to be read
        Returns:
            Dictionary with decoded JSON structure
        """
        json_start = message.index("{")
        json_end = len(message) - message[::-1].index("}")
        output_message_json = json.loads(message[json_start:json_end])
        return output_message_json


if __name__ == '__main__':
    # Example of usage
    import math

    import keyring

    netid = "put_your_netid_here"
    # Either replace with hard coded password (insecure) or set password
    # if keyring - use command line 'keyring set aspera <username>', otherwise simply put password
    password = keyring.get_password("aspera", netid)
    host_address = "aspera-emea.aptiv.com"  # 'aspera-na.aptiv.com'
    aspera_path = r"/F360CORE-USERS/fromPiotr"
    local_path = r"C:\Users\xj1cjd\Logfiles\New2"
    files_to_download = ["WLQ4_20210616_MUN_MUN_085159_064", "WLQ4_20210616_MUN_MUN_085159_070"]
    files_to_download_substring = r"WLQ4_20210616_"

    aspera = AsperaConnect(netid=netid, password=password, host_address=host_address)

    aspera.upload(source=local_path, destination=aspera_path)  # source can be a file or a folder
    aspera.download(source=aspera_path, destination=local_path)  # source can be a file or a folder
    dir_contents = aspera.browse(directory=aspera_path)  # browse only base directory
    dir_contents_recursive = aspera.browse(directory=aspera_path, recursive_depth=math.inf)  # browse recursively

    files_found_1 = aspera.find_and_download(filenames=files_to_download,
                                             root_dir=aspera_path,
                                             destination_dir=local_path,
                                             only_get_ext=['.mudp', '.dvl'])

    files_found_2 = aspera.find_and_download(filenames=files_to_download_substring,
                                             root_dir=aspera_path,
                                             destination_dir=local_path,
                                             recursive_depth=2)
