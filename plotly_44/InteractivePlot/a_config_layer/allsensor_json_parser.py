import json

class AllsensorJSONParser:
    """Parser for JSON files containing data for all sensors"""
    def __init__(self, json_file):
        """
        Initialize parser with configuration file path.
        
        Args:
            config_file: Path to JSON configuration file
        """
        self.json_file = json_file
        self.io_map = {}
        self.parse()

    def parse(self):
        """
        Parse JSON data according to all-sensor format requirements."
        """
		
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
