import argparse
import gc
import os
import platform
import time

import GPUtil
import numpy as np
import pandas as pd
import plotly.graph_objects as go
import plotly.io as pio
import psutil


OUTPUT_DIR = "html_outputs"
CSV_NAME = "scattergl_system_metrics.csv"
PLOTS_PER_HTML = 10
START_POINTS = 100_000
SCALE_FACTOR = 3
MAX_EQUIVALENT_POINTS = 50_000_000
SAFE_SAMPLE_CAP = 1_000_000
RANDOM_SEED = 42


def parse_args():
    """Parse command-line options for the stress test."""
    parser = argparse.ArgumentParser(
        description="Generate Plotly ScatterGL stress-test HTML files and log system metrics."
    )
    parser.add_argument(
        "--temporal-differences",
        type=int,
        required=True,
        help="Number of HTML files to generate. One file is created per temporal level.",
    )
    return parser.parse_args()


def get_system_info():
    """Collect and print the core hardware and operating system details."""
    cpu_model = (
        platform.processor()
        or platform.uname().processor
        or os.environ.get("PROCESSOR_IDENTIFIER")
        or "Unknown CPU"
    )
    physical_cores = psutil.cpu_count(logical=False) or 0
    logical_threads = psutil.cpu_count(logical=True) or os.cpu_count() or 0
    total_ram_gb = psutil.virtual_memory().total / (1024 ** 3)
    os_name_version = f"{platform.system()} {platform.release()} ({platform.version()})"

    system_info = {
        "CPU model": cpu_model,
        "CPU core count": physical_cores,
        "CPU thread count": logical_threads,
        "Total RAM (GB)": round(total_ram_gb, 2),
        "OS name and version": os_name_version,
    }

    print("System information")
    for label, value in system_info.items():
        print(f"  {label}: {value}")
    print()

    return system_info


def get_gpu_metrics():
    """Read average GPU load and total GPU memory usage across detected GPUs."""
    try:
        gpus = GPUtil.getGPUs()
    except Exception:
        return np.nan, np.nan

    if not gpus:
        return np.nan, np.nan

    gpu_usage_percent = float(np.mean([gpu.load * 100 for gpu in gpus]))
    gpu_memory_used_mb = float(np.sum([gpu.memoryUsed for gpu in gpus]))
    return round(gpu_usage_percent, 2), round(gpu_memory_used_mb, 2)


def calculate_equivalent_points(level):
    """Scale the theoretical point count by a factor of three with a hard upper cap."""
    scaled_points = START_POINTS * (SCALE_FACTOR ** (level - 1))
    return min(scaled_points, MAX_EQUIVALENT_POINTS)


def calculate_sampled_points(equivalent_points):
    """Limit actual point creation to a safe sampled size while preserving density growth."""
    return min(equivalent_points, SAFE_SAMPLE_CAP)


def build_trace_data(rng, trace_index, equivalent_points, sampled_points):
    """Generate a dense but memory-safe cloud for a single ScatterGL trace."""
    compression_ratio = max(equivalent_points / sampled_points, 1.0)
    cluster_scale = max(1.0 / np.sqrt(compression_ratio), 0.02)
    phase_shift = trace_index * 0.35
    frequency = 1.0 + (trace_index * 0.1)

    x_values = rng.normal(
        loc=trace_index * 0.25,
        scale=cluster_scale,
        size=sampled_points,
    ).astype(np.float32)
    noise = rng.normal(
        loc=0.0,
        scale=cluster_scale * 0.35,
        size=sampled_points,
    ).astype(np.float32)
    y_values = (np.sin((x_values + phase_shift) * frequency) + noise).astype(np.float32)

    return x_values, y_values


def build_plot_figure(level, trace_index, equivalent_points, sampled_points):
    """Create one standalone ScatterGL chart for the HTML dashboard."""
    rng = np.random.default_rng(RANDOM_SEED + level)
    for offset in range(trace_index + 1):
        x_values, y_values = build_trace_data(
            rng,
            offset,
            equivalent_points,
            sampled_points,
        )

    figure = go.Figure(
        data=[
            go.Scattergl(
                x=x_values,
                y=y_values,
                mode="markers",
                marker={
                    "size": 2,
                    "opacity": min(0.55 + (0.02 * trace_index), 0.85),
                    "color": y_values,
                    "colorscale": "Viridis",
                    "showscale": False,
                },
                showlegend=False,
            )
        ]
    )

    figure.update_layout(
        title=f"Plot {trace_index + 1}",
        template="plotly_white",
        height=420,
        margin={"l": 40, "r": 20, "t": 56, "b": 40},
        paper_bgcolor="#ffffff",
        plot_bgcolor="#f6f7fb",
        font={"family": "Segoe UI, Arial, sans-serif", "color": "#132238"},
    )
    figure.update_xaxes(
        title_text=f"X Axis {trace_index + 1}",
        gridcolor="#d9dfeb",
        zerolinecolor="#b9c6db",
    )
    figure.update_yaxes(
        title_text=f"Y Axis {trace_index + 1}",
        gridcolor="#d9dfeb",
        zerolinecolor="#b9c6db",
    )

    del x_values
    del y_values
    return figure


def build_html_document(level, equivalent_points, sampled_points):
    """Create a styled HTML report containing ten separate Plotly charts."""
    plot_fragments = []

    for trace_index in range(PLOTS_PER_HTML):
        figure = build_plot_figure(level, trace_index, equivalent_points, sampled_points)
        plot_fragments.append(
            f"""
            <section class=\"plot-card\">
                <div class=\"plot-card__meta\">
                    <span>Scatter {trace_index + 1}</span>
                    <span>{equivalent_points:,} equivalent points</span>
                    <span>{sampled_points:,} sampled points</span>
                </div>
                {pio.to_html(
                    figure,
                    include_plotlyjs=(trace_index == 0),
                    full_html=False,
                    config={"responsive": True, "displayModeBar": False},
                )}
            </section>
            """
        )
        del figure

    return f"""<!DOCTYPE html>
<html lang=\"en\">
<head>
    <meta charset=\"utf-8\">
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">
    <title>ScatterGL Stress Test Level {level}</title>
    <style>
        :root {{
            color-scheme: light;
            --page-bg: linear-gradient(180deg, #f3efe4 0%, #e1edf6 100%);
            --panel-bg: rgba(255, 255, 255, 0.92);
            --panel-border: rgba(19, 34, 56, 0.08);
            --text-main: #132238;
            --text-muted: #5d6f82;
            --accent: #0a7ea4;
            --accent-soft: #d6eef5;
            --shadow: 0 24px 60px rgba(20, 35, 59, 0.12);
        }}

        * {{
            box-sizing: border-box;
        }}

        body {{
            margin: 0;
            font-family: "Segoe UI", Arial, sans-serif;
            background: var(--page-bg);
            color: var(--text-main);
        }}

        .shell {{
            width: min(1440px, calc(100vw - 32px));
            margin: 24px auto 40px;
        }}

        .hero {{
            padding: 32px;
            border: 1px solid var(--panel-border);
            border-radius: 28px;
            background: linear-gradient(135deg, rgba(255,255,255,0.96), rgba(240,247,251,0.96));
            box-shadow: var(--shadow);
            overflow: hidden;
            position: relative;
        }}

        .hero::after {{
            content: "";
            position: absolute;
            inset: auto -120px -120px auto;
            width: 320px;
            height: 320px;
            background: radial-gradient(circle, rgba(10,126,164,0.18) 0%, rgba(10,126,164,0) 70%);
        }}

        .eyebrow {{
            display: inline-flex;
            padding: 8px 12px;
            border-radius: 999px;
            background: var(--accent-soft);
            color: var(--accent);
            font-size: 12px;
            font-weight: 700;
            letter-spacing: 0.08em;
            text-transform: uppercase;
        }}

        h1 {{
            margin: 16px 0 12px;
            font-size: clamp(2rem, 4vw, 3.6rem);
            line-height: 1;
        }}

        .hero p {{
            max-width: 760px;
            margin: 0;
            font-size: 1.02rem;
            line-height: 1.7;
            color: var(--text-muted);
        }}

        .stats-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
            gap: 14px;
            margin-top: 24px;
        }}

        .stat-card {{
            padding: 16px 18px;
            border-radius: 18px;
            background: rgba(255,255,255,0.86);
            border: 1px solid rgba(19,34,56,0.06);
        }}

        .stat-card__label {{
            display: block;
            margin-bottom: 8px;
            font-size: 0.82rem;
            color: var(--text-muted);
            text-transform: uppercase;
            letter-spacing: 0.04em;
        }}

        .stat-card__value {{
            font-size: 1.3rem;
            font-weight: 700;
        }}

        .plots-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(560px, 1fr));
            gap: 22px;
            margin-top: 28px;
        }}

        .plot-card {{
            padding: 18px;
            border-radius: 24px;
            background: var(--panel-bg);
            border: 1px solid var(--panel-border);
            box-shadow: var(--shadow);
            backdrop-filter: blur(10px);
        }}

        .plot-card__meta {{
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
            margin-bottom: 14px;
        }}

        .plot-card__meta span {{
            padding: 7px 10px;
            border-radius: 999px;
            background: #edf4fa;
            color: #29425a;
            font-size: 0.84rem;
        }}

        .plot-card .plotly-graph-div {{
            width: 100% !important;
        }}

        @media (max-width: 700px) {{
            .shell {{
                width: min(100vw - 16px, 100%);
                margin-top: 8px;
            }}

            .hero {{
                padding: 22px;
                border-radius: 20px;
            }}

            .plots-grid {{
                grid-template-columns: 1fr;
            }}
        }}
    </style>
</head>
<body>
    <main class=\"shell\">
        <section class=\"hero\">
            <span class=\"eyebrow\">Plotly ScatterGL stress test</span>
            <h1>Temporal level {level}</h1>
            <p>
                This report contains ten separate ScatterGL charts rendered as standalone Plotly figures.
                The dataset simulates very large matrices by increasing point density while keeping sampled points
                within a memory-safe ceiling.
            </p>
            <div class=\"stats-grid\">
                <article class=\"stat-card\">
                    <span class=\"stat-card__label\">Equivalent points per scatter</span>
                    <span class=\"stat-card__value\">{equivalent_points:,}</span>
                </article>
                <article class=\"stat-card\">
                    <span class=\"stat-card__label\">Sampled points per scatter</span>
                    <span class=\"stat-card__value\">{sampled_points:,}</span>
                </article>
                <article class=\"stat-card\">
                    <span class=\"stat-card__label\">Charts in this file</span>
                    <span class=\"stat-card__value\">{PLOTS_PER_HTML}</span>
                </article>
                <article class=\"stat-card\">
                    <span class=\"stat-card__label\">Safety cap</span>
                    <span class=\"stat-card__value\">{SAFE_SAMPLE_CAP:,}</span>
                </article>
            </div>
        </section>
        <section class=\"plots-grid\">
            {''.join(plot_fragments)}
        </section>
    </main>
</body>
</html>
"""


def generate_html_and_metrics(level, equivalent_points, sampled_points, system_info):
    """Build one HTML file, capture system metrics, and return a CSV-ready record."""
    process = psutil.Process(os.getpid())
    file_name = f"scattergl_temporal_{level:02d}.html"
    file_path = os.path.join(OUTPUT_DIR, file_name)

    ram_before_mb = process.memory_info().rss / (1024 ** 2)
    gpu_usage_before, gpu_memory_before = get_gpu_metrics()
    psutil.cpu_percent(interval=None)

    start_time = time.time()
    html_document = build_html_document(level, equivalent_points, sampled_points)
    with open(file_path, "w", encoding="utf-8") as html_file:
        html_file.write(html_document)
    elapsed_seconds = time.time() - start_time

    ram_after_mb = process.memory_info().rss / (1024 ** 2)
    cpu_usage_percent = psutil.cpu_percent(interval=0.25)
    gpu_usage_after, gpu_memory_after = get_gpu_metrics()
    html_file_size_mb = os.path.getsize(file_path) / (1024 ** 2)

    ram_used_mb = max(ram_after_mb - ram_before_mb, 0.0)
    gpu_usage_percent = gpu_usage_after if not np.isnan(gpu_usage_after) else gpu_usage_before
    gpu_memory_used_mb = (
        gpu_memory_after if not np.isnan(gpu_memory_after) else gpu_memory_before
    )

    print(
        f"Temporal level {level}: "
        f"equivalent={equivalent_points:,}, sampled={sampled_points:,}, "
        f"RAM before={ram_before_mb:.2f} MB, RAM after={ram_after_mb:.2f} MB, "
        f"CPU={cpu_usage_percent:.2f}%, GPU={gpu_usage_percent}, "
        f"GPU memory={gpu_memory_used_mb}, time={elapsed_seconds:.2f}s, "
        f"HTML size={html_file_size_mb:.2f} MB"
    )

    del html_document
    gc.collect()

    return {
        "Temporal level": level,
        "Points per scatter": equivalent_points,
        "Sampled points per scatter": sampled_points,
        "Plots per HTML": PLOTS_PER_HTML,
        "RAM before (MB)": round(ram_before_mb, 2),
        "RAM after (MB)": round(ram_after_mb, 2),
        "RAM used (MB)": round(ram_used_mb, 2),
        "CPU usage (%)": round(cpu_usage_percent, 2),
        "GPU usage (%)": gpu_usage_percent,
        "GPU memory used (MB)": gpu_memory_used_mb,
        "Time taken (seconds)": round(elapsed_seconds, 2),
        "HTML file size (MB)": round(html_file_size_mb, 2),
        "Output file": file_name,
        **system_info,
    }


def main():
    """Run the full ScatterGL stress-test workflow and export the metrics CSV."""
    args = parse_args()
    if args.temporal_differences <= 0:
        raise ValueError("--temporal-differences must be greater than zero.")

    os.makedirs(OUTPUT_DIR, exist_ok=True)
    system_info = get_system_info()
    results = []

    print("Stress test configuration")
    print(f"  Temporal differences: {args.temporal_differences}")
    print(f"  Plots per HTML: {PLOTS_PER_HTML}")
    print(f"  Start points per scatter: {START_POINTS:,}")
    print(f"  Scale factor: {SCALE_FACTOR}")
    print(f"  Equivalent hard cap: {MAX_EQUIVALENT_POINTS:,}")
    print(f"  Safe sampled cap: {SAFE_SAMPLE_CAP:,}")
    print()

    for level in range(1, args.temporal_differences + 1):
        equivalent_points = calculate_equivalent_points(level)
        sampled_points = calculate_sampled_points(equivalent_points)
        results.append(
            generate_html_and_metrics(
                level,
                equivalent_points,
                sampled_points,
                system_info,
            )
        )

    dataframe = pd.DataFrame(results)
    dataframe.to_csv(CSV_NAME, index=False)

    print()
    print(f"Generated {len(results)} HTML files in '{OUTPUT_DIR}'.")
    print(f"Metrics CSV written to '{CSV_NAME}'.")


if __name__ == "__main__":
    main()