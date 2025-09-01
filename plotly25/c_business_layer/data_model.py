class DataContainer:
    def __init__(self):
        self.parsed_data = {}
        self.uni_map = {}
        self.rev_uni_map = {}

    def add_data(self, parsed_data, uni_map, rev_uni_map):
        self.parsed_data.update(parsed_data)
        self.uni_map.update(uni_map)
        self.rev_uni_map.update(rev_uni_map)

    def __repr__(self):
        return f"DataContainer(parsed_data={self.parsed_data}, uni_map={self.uni_map}, rev_uni_map={self.rev_uni_map})"
