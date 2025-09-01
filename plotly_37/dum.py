import numpy as np
import plotly.express as px
import pandas as pd

output_data = {
    19: [
        [np.array([50.49, 83.53, 57.08, 95.01, 88.77]),
         np.array([176.53, 170.17, 195.76, 185.84, 167.13]),
         np.array([1101.38, 1105.79, 1106.63, 1103.71, 1101.97]),
         np.array([285, 206, 142, 214, 237]),
         np.array([1002, 1004, 1008, 1006, 1002])],
        [np.array([206, 209, 283, 282, 102]),
         np.array([260, 132, 108, 114, 178])]
    ],
    20: [
        [np.array([82.58, 84.65, 54.12, 66.60, 87.98]),
         np.array([176.08, 170.39, 178.74, 183.27, 181.23]),
         np.array([1100.63, 1107.38, 1105.29, 1108.99, 1107.66]),
         np.array([114, 181, 214, 121, 173]),
         np.array([1001, 1007, 1007, 1003, 1006])],
        [np.array([285, 120, 167, 260, 255]),
         np.array([207, 231, 115, 225, 200])]
    ]
}

data = []

for key in [19, 20]:

    measurement_array = output_data[key][0][0]

    scan_indices = [key] * len(measurement_array)

    for x, y in zip(scan_indices, measurement_array):
        data.append({"Scan Index": x, "Measurement Value": y})

df = pd.DataFrame(data)

fig = px.scatter(
    df,
    x="Scan Index",
    y="Measurement Value",
    color="Key",
    title="Scatter Plot of Measurement Values with Key 19 and 20"
)

fig.show()
