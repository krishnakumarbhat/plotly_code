import pandas as pd
import numpy as np

# Read CSV using pandas (highly optimized C engine)


# Generate the list of column names
columns_to_read = [f'ran_{i}' for i in range(1, 200)]

# Read only the required columns
df = pd.read_csv(r"C:\Users\gjlkfw\Downloads\Wireshark_HIL_Trace_RL_Injection_ORCAS_HIL_OUT_RL_UDP_GEN7_DOWN_SELECTION_STREAM.csv", engine='c', usecols=columns_to_read)


# Convert to 2D NumPy array
data_array = df.to_numpy()

print(data_array)
