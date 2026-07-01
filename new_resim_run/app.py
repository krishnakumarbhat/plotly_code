"""
Demo Flask App: Runtime Map with Resim Run feature
Minimal working version for local demonstration.
"""
import os
import sys
import uuid
import json
import time
import logging
import threading
import subprocess
from pathlib import Path
from datetime import datetime
from typing import Optional

from flask import (
    Flask, render_template, request, redirect, url_for,
    flash, jsonify, session,
)
from flask_login import (
    LoginManager, login_user, logout_user, login_required, current_user, UserMixin,
)
from flask_sqlalchemy import SQLAlchemy
from sqlalchemy import Column, Integer, String, Text, DateTime

from utils import extract_project_from_path, cluster_from_path

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = Flask(__name__)
app.secret_key = 'demo-resim-run-secret-key-change-in-production'
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///resim_demo.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False

db = SQLAlchemy(app)
login_manager = LoginManager()
login_manager.init_app(app)
login_manager.login_view = 'login'


# ── Models ──────────────────────────────────────────────────────────────────

class User(UserMixin, db.Model):
    __tablename__ = 'users'
    id = Column(Integer, primary_key=True)
    net_id = Column(String(80), unique=True, nullable=False)
    name = Column(String(120), nullable=False)
    password = Column(String(200), nullable=False)  # plain text for demo only

    jobs = db.relationship('JobHistory', backref='user', lazy=True)


class JobHistory(db.Model):
    __tablename__ = 'job_history'
    id = Column(Integer, primary_key=True)
    user_id = Column(Integer, db.ForeignKey('users.id'), nullable=False)
    tool_name = Column(String(80), nullable=False)
    input_path = Column(Text, default='')
    input_filename = Column(String(200), default='')
    output_path = Column(Text, default='')
    parameters = Column(Text, default='{}')  # JSON
    status = Column(String(20), default='QUEUED')
    error_message = Column(Text, default='')
    output_log_path = Column(Text, default='')
    slurm_job_id = Column(String(20), default='')
    started_at = Column(DateTime)
    completed_at = Column(DateTime)
    created_at = Column(DateTime, default=datetime.utcnow)

    def to_dict(self):
        return {
            'id': self.id,
            'tool_name': self.tool_name,
            'input_path': self.input_path,
            'input_filename': self.input_filename,
            'status': self.status,
            'parameters': json.loads(self.parameters or '{}'),
            'created_at': self.created_at.isoformat() if self.created_at else None,
            'started_at': self.started_at.isoformat() if self.started_at else None,
            'completed_at': self.completed_at.isoformat() if self.completed_at else None,
        }


# ── Auth ────────────────────────────────────────────────────────────────────

@login_manager.user_loader
def load_user(user_id):
    return User.query.get(int(user_id))


@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        net_id = request.form.get('net_id', '').strip()
        password = request.form.get('password', '').strip()
        user = User.query.filter_by(net_id=net_id).first()
        if user and user.password == password:
            login_user(user)
            return redirect(url_for('dashboard'))
        flash('Invalid credentials', 'error')
    return render_template('login.html')


@app.route('/logout')
@login_required
def logout():
    logout_user()
    return redirect(url_for('login'))


# ── Routes ──────────────────────────────────────────────────────────────────

@app.route('/')
def index():
    if current_user.is_authenticated:
        return redirect(url_for('dashboard'))
    return redirect(url_for('login'))


@app.route('/html')
@login_required
def dashboard():
    recent_jobs = JobHistory.query.filter_by(
        user_id=current_user.id
    ).order_by(JobHistory.created_at.desc()).limit(10).all()
    return render_template('dashboard.html', recent_jobs=recent_jobs)


@app.route('/html/runtime-map', methods=['GET'])
@login_required
def runtime_map():
    recent_jobs = JobHistory.query.filter_by(
        user_id=current_user.id,
        tool_name='resim_run',
    ).order_by(JobHistory.created_at.desc()).limit(20).all()
    return render_template('runtime_map.html', recent_jobs=recent_jobs)


@app.route('/html/resim_run_submit', methods=['POST'])
@login_required
def resim_run_submit():
    input_txt = request.form.get('input_txt', '').strip()
    simg_path = request.form.get('simg_path', '').strip()

    if not input_txt:
        flash('Input file (input.txt) is required', 'error')
        return redirect(url_for('runtime_map'))
    if not simg_path:
        flash('Simg file path is required', 'error')
        return redirect(url_for('runtime_map'))

    # Detect project and cluster
    project_name, project_root = extract_project_from_path(input_txt)
    cluster = cluster_from_path(input_txt)

    if not project_name:
        flash('Could not detect project from the path. Make sure the path contains PLKRA-PROJECTS/<NAME>/ or projects/<NAME>/', 'error')
        return redirect(url_for('runtime_map'))

    # Build the command that would run
    cmd_parts = [
        'cd', project_root or '.',
        '&&',
        './rResim_Gen7.sh',
        input_txt,
        simg_path,
        'highPrio',
    ]
    cmd_str = ' '.join(cmd_parts)

    # Build srun command (simulated locally)
    srun_cmd = [
        'srun',
        '-N', '1',
        '-n', '1',
        '--partition=highPrio',
        f'--account={project_name}',
        '--mem=16G',
        '--cpus-per-task=4',
        '--time=03:00:00',
        '--job-name=resim_run',
        'bash', '-lc',
        cmd_str,
    ]

    # Create job record
    job = JobHistory(
        user_id=current_user.id,
        tool_name='resim_run',
        input_path=input_txt,
        input_filename=os.path.basename(input_txt),
        output_path=project_root or '',
        parameters=json.dumps({
            'input_txt': input_txt,
            'simg_path': simg_path,
            'resim_command': cmd_str,
            'project': project_name,
            'cluster': cluster or 'unknown',
        }),
        status='QUEUED',
    )
    db.session.add(job)
    db.session.commit()
    job_id = job.id

    # Launch background thread (simulates srun locally)
    t = threading.Thread(
        target=_run_resim_job_background,
        args=(job_id, srun_cmd, cmd_str, project_root or '.'),
        daemon=True,
    )
    t.start()

    flash(f'Resim job #{job_id} submitted! Project: {project_name}, Partition: highPrio. Command: {cmd_str}', 'success')
    return redirect(url_for('runtime_map'))


def _run_resim_job_background(job_id, srun_cmd, display_cmd, cwd):
    """Background runner that simulates the resim job."""
    try:
        with app.app_context():
            job = JobHistory.query.get(job_id)
            if not job:
                return
            job.started_at = datetime.utcnow()
            job.status = 'RUNNING'
            db.session.commit()

        log_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), f'resim_{job_id}.log')
        os.makedirs(os.path.dirname(log_path) or '.', exist_ok=True)

        with open(log_path, 'w') as f:
            f.write(f"CMD: {display_cmd}\n")
            f.write(f"CWD: {cwd}\n")
            f.write(f"SRUN: {' '.join(str(x) for x in srun_cmd)}\n")
            f.write(f"START: {datetime.utcnow().isoformat()}Z\n\n")
            f.write(f"[DEMO] This command would be submitted via srun on the cluster.\n")
            f.write(f"[DEMO] In production, srun would allocate resources and run:\n")
            f.write(f"[DEMO]   {display_cmd}\n\n")

            # Simulate some work
            for i in range(5):
                time.sleep(0.5)
                f.write(f"[{i+1}/5] Simulating resim run...\n")
                f.flush()

            f.write(f"\n[DONE] Resim job completed (simulated).\n")
            f.write(f"END: {datetime.utcnow().isoformat()}Z\n")

        with app.app_context():
            job = JobHistory.query.get(job_id)
            if job:
                job.completed_at = datetime.utcnow()
                job.status = 'COMPLETED'
                job.output_log_path = log_path
                db.session.commit()

    except Exception as exc:
        logger.exception('Resim background job failed')
        try:
            with app.app_context():
                job = JobHistory.query.get(job_id)
                if job:
                    job.completed_at = datetime.utcnow()
                    job.status = 'FAILED'
                    job.error_message = str(exc)
                    db.session.commit()
                else:
                    print("JOB NOT FOUND IN EXCEPTION", flush=True)
                    print(f"app.app_context: {app.app_context()}", flush=True)
        except Exception:
            pass


# ── API endpoints ───────────────────────────────────────────────────────────

@app.route('/api/browse')
@login_required
def api_browse():
    path = request.args.get('path', '/net').strip()
    if not path:
        return jsonify({'error': 'Path is required'}), 400

    # Security: restrict browsing to valid paths
    allowed_prefixes = ['/net', '/mnt', '/scratch']
    if not any(path.startswith(p) for p in allowed_prefixes):
        # For local demo, also allow common local paths
        if not os.path.isabs(path):
            return jsonify({'error': 'Path must be absolute'}), 400

    try:
        entries = sorted(os.listdir(path))
        dirs = []
        files = []
        for entry in entries:
            full = os.path.join(path, entry)
            try:
                if os.path.isdir(full):
                    dirs.append(full)
                else:
                    files.append(full)
            except PermissionError:
                continue

        parent = os.path.dirname(path) if path not in ['/', '/net', '/mnt'] else None

        return jsonify({
            'current': path,
            'parent': parent,
            'dirs': dirs,
            'files': files,
        })
    except PermissionError:
        return jsonify({'error': f'Permission denied: {path}'}), 403
    except FileNotFoundError:
        return jsonify({'error': f'Path not found: {path}'}), 404
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/detect_project')
@login_required
def api_detect_project():
    path = request.args.get('path', '').strip()
    if not path:
        return jsonify({'project': None, 'cluster': None})

    cluster = cluster_from_path(path)
    project_name, project_root = extract_project_from_path(path)

    return jsonify({
        'project': project_name,
        'project_root': project_root,
        'cluster': cluster,
    })


# ── Init ────────────────────────────────────────────────────────────────────

def init_demo_db():
    with app.app_context():
        db.create_all()
        # Create demo users if none exist
        if not User.query.first():
            demo_users = [
                User(net_id='pcmzxl', name='Demo User', password='demo123'),
                User(net_id='admin', name='Admin', password='admin123'),
            ]
            db.session.add_all(demo_users)
            db.session.commit()
            print('[*] Created demo users: pcmzxl/demo123, admin/admin123')


if __name__ == '__main__':
    init_demo_db()
    print('[*] Resim Run Demo starting on http://127.0.0.1:5050')
    print('[*] Login with: pcmzxl / demo123')
    print('[*] Navigate to Runtime Map tab to see the Resim Run form.')
    app.run(host='127.0.0.1', port=5050, debug=True)
