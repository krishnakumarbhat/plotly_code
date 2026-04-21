import pandas as pd
import matplotlib.pyplot as plt
import os

# --- Input: Excel file path ---
excel_file = r"C:\Users\h5utrr\Documents\logs\logs\output_phi_in.xlsx" # Replace with your actual file path

# --- Read Excel file ---
df = pd.read_excel(excel_file)
df.columns = df.columns.str.strip()

# --- Check if required columns are present ---
required_cols = ['scanindex1', 'phi1', 'scanindex2', 'phi2']
if not all(col in df.columns for col in required_cols):
    raise ValueError("One or more required columns are missing in the Excel file.")

# --- Extract first 2 digits after decimal as strings ---
def extract_two_decimal_digits(val):
    try:
        return str(val).split(".")[1][:2] if "." in str(val) else "00"
    except:
        return "00"

df['phi1_dec2'] = df['phi1'].apply(extract_two_decimal_digits)
df['phi2_dec2'] = df['phi2'].apply(extract_two_decimal_digits)

# --- Group phi2 decimal parts by scanindex2 ---
phi2_groups = df.groupby('scanindex2')['phi2_dec2'].apply(list).to_dict()

# --- Identify mismatches ---
def is_mismatch(row):
    scanindex = row['scanindex1']
    val = row['phi1_dec2']
    return val not in phi2_groups.get(scanindex, [])

mismatches = df[df.apply(is_mismatch, axis=1)]

# --- Create output DataFrame ---
output_df = mismatches[['scanindex1', 'phi1', 'phi2']].copy()
output_df.rename(columns={'scanindex1': 'mismatched_scanindex'}, inplace=True)

# --- Save to Excel ---
output_path = os.path.join(os.path.dirname(excel_file), "output_mismatch_by_decimal_digits.xlsx")
output_df.to_excel(output_path, index=False)

# --- Plotting ---
plt.figure(figsize=(10, 6))
plt.scatter(output_df['mismatched_scanindex'], output_df['phi1'], color='blue', label='phi1', s=40)
plt.scatter(output_df['mismatched_scanindex'], output_df['phi2'], color='red', label='phi2', s=40)

plt.title("Mismatch Plot: phi1 vs phi2 by Scanindex (First 2 Decimal Digits)")
plt.xlabel("Scanindex")
plt.ylabel("phiocity Value")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(os.path.join(os.path.dirname(excel_file), "phi_mismatch_by_decimal_digits_plot.png"))
plt.show()

print(f"✅ Mismatches saved to: {output_path}")
