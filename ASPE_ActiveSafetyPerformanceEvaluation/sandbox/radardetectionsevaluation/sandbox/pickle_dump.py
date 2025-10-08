import pickle
import pandas as pd
import numpy as np
data_path = r'private\.pickle'

data_raw = np.random.rand(300000, 100)
data = pd.DataFrame(data_raw)

with open(data_path, 'wb') as handle:
    pickle.dump(data, handle, protocol=2)