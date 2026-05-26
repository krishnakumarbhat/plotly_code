---
name: hpcc-runtime-layout
description: "Use when working on HPCC broker jobs, Slurm launcher paths, runtime log storage, SSH-run-as-user, Southfield/Krakow deployment, or broker/UI permission issues. Trigger phrases: hpcc broker, runtime_console.log, launcher.log, slurm_tmux_launcher, ssh_run_as_user, Southfield, Krakow, output_dir, runtime work root, HPCC_RUNTIME_WORK_ROOT."
---

# HPCC Runtime Layout

This repo runs KPI and Interactive Plot workloads through a broker in `hpcc_main.py`.

## Purpose

Use this skill when an agent needs to:
- debug broker submission failures
- change where runtime logs or control files are stored
- update Slurm launcher behavior
- troubleshoot cross-user permission problems
- deploy broker changes to Southfield or Krakow
- understand how UI console logs map to cluster files

## Runtime Model

There are three different path roles. Keep them separate.

1. User output path
The submitted `paths.output_dir` is where tool results belong. Do not hardcode this to a bundle project.

2. Broker control/work root
The broker needs a project-neutral shared location for launcher scripts, askpass scripts, exit files, tmux queues, and the mirror console log that the UI reads.

Default:
- Linux cluster: `/tmp/hpcc_runtime/<netid>/<job>/`
- Windows/local fallback: `<bundle_root>/runs/<netid>/<job>/`

Override:
- `HPCC_RUNTIME_WORK_ROOT`

3. User-visible runtime log dir
For Slurm KPI runs with an explicit output dir, pane logs are written under:
- `<output_dir>/.hpcc_runtime/<job>/`

This keeps logs with the user’s project/output tree without forcing the broker control path into that project.

## Why The Split Exists

Do not put all broker artifacts inside the bundle project.

Problem:
- Some users can submit jobs for projects that the service account `ouymc2` cannot access.
- Some users are not members of the bundle project group, so they cannot traverse a bundle-root `runs/...` tree if it is inside a restricted project.
- The web UI must still read a live console log and write tmux input commands.

Solution:
- Keep broker control files in a neutral shared runtime root.
- Keep user pane logs under the requested output tree.
- Mirror the shared console log into the control dir so the UI can always stream it.

## Important Files

### `hpcc_main.py`

Main responsibilities:
- receives broker submit requests
- chooses control dir and output path
- writes `slurm_tmux_launcher.sh`
- writes SSH askpass script when `user_password` is present
- launches `ssh <user>@127.0.0.1 srun ...`

Key path decisions live in:
- `_runtime_root(...)`
- `_runtime_work_root(...)`
- `_output_runtime_log_dir(...)`
- `RuntimeBroker._build_spec(...)`
- `RuntimeBroker._write_slurm_launch_script(...)`
- `RuntimeBroker._window_payload(...)`

### `main_html/app.py`

Main responsibilities:
- builds submit payloads for the broker
- includes `user_password`
- exposes the reuse-check endpoint
- streams the runtime console log in the UI

### `main_html/runtime_store.py`

Web-side SQLite store used by the UI.

### `hpcc_runtime_store.py`

Broker-side SQLite store used by `hpcc_main.py`.

## Permission Model

Current permission rules are intentional.

- Broker sets `os.umask(0o022)` on Linux.
- Per-job broker control dirs are chmod `0o777` so submitted jobs can write control artifacts when needed.
- Broker-created shared console logs are chmod `0o666` when `ssh_run_as_user` is active.
- The broker does not pre-validate `output_dir` with a broker-side write test.
- The job itself runs `mkdir -p <output_dir>` as the submitting user.

## Southfield And Krakow

Southfield:
- host: `10.192.224.131`
- bundle root: `/mnt/usmidet/projects/RADARCORE/2-Sim/all_service`
- default Slurm account/partition: `radarcore` / `defq`

Krakow all_service2:
- host: `10.214.45.45`
- bundle root: `/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2`
- default Slurm account/partition: `RNA-SDV-SRR7` / `plcyf-com`
- Python 3.6 constraints still matter on that host

## Deployment Scripts In This Repo

Useful helpers already present:
- `tmp/deploy_southfield_umask_fix.py`
- `tmp/deploy_krakow_as2.py`
- `tmp/deploy_both_clusters.py`
- `tmp/start_krakow_as2_broker.py`

Before deploying:
- rebuild `simg_sh_hpcc/hpcc_main.pyz` via `tmp/refresh_hpcc_bundle_support.sh`
- verify the changed snippets are inside the rebuilt pyz

After deploying:
- verify broker port `9100`
- verify UI port `5002` where applicable
- submit a fresh broker job and inspect both control-dir and output-tree runtime logs

## Debug Checklist

If a user sees `Permission denied`:
- determine whether the failure is in the user output path, the broker control dir, or the mirrored console log
- inspect the exact file path from the error
- check `namei -l <path>` to find the first non-traversable directory
- confirm whether the job is being launched with `ssh_run_as_user`
- confirm file modes for `runtime_console.log`, `launcher.log`, pane logs, and the control dir

If a user sees `Unable to validate whether this run already exists`:
- inspect `main_html/app.py` reuse-check endpoint
- inspect `main_html/runtime_store.py` for SQLite or NFS issues

## Safe Defaults For Future Changes

- Never assume the bundle project is the same project as the user output path.
- Never move all runtime artifacts into a user project path unless the web UI can also read them.
- Prefer a mirrored-log design over a single log path when broker and job run under different identities.