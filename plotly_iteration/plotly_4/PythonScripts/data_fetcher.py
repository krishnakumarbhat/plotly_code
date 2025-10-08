import os
import zmq
from multiprocessing import Queue

class DataFetcher:
    """Class responsible for fetching and processing HDF5 data."""
    debug_mode =False

    def __init__(self, shared_queue: Queue) -> None:
        """Initialize the DataFetcher with ZeroMQ context and socket."""
        DataFetcher.debug_mode = self.set_debug_mode() 
        self.queue = shared_queue

        if DataFetcher.debug_mode:
            print("Debug mode activated.")
            from test_web import Timer

    def set_debug_mode(self) -> bool:
        """Set the debug mode based on user input."""
        mode = input("Enter mode \n 1 for debug \n 2 for release: ").strip()
        return mode == '1'

    def run(self) -> None:
        """Listen for incoming HDF5 file paths and enqueue them."""
        
        # Set up ZeroMQ context and socket
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.PULL)
        self.socket.connect("tcp://localhost:5555")
        
        # Receive the first message to get the total number of files
        file_count_message = self.socket.recv_string()  
        try:
            total_files = int(file_count_message)
            print(f"Total number of HDF files to process: {total_files}")
            
        except ValueError:
            print("Received invalid file count. Exiting...")
            
        received_files_count = 0
        while received_files_count < total_files:
            hdf_path = self.socket.recv_string()  # Receive HDF file path
            
            received_files_count += 1
            print(f"Received HDF file path: {hdf_path} (Total files received: {received_files_count})")
            
            # Put the HDF path into the shared queue
            self.queue.put(hdf_path)

            print(f"Current queue size: {self.queue.qsize()}")

if __name__ == "__main__":
    fetcher = DataFetcher()
    fetcher.run()
