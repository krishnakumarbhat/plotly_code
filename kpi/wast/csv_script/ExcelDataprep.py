import pandas as pd
from matplotlib import pyplot as plt
# Step 1: Load CSV
input_csv = r"C:\Users\h5utrr\Documents\logs\logs\CCA_9010_3100099_DEBUG_20250707_154505_0005_r00060103_FC_UDP_GEN7_DET_CORE.csv"
# Update your file path
df = pd.read_csv(input_csv)

# Step 2: Clean - Drop duplicate scanindex rows
if 'scanindex' not in df.columns:
    raise ValueError("Missing 'scanindex' column in input file.")

df = df.drop_duplicates(subset='scanindex')

# Step 3: Extract only ran_0 to ran_679 columns
ran_columns = [f'rcs_{i}' for i in range(680)]
missing_columns = [col for col in ran_columns if col not in df.columns]
if missing_columns:
    raise ValueError(f"Missing columns in input file: {missing_columns}")

# Step 4: Create output DataFrame
output_rows = []

for _, row in df.iterrows():
    scanindex_val = row['scanindex']
    range_values = row[ran_columns].values.tolist()

    for val in range_values:
        output_rows.append({'scanindex': scanindex_val, 'range': val})

# Step 5: Save to Excel
output_df = pd.DataFrame(output_rows)
output_df.to_excel("output.xlsx", index=False)
plt.scatter(output_df.scanindex, output_df.range,s=1)
plt.show()
print("✅ Transformation complete! Output saved as 'output.xlsx'")
