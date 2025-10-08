import sys
from pathlib import Path
from shutil import copy

from tqdm import tqdm

input_dir = Path(sys.argv[1])
if sys.argv[2] == '':
    output_dir = input_dir.parent / 'all_MF4'
else:
    output_dir = Path(sys.argv[2])
output_dir.mkdir(exist_ok=True)

mdf_list = list(input_dir.rglob('*_fas_*.mf4'))
if input(f'Found {len(mdf_list)} MF4 files. Copy?\n') == 'y':
    for src_path in tqdm(mdf_list, 'Copying'):
        dst_path = output_dir / src_path.name
        copy(src_path, dst_path)
result_list = list(output_dir.rglob('*_fas_*.mf4'))
print(f'Result: {len(result_list)}/{len(mdf_list)}')
