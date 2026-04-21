import json
from pathlib import Path
import h5py
import numpy as np

root = Path('.')
cfg = json.loads((root / 'InputsInteractivePlot_first3.json').read_text(encoding='utf-8'))
in_path = Path(cfg['INPUT_HDF'][0])
out_path = Path(cfg['OUTPUT_HDF'][0])

keywords = ('ran', 'range')

def inspect_file(path: Path, label: str):
    print('\n===', label, path.name, '===')
    with h5py.File(path, 'r') as h5:
        def visit(name, obj):
            if isinstance(obj, h5py.Dataset):
                lname = name.lower()
                if any(k in lname for k in keywords):
                    try:
                        arr = obj[()]
                        arr = np.asarray(arr)
                        flat = arr.astype(np.float64, copy=False).ravel() if arr.size else np.array([], dtype=np.float64)
                        finite = np.isfinite(flat)
                        print(name, 'shape', arr.shape, 'dtype', arr.dtype, 'finite', int(finite.sum()), 'total', flat.size)
                    except Exception as e:
                        print(name, 'read_error', e)
        h5.visititems(visit)

inspect_file(in_path, 'INPUT')
inspect_file(out_path, 'OUTPUT')
