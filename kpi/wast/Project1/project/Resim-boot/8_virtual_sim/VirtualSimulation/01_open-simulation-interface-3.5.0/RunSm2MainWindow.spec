# -*- mode: python ; coding: utf-8 -*-


block_cipher = None


a = Analysis(
    ['C:\\Work\\Plastic\\10028634_01_Radar_Sensor_Model\\Tools\\RunSM2_For_OSI_File\\PythonScripts\\RunSm2MainWindow.py'],
    pathex=[],
    binaries=[],
    datas=[],
    hiddenimports=['osi3'],
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
    name='RunSm2MainWindow',
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
    icon=['C:\\Work\\Plastic\\10028634_01_Radar_Sensor_Model\\Tools\\RunSM2_For_OSI_File\\util\\icons\\seo.ico'],
)
