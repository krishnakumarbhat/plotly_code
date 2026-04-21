# Plotly ScatterGL Stress Test

This project generates multiple heavy Plotly ScatterGL HTML files, measures system performance for each file, and writes the results to a CSV report. Each HTML file contains exactly 10 separate ScatterGL plots presented as individual Plotly chart cards in a styled dashboard layout. The point count starts at 100,000 per plot, scales by a factor of 3 for each temporal level, and respects a hard equivalent cap of 50,000,000 points per plot. To keep the workload runnable, the script uses safe sampled point generation once the equivalent density becomes too large to serialize directly.

## Installation

Install the required Python packages:

```bash
pip install numpy plotly psutil GPUtil pandas
```

## How to Run

Run the script with the number of temporal differences you want to test:

```bash
python scattergl_stress_test.py --temporal-differences 5
```

What the script does:

1. Prints system hardware information before the test loop starts.
2. Creates an `html_outputs` folder in the project root.
3. Generates one HTML file per temporal level.
4. Measures RAM, CPU, GPU, GPU memory, execution time, and HTML file size for each file.
5. Writes all results to `scattergl_system_metrics.csv`.

## Expected Results

After the run finishes, the project directory will contain:

1. An `html_outputs` folder with one standalone HTML dashboard per temporal level.
2. A `scattergl_system_metrics.csv` file with one row per generated HTML file.

The CSV includes the required benchmark fields:

1. Temporal level
2. Points per scatter
3. Plots per HTML
4. RAM used (MB)
5. CPU usage (%)
6. GPU usage (%)
7. GPU memory used (MB)
8. Time taken (seconds)
9. HTML file size (MB)

The CSV also includes RAM before and after each run, the sampled point count used for safety, the output HTML file name, and repeated system hardware details for filtering and comparison.

## How to Analyze the CSV

Open `scattergl_system_metrics.csv` in Excel, pandas, or any BI tool and compare how the metrics change as temporal level increases.

Useful checks:

1. Sort by `Temporal level` to inspect scaling behavior over time.
2. Chart `Time taken (seconds)` against `Points per scatter` to see render cost growth.
3. Chart `HTML file size (MB)` against `Points per scatter` to estimate storage overhead.
4. Track `RAM used (MB)` and `GPU memory used (MB)` to identify system pressure points.
5. Filter on hardware columns when comparing results across different machines.

## Notes

1. If no compatible GPU is detected, the GPU fields may appear as blank values in the CSV.
2. The script uses a safe sampled cap to avoid attempting trillions of literal points or unsafe in-memory datasets.
3. Each HTML file is self-contained, includes custom CSS, and can be opened directly in a browser after generation.