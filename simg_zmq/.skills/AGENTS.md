# HPCC Runtime Notes

Use the skill at `.github/skills/hpcc-runtime-layout/SKILL.md` when working on broker-backed KPI, Interactive Plot, or deployment/debugging tasks for this repo.

Key runtime rules:
- Do not hardcode broker work paths to a project tree like `RADARCORE`. The broker control/work root is shared runtime state and should stay project-neutral.
- User outputs stay in the user-provided `output_dir`.
- Broker control files and UI-readable mirror logs must stay in a location that both the service account and the submitted user can access.
- On Linux clusters, the shared runtime work root defaults to `/tmp/hpcc_runtime` and can be overridden with `HPCC_RUNTIME_WORK_ROOT`.
- For Slurm KPI runs, per-run pane logs are placed under the user output tree at `<output_dir>/.hpcc_runtime/<job>/` while the UI reads the mirrored shared console log from the broker control dir.

Primary files:
- `hpcc_main.py`: broker, Slurm launcher generation, runtime path layout, SSH-run-as-user behavior.
- `hpcc_runtime_store.py`: broker-side runtime DB and job tracking.
- `main_html/app.py`: submission payloads, reuse-check API, runtime console UI.
- `main_html/runtime_store.py`: web-side runtime DB and reusable job lookup.

Known cluster targets:
- Southfield: `10.192.224.131`, bundle root `/mnt/usmidet/projects/RADARCORE/2-Sim/all_service`
- Krakow all_service2: `10.214.45.45`, bundle root `/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2`