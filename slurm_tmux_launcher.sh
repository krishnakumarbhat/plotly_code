#!/usr/bin/env bash
set -euo pipefail

cd /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service
srun --exclusive --account=RNA-SDV-SRR7 --partition=plcyf-com --nodes=1 --ntasks=1 --cpus-per-task=8 --mem=72G --time=18:00:00 --job-name=ouymc2_udp_kpi_udp_kpi_20260410_064617 --qos=highPrio bash -lc 'set -euo pipefail
if type module >/dev/null 2>&1; then
    module load slurm >/dev/null 2>&1 || true
    module load singularity/3.11.4 >/dev/null 2>&1 || true
fi
SESSION_NAME=ouymc2_udp_kpi_udp_kpi_20260410_064617
UDP_EXIT=/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs/ouymc2/udp_kpi_20260410_064617/compute_udp.exit
INTERACTIVE_EXIT=/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs/ouymc2/udp_kpi_20260410_064617/compute_interactive.exit
INPUT_QUEUE=/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs/ouymc2/udp_kpi_20260410_064617/tmux_input.queue
rm -f "$UDP_EXIT" "$INTERACTIVE_EXIT"
if command -v tmux >/dev/null 2>&1; then
    tmux kill-session -t "$SESSION_NAME" >/dev/null 2>&1 || true
    tmux new-session -d -s "$SESSION_NAME" -n udp
    tmux send-keys -t "$SESSION_NAME:udp" '"'"'bash -lc '"'"'"'"'"'"'"'"'set -uo pipefail; PANE_LOG=/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs/ouymc2/udp_kpi_20260410_064617/compute_udp.log; SHARED_LOG=/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs/ouymc2/udp_kpi_20260410_064617/runtime_console.log; mkdir -p "$(dirname "$PANE_LOG")"; touch "$PANE_LOG"; if [[ -n "$SHARED_LOG" ]]; then mkdir -p "$(dirname "$SHARED_LOG")"; touch "$SHARED_LOG"; fi; if [[ -n "$SHARED_LOG" ]]; then exec > >(sed -u '"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'s/^/[UDP] /'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"' | tee -a "$SHARED_LOG" >> "$PANE_LOG") 2>&1; else exec >> "$PANE_LOG" 2>&1; fi; printf '"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'%s\n'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"' '"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'[UDP] COMMAND: singularity run /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/udp_kpi.simg zmq 5560'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'; printf "%s\n" "[UDP] START: $(date -Iseconds 2>/dev/null || date)"; set +e; singularity run /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/udp_kpi.simg zmq 5560; status=$?; set -e; printf "%s\n" "[UDP] EXIT_STATUS: $status"; printf '"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'%s'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"' "$status" > /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs/ouymc2/udp_kpi_20260410_064617/compute_udp.exit; exit "$status"'"'"'"'"'"'"'"'"''"'"' C-m
    tmux new-window -t "$SESSION_NAME" -n interactive
    tmux send-keys -t "$SESSION_NAME:interactive" '"'"'bash -lc '"'"'"'"'"'"'"'"'set -uo pipefail; PANE_LOG=/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs/ouymc2/udp_kpi_20260410_064617/compute_interactive.log; SHARED_LOG=/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs/ouymc2/udp_kpi_20260410_064617/runtime_console.log; mkdir -p "$(dirname "$PANE_LOG")"; touch "$PANE_LOG"; if [[ -n "$SHARED_LOG" ]]; then mkdir -p "$(dirname "$SHARED_LOG")"; touch "$SHARED_LOG"; fi; if [[ -n "$SHARED_LOG" ]]; then exec > >(sed -u '"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'s/^/[INTERACTIVE] /'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"' | tee -a "$SHARED_LOG" >> "$PANE_LOG") 2>&1; else exec >> "$PANE_LOG" 2>&1; fi; printf '"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'%s\n'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"' '"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'[INTERACTIVE] COMMAND: singularity run --env KPI_SERVER_HOST=127.0.0.1 --env KPI_SERVER_PORT=5560 /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/interactiveplot.simg /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service//net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/ConfigInteractivePlots.xml /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/InputsInteractivePlot1.json /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/out'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'; printf "%s\n" "[INTERACTIVE] START: $(date -Iseconds 2>/dev/null || date)"; for attempt in $(seq 1 90); do (echo > /dev/tcp/127.0.0.1/5560) >/dev/null 2>&1 && break; sleep 2; done; (echo > /dev/tcp/127.0.0.1/5560) >/dev/null 2>&1 || { echo '"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'Timed out waiting for port 5560'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"' >&2; status=1; printf '"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'%s'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"' "$status" > /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs/ouymc2/udp_kpi_20260410_064617/compute_interactive.exit; exit "$status"; }; set +e; singularity run --env KPI_SERVER_HOST=127.0.0.1 --env KPI_SERVER_PORT=5560 /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/interactiveplot.simg /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service//net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/ConfigInteractivePlots.xml /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/InputsInteractivePlot1.json /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/out; status=$?; set -e; printf "%s\n" "[INTERACTIVE] EXIT_STATUS: $status"; printf '"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'%s'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"'"' "$status" > /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs/ouymc2/udp_kpi_20260410_064617/compute_interactive.exit; exit "$status"'"'"'"'"'"'"'"'"''"'"' C-m
    INPUT_QUEUE=/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs/ouymc2/udp_kpi_20260410_064617/tmux_input.queue
touch "$INPUT_QUEUE"
tmux_queue_worker() {
    local seen_lines=0
    while tmux has-session -t "$SESSION_NAME" >/dev/null 2>&1; do
        local total_lines
        total_lines="$(wc -l < "$INPUT_QUEUE" 2>/dev/null | tr -d '"'"'[:space:]'"'"')"
        total_lines="${total_lines:-0}"
        if (( total_lines > seen_lines )); then
            while IFS=$'"'"'	'"'"' read -r pane_name payload; do
                pane_name="${pane_name:-udp}"
                case "$pane_name" in
                    udp | interactive) ;;
                    *) pane_name=udp ;;
                esac
                if [[ "$payload" == '"'"'__CTRL_C__'"'"' ]]; then
                    tmux send-keys -t "$SESSION_NAME:$pane_name" C-c
                elif [[ "$payload" == '"'"'__ENTER__'"'"' ]]; then
                    tmux send-keys -t "$SESSION_NAME:$pane_name" C-m
                elif [[ -n "$payload" ]]; then
                    tmux send-keys -t "$SESSION_NAME:$pane_name" -l -- "$payload"
                    tmux send-keys -t "$SESSION_NAME:$pane_name" C-m
                fi
            done < <(sed -n "$((seen_lines + 1)),$total_lines p" "$INPUT_QUEUE")
            seen_lines=$total_lines
        fi
        sleep 1
    done
}
tmux_queue_worker &
INPUT_WATCHER_PID=$!
    udp_status='"'"''"'"'
    interactive_status='"'"''"'"'
    while true; do
        if [[ -f "$INTERACTIVE_EXIT" ]]; then
            interactive_status="$(cat "$INTERACTIVE_EXIT")"
            break
        fi
        if [[ -f "$UDP_EXIT" ]]; then
            udp_status="$(cat "$UDP_EXIT")"
            break
        fi
        sleep 2
    done
    kill "${INPUT_WATCHER_PID:-}" >/dev/null 2>&1 || true
    wait "${INPUT_WATCHER_PID:-}" >/dev/null 2>&1 || true
    tmux kill-session -t "$SESSION_NAME" >/dev/null 2>&1 || true
    if [[ -n "$udp_status" && "$udp_status" != '"'"'0'"'"' ]]; then
        echo '"'"'UDP KPI server exited before Interactive Plot finished'"'"' >&2
        exit "$udp_status"
    fi
    if [[ -z "$interactive_status" ]]; then
        echo '"'"'Interactive Plot did not finish cleanly'"'"' >&2
        exit 1
    fi
    exit "$interactive_status"
fi
bash -lc '"'"'set -uo pipefail; PANE_LOG=/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs/ouymc2/udp_kpi_20260410_064617/compute_udp.log; SHARED_LOG=/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs/ouymc2/udp_kpi_20260410_064617/runtime_console.log; mkdir -p "$(dirname "$PANE_LOG")"; touch "$PANE_LOG"; if [[ -n "$SHARED_LOG" ]]; then mkdir -p "$(dirname "$SHARED_LOG")"; touch "$SHARED_LOG"; fi; if [[ -n "$SHARED_LOG" ]]; then exec > >(sed -u '"'"'"'"'"'"'"'"'s/^/[UDP] /'"'"'"'"'"'"'"'"' | tee -a "$SHARED_LOG" >> "$PANE_LOG") 2>&1; else exec >> "$PANE_LOG" 2>&1; fi; printf '"'"'"'"'"'"'"'"'%s\n'"'"'"'"'"'"'"'"' '"'"'"'"'"'"'"'"'[UDP] COMMAND: singularity run /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/udp_kpi.simg zmq 5560'"'"'"'"'"'"'"'"'; printf "%s\n" "[UDP] START: $(date -Iseconds 2>/dev/null || date)"; set +e; singularity run /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/udp_kpi.simg zmq 5560; status=$?; set -e; printf "%s\n" "[UDP] EXIT_STATUS: $status"; printf '"'"'"'"'"'"'"'"'%s'"'"'"'"'"'"'"'"' "$status" > /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs/ouymc2/udp_kpi_20260410_064617/compute_udp.exit; exit "$status"'"'"' &
udp_pid=$!
bash -lc '"'"'set -uo pipefail; PANE_LOG=/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs/ouymc2/udp_kpi_20260410_064617/compute_interactive.log; SHARED_LOG=/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs/ouymc2/udp_kpi_20260410_064617/runtime_console.log; mkdir -p "$(dirname "$PANE_LOG")"; touch "$PANE_LOG"; if [[ -n "$SHARED_LOG" ]]; then mkdir -p "$(dirname "$SHARED_LOG")"; touch "$SHARED_LOG"; fi; if [[ -n "$SHARED_LOG" ]]; then exec > >(sed -u '"'"'"'"'"'"'"'"'s/^/[INTERACTIVE] /'"'"'"'"'"'"'"'"' | tee -a "$SHARED_LOG" >> "$PANE_LOG") 2>&1; else exec >> "$PANE_LOG" 2>&1; fi; printf '"'"'"'"'"'"'"'"'%s\n'"'"'"'"'"'"'"'"' '"'"'"'"'"'"'"'"'[INTERACTIVE] COMMAND: singularity run --env KPI_SERVER_HOST=127.0.0.1 --env KPI_SERVER_PORT=5560 /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/interactiveplot.simg /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service//net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/ConfigInteractivePlots.xml /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/InputsInteractivePlot1.json /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/out'"'"'"'"'"'"'"'"'; printf "%s\n" "[INTERACTIVE] START: $(date -Iseconds 2>/dev/null || date)"; for attempt in $(seq 1 90); do (echo > /dev/tcp/127.0.0.1/5560) >/dev/null 2>&1 && break; sleep 2; done; (echo > /dev/tcp/127.0.0.1/5560) >/dev/null 2>&1 || { echo '"'"'"'"'"'"'"'"'Timed out waiting for port 5560'"'"'"'"'"'"'"'"' >&2; status=1; printf '"'"'"'"'"'"'"'"'%s'"'"'"'"'"'"'"'"' "$status" > /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs/ouymc2/udp_kpi_20260410_064617/compute_interactive.exit; exit "$status"; }; set +e; singularity run --env KPI_SERVER_HOST=127.0.0.1 --env KPI_SERVER_PORT=5560 /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/interactiveplot.simg /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service//net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/ConfigInteractivePlots.xml /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/InputsInteractivePlot1.json /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/out; status=$?; set -e; printf "%s\n" "[INTERACTIVE] EXIT_STATUS: $status"; printf '"'"'"'"'"'"'"'"'%s'"'"'"'"'"'"'"'"' "$status" > /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs/ouymc2/udp_kpi_20260410_064617/compute_interactive.exit; exit "$status"'"'"'
interactive_status=$?
kill "$udp_pid" >/dev/null 2>&1 || true
wait "$udp_pid" >/dev/null 2>&1 || true
exit "$interactive_status"
'
