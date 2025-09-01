import csv
import re

csv_filename = r"C:/Users/ouymc2/Desktop/plotly_35/KPI_Script/csv_db/DEV_X8310_236_SDV_4SRad_20241212_085005_000_ESW-SRad9-164_Aptiv_FL_UDP_GEN7_ROT_OBJECT_STREAM.csv"

name_set = set()

# Pattern: Capture only the leading letters (A-Za-z)
# This will remove everything after the first non-letter character.
remove_after_digit_pattern = re.compile(r"^([A-Za-z]+).*")

with open(csv_filename, newline="") as csvfile:
    reader = csv.reader(csvfile)
    
    # Retrieve the first row (if available)
    try:
        row = next(reader)
    except StopIteration:
        row = []

    for value in row:
        value = value.strip()
        # Use re.sub to replace the whole string with only the captured letters
        new_value = re.sub(remove_after_digit_pattern, r"\1", value)
        name_set.add(new_value)

print("Unique names:", name_set)
