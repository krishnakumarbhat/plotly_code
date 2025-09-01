# -*- mode: python ; coding: utf-8 -*-
from PyInstaller.utils.hooks import collect_data_files
from PyInstaller.utils.hooks import collect_all

datas = [('a_config_layer', 'a_config_layer'), ('b_db_layer', 'b_db_layer'), ('c_business_layer', 'c_business_layer'), ('d_presentation_layer', 'd_presentation_layer'), ('ConfigInteractivePlots.xml', '.'), ('InputsInteractivePlot.json', '.'), ('InputsPerSensorInteractivePlot.json', '.')]
binaries = []
hiddenimports = ['lxml', 'h5py', 'plotly']
datas += collect_data_files('plotly')
tmp_ret = collect_all('plotly')
datas += tmp_ret[0]; binaries += tmp_ret[1]; hiddenimports += tmp_ret[2]


a = Analysis(
    ['main.py'],
    pathex=[],
    binaries=binaries,
    datas=datas,
    hiddenimports=hiddenimports,
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='main',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)
