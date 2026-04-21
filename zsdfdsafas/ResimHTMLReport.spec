# -*- mode: python ; coding: utf-8 -*-
block_cipher = None

from PyInstaller.building.build_main import Analysis, PYZ, EXE
from PyInstaller.utils.hooks import collect_submodules, collect_data_files
import os

# Collect hidden imports from the InteractivePlot package and protobuf-generated modules
hiddenimports = collect_submodules('InteractivePlot') + [
    'kpi_client.hdf_pb2',
    'kpi_client.hdf_add_pb2',
]

# Collect data files: prefer collect_data_files (for installed packages),
# fallback to walking local package directories if needed.
datas = []
try:
    datas += collect_data_files('InteractivePlot') or []
except Exception:
    pass
try:
    datas += collect_data_files('kpi_client') or []
except Exception:
    pass

def add_tree(src, dest_prefix):
    if not os.path.isdir(src):
        return
    for root, _, files in os.walk(src):
        for f in files:
            srcpath = os.path.join(root, f)
            rel = os.path.relpath(root, src)
            dest = os.path.join(dest_prefix, rel) if rel != '.' else dest_prefix
            # Normalize to forward slashes for PyInstaller
            datas.append((srcpath, dest.replace('\\', '/')))

# Ensure local package directories are included when collect_data_files didn't find them
add_tree('InteractivePlot', 'InteractivePlot')
add_tree('kpi_client', 'kpi_client')

a = Analysis([
    'ResimHTMLReport.py',
],
    pathex=['.'],
    binaries=[],
    datas=datas,
    hiddenimports=hiddenimports,
    hookspath=[],
    runtime_hooks=[],
    excludes=[],
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=block_cipher,
)

pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)

# Build the executable
exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.zipfiles,
    a.datas,
    [],
    name='ResimHTMLReport',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    console=True,
)
