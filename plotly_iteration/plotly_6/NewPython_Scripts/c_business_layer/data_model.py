class DataContainer:
    def __init__(self):
        self.input_data = []
        self.output_data = []
        self.input_unique_map = {}
        self.output_unique_map = {}
        self.input_reverse_unique_map = {}
        self.output_reverse_unique_map = {}
        self.input_parent_map = {}
        self.ouput_parent_map = {}
        self.lenud = {}

    def add_input_data(self, parsed_data, unique_map, reverse_unique_map ,parent_map ,lenud):
        self.input_data.append(parsed_data)
        self.input_unique_map.update(unique_map)
        self.input_reverse_unique_map.update(reverse_unique_map)
        self.input_parent_map.update(parent_map)
        self.lenud.update(parent_map)

    def add_output_data(self, parsed_data, unique_map, reverse_unique_map ,parent_map ,lenud):
        self.output_data.append(parsed_data)
        self.output_unique_map.update(unique_map)
        self.output_reverse_unique_map.update(reverse_unique_map)
        self.ouput_parent_map.update(parent_map)
        self.lenud.update(parent_map)

    # def __repr__(self):
    #     return f"Input Data: {self.input_data}, Output Data: {self.output_data}"
