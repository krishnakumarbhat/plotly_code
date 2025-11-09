# HDF Data Dumper

A minimal Python application for extracting and dumping specific datasets from HDF files using DataModelStorage format.

## Structure

```
dc_html/
├── c_data_storage/           # Data storage layer
│   ├── config_storage.py     # Signal configuration
│   ├── data_model_storage.py # DataModelStorage class
│   └── __init__.py
├── d_business_layer/         # Business logic layer
│   ├── data_dumper.py        # Main data dumping functionality
│   ├── utils.py              # Utility functions
│   └── __init__.py
├── html/                     # Output directory
│   ├── Track.txt            # Tracker_Information/OLP data
│   └── OSI track.txt        # OSI_Ground_Truth/Object data
├── hdf_dc.h5                # Input HDF file
├── run_data_dumping.py      # Main execution script
└── README.md                # This file
```

## Usage

```bash
python run_data_dumping.py
```

## Extracted Datasets

### Tracker_Information/OLP
- `vcs_accel_x`, `vcs_accel_y`
- `vcs_pos_x`, `vcs_pos_y`
- `vcs_vel_x`, `vcs_vel_y`

### OSI_Ground_Truth/Object
- `length`, `speed`
- `vcs_lat_accel`, `vcs_lat_posn`, `vcs_lat_vel`
- `vcs_long_accel`, `vcs_long_posn`, `vcs_long_vel`

## Output Format

Data is dumped in DataModelStorage format with:
- Hierarchical organization (Scan indices → Groups → Items → Values)
- Signal mappings for easy data access
- Bidirectional mapping between signals and storage keys
- Parent-child relationships for data organization 