#!/usr/bin/env python3
"""
Memory profiling script for InteractivePlot
Run with: python -m memory_profiler run_with_profiler.py
"""
# Import your main modules that start the processing
# This is just an example - adjust to your actual entry point
from InteractivePlot.d_business_layer.data_prep import DataPrep
# Import any other necessary components to initialize your data objects

def main():
    # Initialize your data objects
    # For example:
    # input_data = ...
    # output_data = ...
    
    # Create DataPrep instance with profiled methods
    # data_prep = DataPrep(input_data, output_data, "memory_profile_test", "your_sensor", "your_stream")
    
    # The generate_plots method will be profiled automatically
    # due to the @profile decorator we added
    
    print("Memory profiling complete. Check the results in the terminal output.")

if __name__ == "__main__":
    main()
