import numpy as np

@staticmethod
def get_value(data, signal_name, grp_name=None):
    """
    Get all data values for a specific signal, returned as a list of [scan_index, value(s)] pairs.
    (Docstring remains the same)
    """
    signal_info = data._signal_to_value.get(signal_name)

    if not signal_info:
        return [], f"Signal '{signal_name}' not found in data model."

    # --- Improved: Clearer logic for finding the storage key ---
    storage_key = None
    if isinstance(signal_info, str):
        storage_key = signal_info
    elif isinstance(signal_info, list):
        if grp_name:
            # Find the key for the specified group
            for item in signal_info:
                if grp_name in item:
                    storage_key = item[grp_name]
                    break
        elif signal_info:
            # Default to the first available key if no group is specified
            storage_key = list(signal_info[0].values())[0]

    if not storage_key:
        return [], f"Could not find a valid storage key for signal '{signal_name}' with group '{grp_name}'."

    try:
        # Using a list comprehension is slightly more modern than map()
        grp_idx, plt_idx = [int(i) for i in storage_key.split("_")]
    except (ValueError, AttributeError):
        return [], f"Invalid storage key format '{storage_key}' for signal '{signal_name}'."

    # --- More efficient list comprehension to get all values ---
    all_values = []
    for scan_idx, scan_data in data._data_container.items():
        # Use .get() with a default to avoid nested ifs
        group_data = scan_data.get(grp_idx)
        if group_data and plt_idx < len(group_data):
            value = group_data[plt_idx]
            if np.isscalar(value):
                all_values.append([scan_idx, value])
            else:
                # Use .tolist() for NumPy arrays, which is safer than list()
                all_values.append([scan_idx] + value.tolist() if hasattr(value, 'tolist') else [scan_idx] + list(value))
    
    return all_values, "success"