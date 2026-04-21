Flask + Plotly served from a Singularity / Apptainer image

This repository demonstrates a minimal pattern for packaging a small Python
web app in a Singularity/Apptainer image (`.sif` / `.simg`) while keeping
application code outside the image and bind-mounting it at runtime.

The approach is intentionally minimal and focused on reproducibility,
portability, and fast development iteration on HPC clusters where Docker is
not available. The container provides the runtime (Python and required
libraries); the `project/` folder contains the application code and is
mounted into the running container. This lets you update code locally and
push only the code (not the whole image) to the cluster.

**Why create a `.sif`/`.simg` with only the runtime and keep the app code
mounted from the host?**
- Reproducibility: the image pins a known Python base and runtime packages so
  any node with Apptainer/Singularity can run the same environment.
- Small iterative updates: rebuilding container images is slow and
  transferring large images across networks is inefficient. By keeping the
  app outside the image you can `scp`/`rsync` just the `project/` folder.
- Cluster constraints: many HPC systems do not allow Docker; Singularity/
  Apptainer are supported and the `.sif` file can be stored on a shared
  filesystem for multiple users to run.
- Security & governance: the image contains only runtime dependencies and
  not credentials or large datasets; mounting host paths provides control
  over what gets executed on the cluster node.

Files in this repo and their purpose
- `project/app.py` — minimal Flask app producing two Plotly visualizations.
- `project/templates/index.html` — HTML template used by the app.
- `project/requirements.txt` — reference dependency list for local installs.
- `Singularity.def` — definition file to build the Singularity/Apptainer
  image (uses `python:3.10-slim` and installs `flask`, `plotly`, `pandas`,
  `numpy`).
- `sync_to_cluster.py` — optional helper (Paramiko/SFTP) to push changed
  files from your laptop to the cluster path. Useful when `scp`/`rsync` is
  not available or you want an incremental push.
- `run_container.sh` — small wrapper to run the container image on the
  cluster and bind the `project` folder into `/project` inside the image.

How the image is intended to be used
- Build the image (on a machine or admin node that has `singularity` or
  `apptainer` and privileges to build images). Builds typically require
  root/sudo access on the builder host:

```sh
# Singularity
sudo singularity build flask_plotly.simg Singularity.def

# or Apptainer
sudo apptainer build flask_plotly.sif Singularity.def
```

- Do NOT copy the built `.sif`/`.simg` to every developer machine. Instead,
  place the image on the cluster's shared filesystem (for example,
  `/net/.../all_service/flask_plotly.sif`) and upload only the `project/`
  folder from your laptop using `scp` or `rsync`.

Running the app on the cluster (recommended patterns)

1. Push only code (fast) — example `scp` (run on your laptop):

```bash
scp -r ./project ouymc2@10.214.45.45:"/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/project"
```

2. On the cluster, make sure a container runtime is available. Many sites
   expose `apptainer`/`singularity` as environment modules:

```bash
module load apptainer   # or module load singularity/<version>
which apptainer || which singularity
```

3. Run the image and bind your `project` directory into `/project` inside
   the container so the container executes the host code:

```bash
singularity exec --bind "/net/.../all_service/project:/project" \
  /net/.../all_service/flask_plotly.sif \
  bash -c "cd /project && python app.py"
```

4. If port 5000 is already in use on the node, run on a different port by
   setting `FLASK_PORT` before running:

```bash
export FLASK_PORT=5001
singularity exec --bind "/net/.../all_service/project:/project" \
  /net/.../all_service/flask_plotly.sif \
  bash -c "cd /project && FLASK_PORT=${FLASK_PORT:-5000} python app.py"
```

Notes about the contained runtime
- The `Singularity.def` installs the runtime Python packages (`flask`,
  `plotly`, `pandas`, `numpy`) so the container can run the app without
  network access or requiring pip on the node at runtime. Because the
  image is intentionally minimal it avoids bundling datasets and large
  artifacts. If you need to include additional native libraries, add them
  to the `%post` section of `Singularity.def` and rebuild.

Transferring the image vs. transferring code
- Container images (especially multi-GB scientific images) are expensive
  to transfer. For normal development, copy only `project/` to the
  cluster. If you must transfer the image, prefer `rsync` or an admin
  stage area. `sync_to_cluster.py` purposely focuses on pushing the code
  (not the image) and can be used when `scp` is unavailable.

Security and credentials
- Do NOT embed passwords, keys, or credentials inside the image or the
  repo. The `sync_to_cluster.py` script prompts for your SSH password or
  can use a private key via `--key`. When running in production, prefer
  SSH keys with passphrases and agent forwarding (or a cluster-managed
  credential store) instead of plain passwords.

Troubleshooting
- "Neither apptainer nor singularity found in PATH": load the cluster
  module (`module load apptainer` or `module load singularity/<ver>`).
- "Port 5000 is already in use": either stop your previous process (no
  sudo required if you own the process) or run on a different port using
  `FLASK_PORT` as shown above.
- If `tmux` shows `[server exited]` when trying to start a persistent
  session, collect the tmux server logs with `tmux -vv new -s debug` and
  inspect `tmux-server-*.log` for pty/socket errors. As a fallback use
  `screen` or run `python app.py` inside `nohup`.

How an LLM or new developer should read this repository
- Intent: this repo demonstrates a small, safe pattern for running a
  Python web app on HPC clusters via Singularity/Apptainer while keeping
  iterative development fast by mounting code from the host.
- Key constraints: cluster environments often restrict privileged
  operations (no Docker builds, limited installs). The workflow assumes
  the cluster provides either `singularity` or `apptainer` (via modules),
  a shared filesystem for the `.sif` image and the `project/` code, and
  a mechanism for SSH file transfer into the shared path.
- To reproduce or extend: build the image from `Singularity.def` on a
  capable host, or install dependencies locally and run `project/app.py`
  for quick dev feedback.

Quick reference commands
- Build image (builder host): `sudo apptainer build flask_plotly.sif Singularity.def`
- Upload code: `scp -r ./project ouymc2@10.214.45.45:/path/to/all_service/project`
- Run container (cluster): `singularity exec --bind /path/to/all_service/project:/project /path/to/all_service/flask_plotly.sif bash -c "cd /project && python app.py"`
- Run locally (no container):
  ```bash
  python -m venv venv
  source venv/bin/activate
  pip install -r project/requirements.txt
  cd project
  python app.py
  ```

If you want, I can also:
- add the `FLASK_PORT` support to `run_container.sh`,
- add an `upload_project.sh` convenience script that uses `scp`, or
- move these notes to a `README.dev.md` with step-by-step developer
  workflows. 
