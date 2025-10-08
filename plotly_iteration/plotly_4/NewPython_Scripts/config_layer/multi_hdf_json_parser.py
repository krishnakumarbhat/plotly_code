import json

class MultiHDFJSONParser:
    def __init__(self, json_file):
        self.json_file = json_file
        self.io_map = {}
        self.parse()

    def parse(self):
        with open(self.json_file, 'r') as file:
            data = json.load(file)
            inputs = [list(item.values())[0] for item in data['INPUT_HDF']]
            outputs = [list(item.values())[0] for item in data['OUTPUT_HDF']]
            
            # Flattening the lists and creating a mapping
            flat_inputs = [item for sublist in inputs for item in sublist]
            flat_outputs = [item for sublist in outputs for item in sublist]
            if len(flat_inputs) == len(flat_outputs):
                print("Matching input and output files.")
                self.io_map = dict(zip(flat_inputs, flat_outputs))
            else:
                print(f"Input and output file counts do not match: {len(inputs)} != {len(outputs)}")

    def get_io_map(self):
        return self.io_map
