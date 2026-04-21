load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "rspp_iface_variant_C",
    hdrs = [
        "rspp_iface_variant_C/rspp_calibrations.h",
        "rspp_iface_variant_C/rspp_constants.h",
        "rspp_iface_variant_C/rspp_core_info.h",
        "rspp_iface_variant_C/rspp_detection.h",
        "rspp_iface_variant_C/rspp_detection_list.h",
        "rspp_iface_variant_C/rspp_detection_motion_status.h",
        "rspp_iface_variant_C/rspp_get_version.h",
        "rspp_iface_variant_C/rspp_host.h",
        "rspp_iface_variant_C/rspp_inputs_preprocessing.h",
        "rspp_iface_variant_C/rspp_look_ID.h",
        "rspp_iface_variant_C/rspp_look_type.h",
        "rspp_iface_variant_C/rspp_math.h",
        "rspp_iface_variant_C/rspp_mounting_location.h",
        "rspp_iface_variant_C/rspp_position.h",
        "rspp_iface_variant_C/rspp_radar_sensor.h",
        "rspp_iface_variant_C/rspp_range_type.h",
        "rspp_iface_variant_C/rspp_reuse.h",
        "rspp_iface_variant_C/rspp_sensor_mounting_position.h",
        "rspp_iface_variant_C/rspp_sensor_type.h",
        "rspp_iface_variant_C/rspp_variant_definition.h",
        "rspp_iface_variant_C/rspp_variant_type.h",
        "rspp_iface_variant_C/rspp_velocity.h",
        "rspp_iface_variant_C/rspp_version.h",
    ],
    strip_include_prefix = "rspp_iface_variant_C",
    visibility = ["//visibility:public"],
)

cc_import(
    name = "librspp_variant_C_a",
    static_library = "librspp_variant_C.a",
    visibility = ["//visibility:public"],
)
