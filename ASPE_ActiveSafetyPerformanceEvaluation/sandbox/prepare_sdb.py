import shutil
from pathlib import Path

from tqdm import tqdm

sdb_dir = Path(r'C:\Users\hjyjks\Downloads\1k_occlusion_output')
target_dir = Path(r'E:\1k\data\FASETH\ASP_output\SDB_cache')

label_sets = list(sdb_dir.iterdir())
for sub_dir in tqdm(label_sets):
    try:
        src_sdb_pkl, = sub_dir.glob('*.pkl')
        sdb_pkl_id = sub_dir.name
        dst_sdb_pkl = target_dir / f'log_{sdb_pkl_id}_sdb_raw.pickle'
        if not dst_sdb_pkl.exists():
            shutil.copy(src_sdb_pkl, dst_sdb_pkl)
    except Exception:
        pass
