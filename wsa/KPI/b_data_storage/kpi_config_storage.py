# KPI Configuration Storage
# Based on InteractivePlot config_storage.py structure but optimized for KPI needs

# KPI-specific configuration for alignment data
KPI_ALIGNMENT_CONFIG = {
    "ALIGNMENT_STREAM": {
        "vacs_boresight_az_nominal": {
            "aliases": ["vacs_boresight_az_nominal", "az_nominal"],
            "call": ["vacs_boresight_az_nominal"],
            "plot_types": ["kpi_alignment_analysis"],
            "unit": "degrees",
            "range_to_be_accepted": [-180, 180],
            "kpi_required": True
        },
        "vacs_boresight_az_estimated": {
            "aliases": ["vacs_boresight_az_estimated", "az_estimated"],
            "call": ["vacs_boresight_az_estimated"],
            "plot_types": ["kpi_alignment_analysis"],
            "unit": "degrees",
            "range_to_be_accepted": [-180, 180],
            "kpi_required": True
        },
        "vacs_boresight_az_kf_internal": {
            "aliases": ["vacs_boresight_az_kf_internal", "az_kf_internal"],
            "call": ["vacs_boresight_az_kf_internal"],
            "plot_types": ["kpi_alignment_analysis"],
            "unit": "degrees",
            "range_to_be_accepted": [-180, 180],
            "kpi_required": True
        },
        "vacs_boresight_el_nominal": {
            "aliases": ["vacs_boresight_el_nominal", "el_nominal"],
            "call": ["vacs_boresight_el_nominal"],
            "plot_types": ["kpi_alignment_analysis"],
            "unit": "degrees",
            "range_to_be_accepted": [-90, 90],
            "kpi_required": True
        },
        "vacs_boresight_el_estimated": {
            "aliases": ["vacs_boresight_el_estimated", "el_estimated"],
            "call": ["vacs_boresight_el_estimated"],
            "plot_types": ["kpi_alignment_analysis"],
            "unit": "degrees",
            "range_to_be_accepted": [-90, 90],
            "kpi_required": True
        },
        "vacs_boresight_el_kf_internal": {
            "aliases": ["vacs_boresight_el_kf_internal", "el_kf_internal"],
            "call": ["vacs_boresight_el_kf_internal"],
            "plot_types": ["kpi_alignment_analysis"],
            "unit": "degrees",
            "range_to_be_accepted": [-90, 90],
            "kpi_required": True
        }
    }
}

# "ALIGNMENT_STREAM": {
#     "vacs_boresight_az_nominal":{},
#     "vacs_boresight_az_estimated":{},
#     "vacs_boresight_az_kf_internal":{},
#     "vacs_boresight_el_nominal":{},
#     "vacs_boresight_el_estimated":{},
#     "vacs_boresight_el_kf_internal":{}
#     },

# KPI-specific configuration for detection data
KPI_DETECTION_CONFIG = {
    "DETECTION_STREAM": {
        "rdd1_dindx": {
            "aliases": ["rdd1_dindx", "detection_index"],
            "call": ["rdd1_dindx"],
            "plot_types": ["kpi_detection_analysis"],
            "kpi_required": True
        },
        "rdd1_rindx": {
            "aliases": ["rdd1_rindx", "range_index"],
            "call": ["rdd1_rindx"],
            "plot_types": ["kpi_detection_analysis"],
            "kpi_required": True
        }
    }
}

# KPI-specific configuration for tracker data
KPI_TRACKER_CONFIG = {
    "TRACKER_STREAM": {
        "track_id": {
            "aliases": ["track_id", "tracker_id"],
            "call": ["track_id"],
            "plot_types": ["kpi_tracker_analysis"],
            "kpi_required": True
        },
        "track_quality": {
            "aliases": ["track_quality", "quality"],
            "call": ["track_quality"],
            "plot_types": ["kpi_tracker_analysis"],
            "range_to_be_accepted": [0, 1],
            "kpi_required": True
        }
    }
}




# KPI validation rules
KPI_VALIDATION_RULES = {
    "alignment_tolerance": 0.1,  # degrees
    "detection_threshold": 0.8,  # percentage
    "tracker_quality_min": 0.7,  # minimum quality score
    "data_completeness_threshold": 0.95  # minimum data completeness
} 