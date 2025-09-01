from c_business_layer.data_prep import DataPrep

class DataContainer:
    def __init__(self, data_container, unique_map, data_container_out, unique_map_out, html_name):
        self.data_container = data_container
        self.unique_map = unique_map
        self.data_container_out = data_container_out
        self.unique_map_out = unique_map_out
        self.html_name = html_name
        
        # Initialize DataPrep with the current instance parameters
        self.data_prep = DataPrep(data_container, unique_map, data_container_out, unique_map_out, html_name)

