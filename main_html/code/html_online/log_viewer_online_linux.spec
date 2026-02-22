# -*- mode: python ; coding: utf-8 -*-

import os
import glob

block_cipher = None

try:
    base_path = os.path.abspath(os.path.dirname(__file__) or '.')
except NameError:
    base_path = os.path.abspath(os.getcwd())
pathex = [base_path]

datas = []
static_root = os.path.join(base_path, 'html_online', 'static')
if os.path.isdir(static_root):
    for src in glob.glob(os.path.join(static_root, '**'), recursive=True):
        if os.path.isfile(src):
            rel = os.path.relpath(src, static_root)
            dest_dir = os.path.dirname(rel)
            datas.append((src, os.path.join('static', dest_dir)))

db_root = os.path.join(base_path, 'db')
if os.path.isdir(db_root):
    for src in glob.glob(os.path.join(db_root, '**'), recursive=True):
        if os.path.isfile(src):
            rel = os.path.relpath(src, db_root)
            dest_dir = os.path.dirname(rel)
            datas.append((src, os.path.join('db', dest_dir)))

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
    strip=True,
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
