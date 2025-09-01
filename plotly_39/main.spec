# -*- mode: python ; coding: utf-8 -*-


a = Analysis(
    ['main.py'],
    pathex=[],
    binaries=[],
    datas=[('InteractivePlot/a_config_layer', './a_config_layer'), ('InteractivePlot/b_persistence_layer', './b_persistence_layer'), ('InteractivePlot/c_business_layer', './c_business_layer'), ('InteractivePlot/d_presentation_layer', './d_presentation_layer')],
    hiddenimports=['lxml', 'h5py', 'plotly'],
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
