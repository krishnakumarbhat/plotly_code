# Fixes Needed for detection_matching_kpi.py

## Summary of Issues

Your numpy conversion has **critical bugs** that cause the errors you're experiencing:

### Issue 1: `KPI_DataModelStorage.get_value()` returns a TUPLE
- **Returns:** `(numpy_array, status_string)` 
- **Current code** treats it as just an array → causes "tuple has no attribute 'size'" error

### Issue 2: NaN handling causes integer conversion errors
- The error "cannot convert float NaN to integer" happens when trying to convert NaN values
- Need to filter out NaN values BEFORE converting to int

### Issue 3: Incorrect data structure understanding
- The 2D array from `get_value()` has structure: `[scan_index, value1, value2, ...]`
- Column 0 is always the scan_index
- Column 1+ contains the actual data values

---

## Required Fixes

### Fix 1: In `process_detection_matching()` (lines 80-90)

**WRONG:**
```python
for param in det_params:
    try:
        veh_det_df[param] = KPI_DataModelStorage.get_value(src['input'], param)
        sim_det_df[param] = KPI_DataModelStorage.get_value(src['output'], param)
```

**CORRECT:**
```python
for param in det_params:
    try:
        # get_value returns (numpy_array, status_string)
        veh_result, veh_status = KPI_DataModelStorage.get_value(src['input'], param)
        sim_result, sim_status = KPI_DataModelStorage.get_value(src['output'], param)
        
        if veh_status == "success" and sim_status == "success":
            veh_det_df[param] = veh_result
            sim_det_df[param] = sim_result
        else:
            logger.warning(f"Failed to get parameter {param}: veh={veh_status}, sim={sim_status}")
            veh_det_df[param] = np.array([])
            sim_det_df[param] = np.array([])
```

### Fix 2: In `process_rdd_matching()` (lines 143-156)

**WRONG:**
```python
for param in rdd_params:
    try:
        veh_rdd[param] = KPI_DataModelStorage.get_value(src['input'], param)
        sim_rdd[param] = KPI_DataModelStorage.get_value(src['output'], param)
```

**CORRECT:**
```python
for param in rdd_params:
    try:
        veh_result, veh_status = KPI_DataModelStorage.get_value(src['input'], param)
        sim_result, sim_status = KPI_DataModelStorage.get_value(src['output'], param)
        
        if veh_status == "success" and sim_status == "success":
            veh_rdd[param] = veh_result
            sim_rdd[param] = sim_result
        else:
            logger.warning(f"Failed to get RDD parameter {param}: veh={veh_status}, sim={sim_status}")
            return {}
```

### Fix 3: Extract data correctly from 2D arrays (add after line 156)

**ADD THIS HELPER FUNCTION:**
```python
# Extract data from 2D arrays: column 0 is scan_index, column 1 is the actual value
def extract_data(arr_2d):
    """Extract scan indices and values from get_value() result."""
    if arr_2d.size == 0:
        return np.array([]), np.array([])
    if arr_2d.ndim == 1:
        # Single row case
        return np.array([arr_2d[0]]), arr_2d[1:] if len(arr_2d) > 1 else np.array([arr_2d[0]])
    # Multiple rows: column 0 is scan_index, column 1 is value
    scan_indices = arr_2d[:, 0].astype(int)
    values = arr_2d[:, 1] if arr_2d.shape[1] > 1 else arr_2d[:, 0]
    return scan_indices, values

# Extract scan indices and values for rdd1_rindx (range index)
veh_scan, veh_rindx_vals = extract_data(veh_rdd['rdd1_rindx'])
sim_scan, sim_rindx_vals = extract_data(sim_rdd['rdd1_rindx'])

# Extract num_detect values
_, veh_num = extract_data(veh_rdd['rdd1_num_detect'])
_, sim_num = extract_data(sim_rdd['rdd1_num_detect'])

# Validate we have data
if veh_scan.size == 0 or sim_scan.size == 0:
    logger.warning("No RDD data available for matching")
    return {}
```

**THEN REPLACE lines 132-135:**
```python
# OLD (WRONG):
veh_scan = veh_rdd['rdd1_rindx']
sim_scan = sim_rdd['rdd1_rindx']
veh_num = veh_rdd['rdd1_num_detect']
sim_num = sim_rdd['rdd1_num_detect']
```
**DELETE THESE** - they're already extracted above

### Fix 4: Handle NaN values before integer conversion (around line 208-210)

**ADD AFTER extracting matched values:**
```python
# Extract corresponding num_detect values
matched_veh_nums = veh_num[matched_veh_indices]
matched_sim_nums = sim_num[matched_sim_indices]

# Handle NaN values in num_detect
veh_valid = ~np.isnan(matched_veh_nums)
sim_valid = ~np.isnan(matched_sim_nums)
both_valid = veh_valid & sim_valid

if not np.any(both_valid):
    logger.warning("All RDD num_detect values are NaN")
    return {}

# Filter to only valid entries
matched_veh_nums = matched_veh_nums[both_valid]
matched_sim_nums = matched_sim_nums[both_valid]

num_same_si = int(np.sum(both_valid))  # Now safe to convert to int
num_same_num_dets = int(np.sum(matched_veh_nums == matched_sim_nums))
```

### Fix 5: `process_af_detection_matching()` needs complete rewrite

The pandas code won't work with numpy dictionaries. Replace the entire try block (lines 303-353) with:

```python
# Extract scan indices and num_af_det from the 2D arrays
def extract_data(arr_2d):
    """Extract scan indices and values from get_value() result."""
    if arr_2d.size == 0:
        return np.array([]), np.array([])
    if arr_2d.ndim == 1:
        return np.array([arr_2d[0]]), arr_2d[1:] if len(arr_2d) > 1 else np.array([arr_2d[0]])
    scan_indices = arr_2d[:, 0].astype(int)
    values = arr_2d[:, 1] if arr_2d.shape[1] > 1 else arr_2d[:, 0]
    return scan_indices, values

# Extract num_af_det data
veh_scan, veh_num_det = extract_data(veh_det_df.get('num_af_det', np.array([])))
sim_scan, sim_num_det = extract_data(sim_det_df.get('num_af_det', np.array([])))

if veh_scan.size == 0 or sim_scan.size == 0:
    logger.warning("No AF detection data available")
    return {}

# Build mapping for matching scan indices
sim_map = {}
for idx, scan in enumerate(sim_scan):
    if scan not in sim_map:
        sim_map[scan] = idx

# Find matching scans
matching_scans = np.array([s for s in np.unique(veh_scan) if s in sim_map])

if matching_scans.size == 0:
    logger.warning("No matching scan indices found for AF detection")
    return {}

# Build veh mapping
veh_map = {}
for idx, scan in enumerate(veh_scan):
    if scan not in veh_map:
        veh_map[scan] = idx

matched_veh_indices = np.array([veh_map[s] for s in matching_scans])
matched_sim_indices = np.array([sim_map[s] for s in matching_scans])

# Get matched num_detect values
matched_veh_nums = veh_num_det[matched_veh_indices]
matched_sim_nums = sim_num_det[matched_sim_indices]

# Handle NaN values
veh_valid = ~np.isnan(matched_veh_nums)
sim_valid = ~np.isnan(matched_sim_nums)
both_valid = veh_valid & sim_valid

if not np.any(both_valid):
    logger.warning("All AF num_det values are NaN")
    return {}

# Filter to valid entries
matched_veh_nums = matched_veh_nums[both_valid]
matched_sim_nums = matched_sim_nums[both_valid]

num_same_si = int(np.sum(both_valid))
num_same_num_dets = int(np.sum(matched_veh_nums == matched_sim_nums))
total_detections_veh = int(np.sum(matched_veh_nums))

# Simplified matching
matched_detections = num_same_num_dets

same_num_dets_pct = round((num_same_num_dets / num_same_si * 100), 2) if num_same_si > 0 else 0.0
accuracy = round((matched_detections / total_detections_veh * 100), 2) if total_detections_veh > 0 else 0.0

af_kpis = {
    'num_same_si': num_same_si,
    'num_same_num_dets': num_same_num_dets,
    'same_num_dets_pct': same_num_dets_pct,
    'total_detections_veh': total_detections_veh,
    'matched_detections': matched_detections,
    'accuracy': accuracy
}

return af_kpis
```

---

## Quick Manual Fix Steps

1. Open `detection_matching_kpi.py`
2. Search for `KPI_DataModelStorage.get_value` (there are 4 occurrences)
3. For each occurrence, change from:
   - `var = KPI_DataModelStorage.get_value(...)`
   - TO: `var, status = KPI_DataModelStorage.get_value(...)`
   - Then check if `status == "success"` before using `var`
4. Add NaN filtering before any `int()` conversions
5. Add the `extract_data()` helper function to properly parse 2D arrays

---

## Root Cause

The `get_value()` method in `kpi_data_model_storage.py` line 328 returns:
```python
return stacked, "success"  # Returns TUPLE (array, status)
```

Your code assumed it returned just the array, causing the tuple/attribute errors.
