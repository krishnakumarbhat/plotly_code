# HPC Tools Platform

A comprehensive Flask-based web application for managing HPC (High-Performance Computing) tools including DC HTML Report, Interactive Plot, KPI Analysis, and Hyperlink Tool with integrated AI chat assistance.

## 🏗️ Architecture

This platform follows a **Manager-Worker** model designed for HPC clusters:

```
┌─────────────────────────────────────────────────────────────────────┐
│                         LOGIN NODE (Manager)                         │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐                 │
│  │   Flask     │  │  PostgreSQL │  │  ChromaDB   │                 │
│  │   Web App   │  │  Database   │  │  (Vector)   │                 │
│  └─────────────┘  └─────────────┘  └─────────────┘                 │
│         │                                                            │
│         │  sbatch                                                    │
│         ▼                                                            │
├─────────────────────────────────────────────────────────────────────┤
│                        SLURM JOB SCHEDULER                          │
├─────────────────────────────────────────────────────────────────────┤
│                       COMPUTE NODES (Workers)                        │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌────────────┐ │
│  │  DC HTML    │  │ Interactive │  │    KPI      │  │  Hyperlink │ │
│  │  Container  │  │    Plot     │  │  Analysis   │  │    Tool    │ │
│  └─────────────┘  └─────────────┘  └─────────────┘  └────────────┘ │
├─────────────────────────────────────────────────────────────────────┤
│                        GPU NODE                                      │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    LLM Server (Qwen)                         │   │
│  └─────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
```

## 🚀 Features

- **User Authentication**: Login/Registration with Net ID
- **AI Chat Assistant**: LLM-powered chat for tool assistance
- **Tool Integration**:
  - DC HTML Report Generation
  - Interactive Plot Creation
  - KPI Analysis
  - Hyperlink Tool
- **Job Management**: Slurm job submission and monitoring
- **File Browser**: Secure cluster file system navigation
- **Job History**: Complete history with status tracking

## 📁 Project Structure

```
all_services/
├── app.py                 # Main Flask application
├── config.py              # Configuration settings
├── models.py              # SQLAlchemy database models
├── utils.py               # Utility functions (Slurm, FileBrowser)
├── requirements.txt       # Python dependencies
├── docker-compose.yml     # PostgreSQL container
├── .env.template          # Environment variables template
│
├── templates/             # Jinja2 HTML templates
│   ├── base.html          # Base template with navbar
│   ├── login.html         # Login page
│   ├── register.html      # Registration page
│   ├── dashboard.html     # Main dashboard with chat
│   ├── history.html       # Job history page
│   ├── tools/             # Tool-specific templates
│   │   ├── dc_html.html
│   │   ├── interactive_plot.html
│   │   ├── kpi.html
│   │   └── hyperlink_tool.html
│   └── errors/            # Error pages
│       ├── 404.html
│       └── 500.html
│
├── static/                # Static assets
│   ├── css/
│   │   └── style.css
│   └── js/
│       ├── main.js
│       └── file_browser.js
│
├── scripts/               # Build/ops helpers (WSL build, sync, maintenance)
│   ├── build_and_run_singularity.sh
│   ├── build_and_sync.sh
│   ├── cleanup_memory.sh
│   ├── start_llm_server.sh
│   ├── wsl_build_all_images.sh
│   ├── wsl_build_kpi_interactive_and_run_main.sh
│   └── wsl_build_simg.sh
│
├── simg/                  # Cluster-deployed images + run entrypoints
│   ├── run_all_services.sh
│   ├── run_dc_html.sh
│   ├── run_interactive_plot.sh
│   └── run_kpi.sh
│
└── tools/                 # Existing tool implementations
    ├── dc_html/
    ├── InteractivePlot/
    ├── KPI/
    └── Hyperlink_tool/
```

## 🛠️ Installation

### Prerequisites

- Python 3.10+
- PostgreSQL 15+
- Singularity/Apptainer (for HPC)
- Slurm workload manager

### 1. Clone and Setup

```bash
# Navigate to project directory
cd /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/2-Sim/USER_DATA/ouymc2/all_services

# Create virtual environment
python -m venv venv
source venv/bin/activate  # Linux
# or
.\venv\Scripts\activate   # Windows

# Install dependencies
pip install -r requirements.txt
```

### 2. Configure Environment

```bash
# Copy environment template
cp .env.template .env

# Edit .env with your settings
nano .env
```

Key settings to configure:
- `DATABASE_URL`: PostgreSQL connection string
- `SECRET_KEY`: Secure random key for sessions
- `DATA_BASE_PATH`: Base path for cluster data
- `LLM_SERVICE_URL`: URL of LLM service

### 3. Start PostgreSQL Database

Note for Windows local dev:
- If you do **not** set `DATABASE_URL`, the app defaults to a local SQLite DB file (`hpc_tools_dev.db`) so it can start without PostgreSQL.
- If you want to use PostgreSQL on Windows, start it via Docker Compose (below) and set `DATABASE_URL` accordingly.

```bash
# Using Docker Compose
docker-compose up -d postgres

# Verify database is running
docker-compose ps
```

### 4. Initialize Database

```bash
# Initialize Flask database
flask db init
flask db migrate -m "Initial migration"
flask db upgrade

# Or use the CLI command
flask init-db
```

### 5. Create Admin User

```bash
flask create-admin
```

### 6. Start the Application

```bash
# Development mode
flask run --host=0.0.0.0 --port=5000

# Production mode with Gunicorn
gunicorn -w 4 -b 0.0.0.0:5000 app:app
```

## 🌐 URL Routes

| Route | Description |
|-------|-------------|
| `/` | Redirect to dashboard or login |
| `/login` | User login page |
| `/register` | New user registration |
| `/logout` | User logout |
| `/html` | Main dashboard with chat |
| `/html/dc_html` | DC HTML Report tool |
| `/html/interactive_plot` | Interactive Plot tool |
| `/html/kpi` | KPI Analysis tool |
| `/html/hyperlink_tool` | Hyperlink tool |
| `/html/history` | Job history |

## 🔧 API Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/chat` | POST | Send chat message to LLM |
| `/api/job/<id>/status` | GET | Get job status |
| `/api/job/<id>/cancel` | POST | Cancel a running job |
| `/api/jobs` | GET | List all user jobs |
| `/api/browse` | GET | Browse cluster files |

## 🖥️ HPC Deployment

### Run the main UI via Singularity/Apptainer (recommended for clusters)

This avoids relying on the cluster's system Python (often older, e.g. 3.6).

**Build the `.sif` on your PC (WSL) and copy to cluster**

In WSL (Ubuntu, etc.):

```bash
cd /mnt/c/git/Core_RESIM_KPI/all_services

# Build image locally (defaults to all_services.sif)
./scripts/wsl_build_sif.sh all_services.sif \
    ouymc2@10.214.45.45:/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/2-Sim/USER_DATA/ouymc2/all_services/
```

**Run on cluster (port 5001)**

```bash
cd /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/2-Sim/USER_DATA/ouymc2/all_services

# Uses PORT=5001 by default
singularity run --bind /net:/net --bind /scratch:/scratch ./all_services.sif

# Optional overrides
# PORT=5001 WORKERS=2 HOST=0.0.0.0 singularity run --bind /net:/net --bind /scratch:/scratch ./all_services.sif
```

If you cannot access the login node port directly from your laptop (common firewall policy), use SSH port-forwarding:

```bash
ssh -L 5001:127.0.0.1:5001 ouymc2@10.214.45.45
```

Then open:

- `http://localhost:5001/html`

### Building Singularity Images

The platform includes separate Singularity images for each tool with specific resource requirements:

```bash
# Navigate to simg directory
cd /path/to/all_services/simg

# Build all images at once
./build_all_images.sh

# Or build individually:

# 1. Main web application (Flask + Hyperlink viewer)
cd /path/to/all_services
apptainer build --fakeroot simg/all_services.simg Singularity.def

# 2. Interactive Plot tool (64GB RAM, 8 CPUs)
cd /path/to/all_services/tools/InteractivePlot
apptainer build --fakeroot ../../simg/interactive_plot.simg Singularity_InteractivePlot.def

# 3. KPI tool (32GB RAM, 8 CPUs)
cd /path/to/all_services/tools/KPI
apptainer build --fakeroot ../../simg/kpi.simg Singularity_KPI.def
```

## 🔧 Tool Usage Guide

### Interactive Plot Tool

Generates interactive HTML reports from HDF sensor data files.

**Resource Requirements:**
- Memory: 64 GB
- CPUs: 8 cores
- Partition: plcyf-com
- Module: singularity/3.11.4

**Usage via Web Interface:**
1. Go to `/html/interactive_plot`
2. Select input mode:
   - **JSON Mode**: Provide `config.xml` and `inputs.json`
   - **HDF Pair Mode**: Provide input/output HDF files
3. Optionally specify output directory (defaults to `html_db/` in simg folder)
4. Click "Submit to Slurm"

**Usage via Command Line:**
```bash
# JSON Mode
./simg/run_interactive_plot.sh config.xml inputs.json /output/html_db

# HDF Pair Mode
./simg/run_interactive_plot.sh --hdf input.h5 output.h5 /output/html_db

# Direct singularity execution
srun --partition=plcyf-com --mem=64G --cpus-per-task=8 \
    singularity run simg/interactive_plot.simg config.xml inputs.json /output
```

**JSON Input File Format (inputs.json):**
```json
[
  {"input_path": "/path/to/input1.h5", "output_path": "/path/to/output1.h5"},
  {"input_path": "/path/to/input2.h5", "output_path": "/path/to/output2.h5"}
]
```

### KPI Tool

Generates KPI HTML reports with support for JSON batch and HDF pair modes.

**Resource Requirements:**
- Memory: 32 GB
- CPUs: 8 cores
- Partition: plcyf-com
- Module: singularity/3.11.4

**Usage via Web Interface:**
1. Go to `/html/kpi`
2. Select input mode:
   - **JSON Batch Mode**: Provide `KPIPlot.json` with HDF pairs
   - **HDF Pair Mode**: Provide single input/output HDF files
3. Optionally specify HTML output directory
4. Click "Submit to Slurm"

**Usage via Command Line:**
```bash
# JSON Batch Mode
./simg/run_kpi.sh json KPIPlot.json /output/html_db

# HDF Pair Mode
./simg/run_kpi.sh hdf input.h5 output.h5 /output/html_db

# Direct singularity execution
srun --partition=plcyf-com --mem=32G --cpus-per-task=8 \
    singularity run simg/kpi.simg json KPIPlot.json /output/html_db
```

**JSON Input File Format (KPIPlot.json):**
```json
[
  {"input_path": "/path/to/input1.h5", "output_path": "/path/to/output1.h5"},
  {"input_path": "/path/to/input2.h5", "output_path": "/path/to/output2.h5"}
]
```

### Hyperlink Tool (LogView)

Serves HTML & video files and provides mapping API for the viewer.

**Usage:**
The Hyperlink Tool is integrated into the main web application and accessible at `/hyperlink/`.

## 🖥️ Krakow Cluster Configuration

The platform is optimized for the Krakow HPC cluster with the following configuration:

| Setting | Value |
|---------|-------|
| OS | Rocky Linux 8.7 |
| Scheduler | Slurm |
| Default Partition | plcyf-com |
| Singularity Module | singularity/3.11.4 |
| GPU Node | plcyf-com-prod-gpu001 |

**Available Partitions:**
```
PARTITION    TIMELIMIT  NODES
highPrio     3:00:00    19
interactive  1-00:00:00 1
plcyf-com    7-00:00:00 19
plcyf-gpu    7-00:00:00 1
plcyf-upload 7-00:00:00 8
```

### Running on Login Node (plcyf-com-prod-log01)

The login node has limited resources (4 CPUs, 15 GB RAM) and no Python support. To run the web application:

1. **Use pre-built Singularity images** - No Python installation required
2. **Submit heavy processing to compute nodes** - Tools automatically submit via Slurm

```bash
# Start web app on login node
cd /path/to/all_services
module load singularity/3.11.4
PORT=5001 singularity run --bind /net:/net --bind /scratch:/scratch simg/all_services.simg

# Tool jobs are automatically submitted to compute nodes via sbatch
```

### Output Directory Structure

When no output directory is specified, outputs are stored in:
```
simg/
├── html_db/                    # Default HTML output directory
│   ├── interactive_plot/       # Interactive Plot outputs
│   ├── kpi/                    # KPI outputs
│   └── dc_html/               # DC HTML outputs
├── .cache_html/               # Cache for Hyperlink viewer
│   ├── html/
│   └── video/
└── all_services.simg          # Main web application image
```

## 📊 Job Monitoring

Monitor submitted jobs using standard Slurm commands:

```bash
# View your jobs
squeue -u $USER

# View specific job
squeue -j <job_id>

# View job details
sacct -j <job_id> --format=JobID,State,ExitCode,Elapsed,MaxRSS

# Cancel a job
scancel <job_id>

# View job output logs
tail -f /scratch/logs/interactive_plot_<job_id>.out
```
singularity build interactive_plot.sif docker://your-registry/interactive_plot:latest

# Build KPI image
singularity build kpi.sif docker://your-registry/kpi:latest
```

### Starting LLM Server

```bash
# Submit LLM server job to GPU node
sbatch scripts/start_llm_server.sh

# Check LLM server status
squeue -u $USER -n llm_server
```

### Running Flask on Login Node

```bash
# Using systemd service (recommended)
sudo systemctl start hpc-tools

# Or using nohup
nohup gunicorn -w 4 -b 0.0.0.0:5000 app:app > flask.log 2>&1 &
```

## 🔐 Security Considerations

1. **Change Default Passwords**: Update all default passwords in `.env`
2. **Use HTTPS**: Configure nginx/Apache as reverse proxy with SSL
3. **File Access**: The file browser restricts access to configured paths
4. **Session Security**: Sessions expire after 8 hours by default

## 🔄 Adding New Tools

1. Create tool template in `templates/tools/new_tool.html`
2. Add route in `app.py`:
   ```python
   @app.route('/html/new_tool', methods=['GET', 'POST'])
   @login_required
   def tool_new_tool():
       if request.method == 'POST':
           return submit_tool_job('new_tool')
       return render_template('tools/new_tool.html', ...)
   ```
3. Add tool config in `config.py`:
   ```python
   TOOLS = {
       'new_tool': {
           'name': 'New Tool',
           'description': 'Description',
           'singularity_image': 'new_tool.sif',
           'script': 'run_new_tool.sh'
       }
   }
   ```
4. Update `generate_slurm_script()` in `main_html/utils.py` if your new tool needs special arguments

    Note: the current web UI generates Slurm scripts dynamically and stores them under `/tmp/slurm_scripts`.

## 📊 Monitoring

### Check Job Status

```bash
# View all running jobs
squeue -u $USER

# View job details
scontrol show job <job_id>

# View job logs
tail -f /scratch/logs/slurm_<job_id>.out
```

### Database Queries

```sql
-- Recent jobs
SELECT * FROM job_history ORDER BY created_at DESC LIMIT 10;

-- Jobs by status
SELECT status, COUNT(*) FROM job_history GROUP BY status;

-- User activity
SELECT u.net_id, COUNT(j.id) as job_count 
FROM users u JOIN job_history j ON u.id = j.user_id 
GROUP BY u.net_id;
```

## 🐛 Troubleshooting

### Common Issues

1. **Database Connection Failed**
   ```bash
   # Check PostgreSQL container
   docker-compose logs postgres
   
   # Test connection
   psql -h localhost -U hpc_user -d hpc_tools_db
   ```

2. **Slurm Job Submission Failed**
   ```bash
   # Check Slurm status
   sinfo
   
   # Check job submission manually
    sbatch --test-only --wrap="hostname"
   ```

3. **LLM Service Unavailable**
   ```bash
   # Check if LLM job is running
   squeue -n llm_server
   
   # Check LLM logs
   tail -f /scratch/logs/llm_server_*.out
   ```

## 📝 License

Internal use only. All rights reserved.

## 👥 Contributors

- HPC Tools Development Team


# SIMG Folder - Singularity Container Deployment

This folder contains the Singularity container image and run script that will be synced to both HPC clusters.

## Contents

| File | Description |
|------|-------------|
| `all_services.simg` | Main UI container image (built from `../Singularity.def`) |
| `interactive_plot.simg` | Interactive Plot tool image |
| `kpi.simg` | KPI tool image |
| `dc_html.simg` | DC HTML tool image |
| `run_all_services.sh` | Run the main UI container on cluster nodes |
| `run_interactive_plot.sh` | Submit/run Interactive Plot (Slurm/local modes) |
| `run_kpi.sh` | Submit/run KPI (Slurm/local modes) |
| `run_dc_html.sh` | Submit/run DC HTML (Slurm/local modes) |

## Supported Clusters

| Cluster | Host | Remote Path |
|---------|------|-------------|
| Krakow | 10.214.45.45 | `/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/2-Sim/USER_DATA/{user}/all_services/simg` |
| Southfield | 10.192.224.131 | `/mnt/usmidet/projects/RADARCORE/2-Sim/USER_DATA/ouymc/all_services/simg` |

## Usage

### Build and Sync (from Windows)

Use the `build_and_sync.py` script from the project root:

```powershell
# Build in WSL and sync to both clusters
python build_and_sync.py --build --sync

# Only sync existing image
python build_and_sync.py --sync

# Sync to specific cluster
python build_and_sync.py --sync --krakow-only
python build_and_sync.py --sync --southfield-only
```

### Run on Cluster

After syncing, SSH to the cluster and run:

```bash
# On Krakow
ssh ouymc2@10.214.45.45
cd /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/2-Sim/USER_DATA/$USER/all_services/simg
./run_all_services.sh 5001

# On Southfield
ssh ouymc2@10.192.224.131
cd /mnt/usmidet/projects/RADARCORE/2-Sim/USER_DATA/ouymc/all_services/simg
./run_all_services.sh 5001
```

### Run Script Options

```bash
./run_all_services.sh [PORT]     # Default port: 5001
./run_all_services.sh --debug    # Run with debug mode
./run_all_services.sh --shell    # Open interactive shell
./run_all_services.sh --help     # Show help
```

## Notes

- The `.simg` file is a large binary file (~GB), so sync may take time
- The script auto-detects the cluster and sets appropriate bind paths
- LLM URL is automatically configured based on cluster

# SIMG Folder - Singularity Container Deployment

This folder contains the Singularity container image and run script that will be synced to both HPC clusters.
The `.cache_html` subfolder stores runtime data (database, downloaded files) and is NOT synced.

## Contents

| File/Folder | Description | Synced |
|-------------|-------------|--------|
| `all_services.simg` | Singularity container image (~2.4GB) | No (use --include-image) |
| `run_all_services.sh` | Script to run the container on cluster nodes | Yes |
| `.cache_html/` | Runtime cache (database, html/video files) | No |
| `README.md` | This file | Yes |

## Folder Structure

```
simg/
├── all_services.simg          # Container image (large, not synced by default)
├── run_all_services.sh        # Run script
├── README.md
└── .cache_html/               # Runtime data (not synced)
    ├── hpc_tools_dev.db       # SQLite database
    ├── html/                  # Downloaded HTML reports
    └── video/                 # Downloaded videos
```

## Supported Clusters

| Cluster | Host | Remote Path |
|---------|------|-------------|
| Krakow | 10.214.45.45 | `/net/8k3/e0fs01/irods/.../USER_DATA/{user}/all_services/simg` |
| Southfield | 10.192.224.131 | `/mnt/usmidet/projects/RADARCORE/.../all_services/simg` |

## Usage

### Build and Sync (from Windows)

Use the `build_and_sync.py` script from the project root:

```powershell
# Build in WSL (creates simg/all_services.simg)
python build_and_sync.py --build

# Sync run script only (fast, recommended)
python build_and_sync.py --sync

# Sync including large .simg file (slow, ~2.4GB)
python build_and_sync.py --sync --include-image

# Parallel upload for faster .simg transfer
python build_and_sync.py --sync --include-image --parallel-upload

# Sync to specific cluster
python build_and_sync.py --sync --krakow-only
python build_and_sync.py --sync --southfield-only
```

### Run on Cluster

After syncing, SSH to the cluster and run:

```bash
# On Krakow
ssh ouymc2@10.214.45.45
cd /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/2-Sim/USER_DATA/$USER/all_services/simg
./run_all_services.sh 5001

# On Southfield
ssh ouymc2@10.192.224.131
cd /mnt/usmidet/projects/RADARCORE/2-Sim/USER_DATA/ouymc/all_services/simg
./run_all_services.sh 5001
```

### Run Script Options

```bash
./run_all_services.sh [PORT]     # Default port: 5001
./run_all_services.sh --debug    # Run with debug mode
./run_all_services.sh --shell    # Open interactive shell
./run_all_services.sh --help     # Show help
```

### Access Points

After starting the server:
- Main Dashboard: `http://<host>:<port>/html`
- Hyperlink Viewer: `http://<host>:<port>/hyperlink/`
- API: `http://<host>:<port>/api/...`

## Notes

- The `.simg` file is large (~2.4GB), so default sync skips it
- Use `--include-image` only when you've rebuilt the container
- The script auto-detects the cluster and sets appropriate bind paths
- LLM URL and database path are automatically configured based on cluster
- `.cache_html` folder is created automatically on first run and contains:
  - `hpc_tools_dev.db` - SQLite database for user data
  - `html/` and `video/` - Downloaded content from cluster file browser
