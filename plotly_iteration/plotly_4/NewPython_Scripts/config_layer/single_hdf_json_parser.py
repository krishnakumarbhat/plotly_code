import json

class SingleHDFJSONParser:
    def __init__(self, json_file):
        self.json_file = json_file
        self.io_map = {}
        self.parse()

    def parse(self):
        with open(self.json_file, 'r') as file:
            data = json.load(file)
            inputs = data['INPUT_HDF']
            outputs = data['OUTPUT_HDF']
            if len(inputs) == len(outputs):
                print("Matching input and output files.")
                self.io_map = dict(zip(inputs, outputs))
            else:
                print(f"Input and output file counts do not match: {len(inputs)} != {len(outputs)}")

    def get_io_map(self):
        return self.io_map
