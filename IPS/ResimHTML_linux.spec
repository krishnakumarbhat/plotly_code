
# -*- mode: python ; coding: utf-8 -*-

from PyInstaller.utils.hooks import collect_submodules, collect_data_files
import matplotlib
import os

# --- Plotly: collect dynamic modules & data ---
plotly_hidden = collect_submodules('plotly')
plotly_data   = collect_data_files('plotly')

# --- Matplotlib: explicitly collect mpl-data ---
# PyInstaller usually collects mpl-data via its hook, but to be safe we add it explicitly.
mpl_hidden = collect_submodules('matplotlib')
mpl_data   = collect_data_files('matplotlib')

# Extra belt-and-suspenders: include the mpl-data folder path explicitly
mpl_data_path = matplotlib.get_data_path()
# Bundle entire mpl-data under 'matplotlib/mpl-data' in the app
mpl_data_explicit = [(mpl_data_path, os.path.join('matplotlib', 'mpl-data'))]

a = Analysis(
    ['ResimHTMLReport.py'],
    pathex=[],              # add your source roots if needed (e.g., ['.', 'IPS'])
    binaries=[],
    datas=plotly_data + mpl_data + mpl_data_explicit,  # ensure mpl-data is present
    hiddenimports=plotly_hidden + mpl_hidden + [
        # Common entry points for Plotly/Matplotlib
        'plotly', 'plotly.io', 'plotly.graph_objs', 'plotly.subplots',
        'matplotlib', 'matplotlib.pyplot',
        'matplotlib.backends.backend_agg',  # safe non-GUI backend for headless
    ],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=['_runtime_mpl_fix.py'],  # critical: see hook below
    excludes=[],
    noarchive=False,
    optimize=0,
)

pyz = PYZ(a.pure, a.zipped_data, cipher=None)

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
    upx=False,              # keep False until stable; UPX can break resource loading
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,           # keep True while debugging/container runs
)
``

