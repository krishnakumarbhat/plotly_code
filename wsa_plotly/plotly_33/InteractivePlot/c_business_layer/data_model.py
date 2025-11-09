from c_business_layer.data_prep import DataPrep

class DataContainer:
    def __init__(self, data_container_in, val_sig_map_in, data_container_out, val_sig_map_out, html_name):
        self.data_container_in = data_container_in
        self.val_sig_map_in = val_sig_map_in
        self.data_container_out = data_container_out
        self.val_sig_map_out = val_sig_map_out
        self.html_name = html_name
        
        # Initialize DataPrep with the current instance parameters
        self.data_prep = DataPrep(data_container_in, val_sig_map_in, data_container_out, val_sig_map_out, html_name)

