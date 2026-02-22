# -*- mode: python ; coding: utf-8 -*-

import os
import glob

block_cipher = None

# Use the directory containing this spec as the base
try:
    base_path = os.path.abspath(os.path.dirname(__file__) or '.')
except NameError:
    base_path = os.path.abspath(os.getcwd())
pathex = [base_path]

# NOTE: static folder is NOT bundled - it should be placed beside the .exe file
# NOTE: db folder is NOT bundled - the app uses .cache_html for user data
datas = []

a = Analysis(
    ['main.py'],
    pathex=pathex,
    binaries=[],
    datas=datas,
    hiddenimports=[
        'html_build',
        'cluster_connect',
        'flask',
        'flask_cors',
        'werkzeug',
        'werkzeug.serving',
        'werkzeug.middleware',
        'werkzeug.middleware.proxy_fix',
        'jinja2',
        'markupsafe',
        'itsdangerous',
        'click',
        'paramiko',
        'cryptography',
        'bcrypt',
        'nacl',
        'cffi',
        'pycparser',
    ],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=['sqlalchemy'],
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
    name='log_viewer_online',
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
# -*- mode: python ; coding: utf-8 -*-

block_cipher = None

a = Analysis(
    ['main.py'],
    pathex=[],
    binaries=[],
    datas=[],
    hiddenimports=[
        'html_build',
        'cluster_connect',
        'flask',
        'flask_cors',
        'werkzeug',
        'werkzeug.serving',
        'werkzeug.middleware',
        'werkzeug.middleware.proxy_fix',
        'jinja2',
        'markupsafe',
        'itsdangerous',
        'click',
        'paramiko',
        'cryptography',
        'bcrypt',
        'nacl',
        'cffi',
        'pycparser',
    ],
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
    name='log_viewer',
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
