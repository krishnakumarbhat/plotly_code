# -*- mode: python ; coding: utf-8 -*-
 
 
a = Analysis(
    ['ResimHTMLReport.py'],
    pathex=[],
    binaries=[],
    datas=[('InteractivePlot/a_config_layer', './InteractivePlot/a_config_layer'), 
           ('InteractivePlot/b_persistence_layer', './InteractivePlot/b_persistence_layer'), 
           ('InteractivePlot/c_data_storage', './InteractivePlot/c_data_storage'), 
           ('InteractivePlot/d_business_layer', './InteractivePlot/d_business_layer'), 
           ('InteractivePlot/e_presentation_layer', './InteractivePlot/e_presentation_layer'), 
           ('KPI/a_business_layer', './KPI/a_business_layer'), 
           ('KPI/b_presentation_layer', './KPI/b_presentation_layer')],
    hiddenimports=['lxml', 'h5py', 'plotly', 
                  'InteractivePlot.d_business_layer.data_cal',
                  'InteractivePlot.d_business_layer',
                  'pandas', 'numpy', 'pytz', 'six','datashader','memory_profiler'
                  'win32api', 'win32con', 'pywintypes'],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)
 
exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='ResimHTMLReport',
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
    icon=None,  
)