import sys
import os

# Add the build directory to the path
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'build'))

import python_plotter

def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <hdf5_file>")
        return

    hdf5_file = sys.argv[1]

    try:
        parser = python_plotter.HDF5Parser(hdf5_file)
        data_storage = parser.parse()

        # Example of how to access data and plot
        # This assumes you know the names of the datasets you want to plot
        x_key = '/Stream_0/x_data'
        y_key = '/Stream_0/y_data'

        if x_key in data_storage.data and y_key in data_storage.data:
            x_data = data_storage.data[x_key]
            y_data = data_storage.data[y_key]
            python_plotter.create_scatter_plot(x_data, y_data, "My Scatter Plot from Python", "X-Axis", "Y-Axis")
        else:
            print(f"Could not find datasets {x_key} and {y_key} in the HDF5 file.")

    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    main()