class SignalValueMapper:
    """A class to map values to signals and signals to values for fast access."""

    def __init__(self):
        self.val_sig_map_in = {}  # Maps values to signals
        self.sig_val_map_in = {}  # Maps signals to values and their can be duplicate in this 
        self.val_sig_map_out = {}  # Maps values to signals
        self.sig_val_map_out = {}  # Maps signals to values and their can be duplicate in this 

    @staticmethod
    def set_mapping(self, value, signal ,parent):
        """Add a mapping from value to signal and vice versa."""
        if signal in sig_val_map.keys():
            self.sig_val_map[signal].append({parent:value})
            self.sig_val_map[signal] = value
        self.val_sig_map[value] = signal
        
    @staticmethod
    def get_signal(self, value , type_of_data):
        """Retrieve the signal associated with a given value."""
        return self.val_sig_map.get(value, None)

    def get_value(self, signal , parent,type_of_data):
        """Retrieve the value associated with a given signal."""
        return self.sig_val_map.get(signal, None)

    def remove_mapping(self, value,type_of_data):
        """Remove a mapping by value."""
        signal = self.val_sig_map.pop(value, None)
        if signal:
            self.sig_val_map.pop(signal, None)

    def clear_mappings(self):
        """Clear all mappings."""
        self.val_sig_map.clear()
        self.sig_val_map.clear()

    def __repr__(self):
        return f"SignalValueMapper(val_sig_map={self.val_sig_map}, sig_val_map={self.sig_val_map})"
