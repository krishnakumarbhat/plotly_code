import os
import platform
import pandas as pd
import psutil
import zmq
from multiprocessing import Process, Queue
import time

current_time = time.time()

def run_script_in_terminal(script_name: str) -> None:
    """Run a script in a new terminal window based on the OS."""
    commands = {
        "Windows": f'start cmd /k python {script_name}',
        "Linux": f'gnome-terminal -- python3 {script_name}',
        "Darwin": f'open -a Terminal {script_name}'
    }
    command = commands.get(platform.system())
    if command:
        os.system(command)
    else:
        print("Unsupported OS")

def get_cpu_info() -> int:
    """Get the optimal number of processes to use based on CPU info."""
    physical_cores = psutil.cpu_count(logical=False)
    cpu_utilization = psutil.cpu_percent(interval=1)
    num_processes = max(1, physical_cores - (physical_cores * (cpu_utilization + 10) // 100))
    print(f"Calculated number of processes: {num_processes}, CPU Utilization: {cpu_utilization}%")
    return num_processes

def convert_csv_to_hdf5(csv_file: str, output_directory: str = 'data_hdf') -> str:
    """Convert a CSV file to HDF5 format."""
    try:
        df = pd.read_csv(csv_file)
        hdf_file = os.path.join(output_directory, os.path.basename(csv_file).replace('.csv', '.h5'))
        df.to_hdf(hdf_file, key='data', mode='w')
        absolute_hdf_file = os.path.abspath(hdf_file)
        return absolute_hdf_file
    except Exception as e:
        print(f"Failed to convert {csv_file}: {e}")
        return None

def worker(queue: Queue, csv_files: list) -> None:
    """Worker function to convert CSV files to HDF5."""
    output_directory = 'data_hdf'
    os.makedirs(output_directory, exist_ok=True)

    for csv_file in csv_files:
        csv_file = csv_file.strip().strip('"')
        if csv_file:
            hdf_file = convert_csv_to_hdf5(csv_file, output_directory)
            if hdf_file:
                queue.put(hdf_file)

def push_hdf_addresses(queue: Queue) -> None:
    """Push HDF file addresses to a ZeroMQ socket."""
    context = zmq.Context()
    socket = context.socket(zmq.PUSH)
    socket.bind("tcp://*:5555")  # Server endpoint
    print("ZMQ process started and waiting for data...")

    while True:
        hdf_file = queue.get()
        if hdf_file is None:
            print("Received signal to exit.")
            break
        try:
            print(f"Sending HDF file address over ZMQ: {hdf_file}")
            if type(hdf_file) == int:
                hdf_file = str(hdf_file)
            socket.send_string(hdf_file)
        except Exception as e:
            print(f"Failed to send HDF file address: {e} hi {hdf_file}")
    
def main() -> None:
    """Main function to orchestrate the CSV to HDF5 conversion and ZMQ communication."""
    log_file = 'logs.txt'
    # run_script_in_terminal('application.py')
    with open(log_file, 'r') as file:
        csv_files = [line.strip().strip('"') for line in file if line.strip()]
    
    num_files = len(csv_files)
    num_processes = get_cpu_info()
    
    num_processes_to_use = min(num_processes, num_files)
    
    queue = Queue()
    
    queue.put(num_files)# Send the number of files first
    
    # Start ZeroMQ address push process
    zmq_process = Process(target=push_hdf_addresses, args=(queue,))
    zmq_process.start()
    
    # Start worker processes for CSV to HDF conversion
    if  num_processes_to_use > 4:
            num_processes_to_use = 4
    else:
        num_processes_to_use = num_processes_to_use
    files_per_worker = num_files // num_processes_to_use
    workers = []
    for i in range(num_processes_to_use):
        start_index = i * files_per_worker
        end_index = (i + 1) * files_per_worker if i != num_processes_to_use - 1 else num_files
        p = Process(target=worker, args=(queue, csv_files[start_index:end_index]))
        p.start()
        workers.append(p)
    
    for p in workers:
        p.join()
        
    zmq_process.join()
    end = time.time()
    print(f"\n hi {end - current_time}")

if __name__ == "__main__":
    main()
