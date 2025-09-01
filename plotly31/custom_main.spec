# -*- mode: python ; coding: utf-8 -*-

block_cipher = None

a = Analysis(
    ['main.py'],
    pathex=[],
    binaries=[],
    datas=[
        ('a_config_layer', 'a_config_layer'),
        ('b_db_layer', 'b_db_layer'),
        ('c_business_layer', 'c_business_layer'),
        ('d_presentation_layer', 'd_presentation_layer'),
        ('ConfigInteractivePlots.xml', '.'),
        ('InputsInteractivePlot.json', '.'),
        ('InputsPerSensorInteractivePlot.json', '.')
    ],
    hiddenimports=['lxml', 'h5py', 'plotly', 'numpy', 'pandas'],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=block_cipher,
    noarchive=False,
)

pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.zipfiles,
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
