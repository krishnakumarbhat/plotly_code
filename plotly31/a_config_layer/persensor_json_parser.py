import json
import os

class PersensorJSONParser:
    def __init__(self, json_file):
        self.json_file = json_file
        self.io_map = {}
        self.parse()

    def parse(self):
        with open(self.json_file, 'r') as json_file:
            json_data = json.load(json_file)
            input_file_groups = [list(item.values())[0] for item in json_data['INPUT_HDF']]
            output_file_groups = [list(item.values())[0] for item in json_data['OUTPUT_HDF']]
            
            # Create a mapping based on the naming convention
            for input_group, output_group in zip(input_file_groups, output_file_groups):
                for input_file_path in input_group:
                    input_filename = os.path.basename(input_file_path).rsplit('.', 1)[0]  # Get filename without extension
                    sensor_suffix = input_filename[-2:]  # Get last two characters (FL or FR)

                    # Find corresponding output file
                    for output_file_path in output_group:
                        output_filename = os.path.basename(output_file_path).rsplit('.', 1)[0]
                        if output_filename.endswith(sensor_suffix):  # this Check if suffix matches
                            self.io_map[input_file_path] = output_file_path
                            break

            # Flatten inputs after mapping
            all_input_files = [file_path for group in input_file_groups for file_path in group]

            if len(self.io_map) == len(all_input_files):
                print("Successfully mapped all input files to output files.")
            else:
                print(f"Mapping incomplete: {len(self.io_map)} out of {len(all_input_files)} inputs mapped.")

    def get_io_map(self):
        return self.io_map
