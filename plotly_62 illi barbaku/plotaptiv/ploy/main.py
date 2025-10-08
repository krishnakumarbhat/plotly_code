import sys
import os
from InteractivePlot.main_processor import MainProcessor

if __name__ == "__main__":
    if len(sys.argv) < 3 or len(sys.argv) > 4:
        print("Usage: python script.py <config_file> <input_plot_json_file> [output_directory]")
        sys.exit(1)

    config_file = sys.argv[1]
    input_plot_json_file = sys.argv[2]
    output_directory = sys.argv[3] if len(sys.argv) == 4 else None

    print(f"Starting Interactive Plot System")
    print(f"  - Config file: {config_file}")
    print(f"  - Input file: {input_plot_json_file}")
    print(f"  - Output directory: {output_directory or 'html'}")
    
    try:
        processor = MainProcessor(config_file, input_plot_json_file, output_directory)
        processor.run()
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)
