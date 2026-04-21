load("@rules_cc//cc:defs.bzl", "cc_library")



# load("@bazel_skylib//rules:common_settings.bzl", "string_flag")


# cc_binary(
#     name = "f360_tracker_lib",
#     srcs = glob(["Fusion360/**/*.cpp", "integration_testing/integtest/**/*.cpp", "SharedTrackerAPI/**/*.cpp", "StateManager/source/**/*.cpp"]),
#     hdrs = glob(["Fusion360/**/*.h", "SharedTrackerAPI/**/*.h"]),
#     includes = ["Fusion360", "SharedTrackerAPI"],
#     visibility = ["//visibility:public"],
#     copts = ["-std=c++11"],  # Adjust to your C++ version
# )

# cc_binary(
#     name = "f360_tracker_exe",
#     srcs = ["main.cpp"],  # Assuming main.cpp is your entry point
#     deps = [":f360_tracker_lib"],  # Link against the library we defined earlier
#     visibility = ["//visibility:public"],
# )


# Set up all existing subfolders, each containing a BUILD file
# SRC_DIRS = [
#    "Fusion360/autocode_reuse",
#    "Fusion360/cluster_grouping",
#    "Fusion360/clustering",
#    "Fusion360/common",
#    "Fusion360/detection_to_track_association",
#    "Fusion360/e2e_protection",
#    "Fusion360/include",
#    "Fusion360/initialize_tracks",
#    "Fusion360/inputs_preprocessing",
#    "Fusion360/internal_preprocessing",
#    "Fusion360/logging",
#    "Fusion360/measurement_update_tracks",
#    "Fusion360/multipath_detector",
#    "Fusion360/occlusion",
#    "Fusion360/post_update_track_adjustments",
#    "Fusion360/pre_association_track_management",
#    "Fusion360/sensor_postprocessing",
#    "Fusion360/static_environment",
#    "Fusion360/time_update_tracks",
#    "Fusion360/track_classification",
#    "Fusion360/track_downselection",
#    "Fusion360/track_grouping",
#    "Fusion360/track_validity",
#    "Fusion360/update_relative_timestamps",
#    "Fusion360/trailer_detector",
# ]




cc_library(
    name = "f360_tracker_lib",
    srcs = [
        "Fusion360/f360_calibrations.cpp",
        "Fusion360/f360_tracker.cpp",
        "Fusion360/f360_xtrk_logging.cpp",
        "Fusion360/f360_xtrk_logging.h",
        "Timing/f360_get_wall_time.cpp",
    ],
    hdrs = [
        "Fusion360/clustering/include/f360_reuse.h",
        # "StateManager/source/f360_input_diagnostics.h",
        # "StateManager/source/f360_output_diagnostics.h",
        # "StateManager/source/f360_safety_control_logic.h",
        # "StateManager/source/State_Manager.h",
    ],
    includes = [

        # "../../../dss/src/F360Tracker/Fusion360/include",
        "Fusion360/include",
        "SharedTrackerAPI/core",
        "SharedTrackerAPI/Types",
        # "../../../dss/src/F360Tracker/SharedTrackerAPI/Types",
        # "../../../dss/src/F360Tracker/SharedTrackerAPI/core",
        "Fusion360/clustering/include",
        # "SharedTrackerAPI/core",
        # "SharedTrackerAPI/Logging",
        # "SharedTrackerAPI/Types",
    ],

    # copts = ["-DMAX_NR_OF_VCS_LONG_SORTED_DETS_REF_POINTS=31"],

    # copts = [
    #     "-DMAX_NR_OF_VCS_LONG_SORTED_DETS_REF_POINTS=31",
    #     "-std=c++11"
    # ],
    visibility = ["//visibility:public"],
    # copts = ["-std=c++11"],
    # deps = [aurocode_resue

    # "/SharedTrackerAPI:dependency1",
    # "//SharedTrackerAPI:dependency2",
    # ],
)

cc_library(
    name = "autocode_reuse",
    srcs = [
        "Fusion360/autocode_reuse/source/f360_LinearSolvers.cpp",
        "Fusion360/autocode_reuse/source/f360_matrix_vector_Init_real32_T.cpp",
        # "Fusion360/autocode_reuse/unittest/f360_LinearSolvers_unittest.cpp",
        # "Fusion360/autocode_reuse/unittest/f360_matrix_vector_Init_real32_T_unittest.cpp"
    ],
    hdrs = [
        "Fusion360/autocode_reuse/include/f360_LinearSolvers.h",
        "Fusion360/autocode_reuse/include/f360_linear_solver_data_types.h",
        "Fusion360/autocode_reuse/include/f360_matrix_vector_Init_real32_T.h",
    ],
    defines = ["ROT_STANDALONE"],
    strip_include_prefix = "Fusion360/autocode_reuse/include",
    #strip_include_prefix = "",
    visibility = ["//visibility:public"],
    deps = [
        ":Fusion360_include",
        # ":SharedTrackerAPI_Types",
        ":SharedTrackerAPI_core",
        "//tracker_emb_lib/F360Tracker/F360TrackerLib/SharedTrackerAPI/Types:SharedTrackerAPI_Types"
    ],
)

cc_library(
    name = "Fusion360_include",
    srcs = [],
    hdrs = [
        "Fusion360/include/f360_accel.h",
        "Fusion360/include/f360_angle.h",
        "Fusion360/include/f360_any_other_assoc_det_close.h",
        "Fusion360/include/f360_bounding_box.h",
        "Fusion360/include/f360_calculate_priority.h",
        "Fusion360/include/f360_calibrations.h",
        "Fusion360/include/f360_circular_buffer.h",
        # "Fusion360/include/f360_cluster.h",
        "Fusion360/include/f360_cluster_motion_status.h",
        "Fusion360/include/f360_clustering_configuration.h",
        "Fusion360/include/f360_concrete_wall_detector.h",
        "Fusion360/include/f360_concrete_wall_measurements.h",
        "Fusion360/include/f360_concrete_wall_sensor.h",
        "Fusion360/include/f360_concrete_wall_side.h",
        "Fusion360/include/f360_conf.h",
        "Fusion360/include/f360_convert_tcs_posn_to_vcs_posn.h",
        # "Fusion360/include/f360_covariances.h",
        "Fusion360/include/f360_current_msmt_type.h",
        "Fusion360/include/f360_dead_zone_status.h",
        "Fusion360/include/f360_det_cross_covariances.h",
        "Fusion360/include/f360_detection_2_cluster_type.h",
        "Fusion360/include/f360_detection_hist.h",
        # "Fusion360/include/f360_detection_motion_status.h",
        # "Fusion360/include/f360_detection_posn_hist.h",
        "Fusion360/include/f360_detection_props.h",
        "Fusion360/include/f360_detection_time_slots.h",
        "Fusion360/include/f360_detection_wheelspin_type.h",
        "Fusion360/include/f360_dimensions.h",
        "Fusion360/include/f360_functional.h",
        "Fusion360/include/f360_globals.h",
        "Fusion360/include/f360_host_props.h",
        "Fusion360/include/f360_internal_reflection_buffer_slot.h",
        "Fusion360/include/f360_interval.h",
        "Fusion360/include/f360_iterator.h",
        "Fusion360/include/f360_iterator_detail.h",
        "Fusion360/include/f360_line.h",
        # "Fusion360/include/f360_line_segment.h",
        "Fusion360/include/f360_local_clusters.h",
        "Fusion360/include/f360_longi_stat_curve.h",
        "Fusion360/include/f360_math_func.h",
        "Fusion360/include/f360_math.h",
        "Fusion360/include/f360_matrix_dimension.h",
        "Fusion360/include/f360_nees_cfmi_vel_hyp_source.h",
        "Fusion360/include/f360_norm_heading_angle.h",
        "Fusion360/include/f360_object_class_probability_type.h",
        "Fusion360/include/f360_object_class.h",
        "Fusion360/include/f360_object_occlusion_status.h",

        # "Fusion360/include/f360_object_linear_moving_type.h",
        "Fusion360/include/f360_object_sides.h",
        "Fusion360/include/f360_object_status.h",
        "Fusion360/include/f360_object_track.h",
        "Fusion360/include/f360_point.h",
        "Fusion360/include/f360_polygon.h",
        "Fusion360/include/f360_pseudo_msmt.h",
        "Fusion360/include/f360_radar_sensor_props.h",
        "Fusion360/include/f360_reference_point_candidate.h",
        "Fusion360/include/f360_reference_point.h",
        "Fusion360/include/f360_rotate_state_covariance_matrix.h",
        "Fusion360/include/f360_sort_data_type.h",
        "Fusion360/include/f360_static_env_helpers.h",
        "Fusion360/include/f360_static_env_poly_types.h",
        "Fusion360/include/f360_static_environment_class.h",
        "Fusion360/include/f360_timing_info.h",
        "Fusion360/include/f360_track_init.h",
        "Fusion360/include/f360_tracked_object_init_info.h",
        "Fusion360/include/f360_tracker_info.h",
        "Fusion360/include/f360_trk_fltr_type.h",
        # "Fusion360/include/f360_underdrivability_enum.h",
        # "Fusion360/include/f360_underdrivability_states.h",
        # "Fusion360/include/f360_underdrivability_type.h",
        # "Fusion360/include/f360_update_object_reference_point.h",
        "Fusion360/include/f360_vector.h",
        "Fusion360/include/rspp_calibrations.h",
    ],
    # deps = [
    #     ":f360_h38",
    #     ":f360_h46",
    # ],
    deps = [
        "//tracker_emb_lib/F360Tracker/F360TrackerLib/SharedTrackerAPI/Types:SharedTrackerAPI_Types"
    ],
    defines = ["ROT_STANDALONE"],
    strip_include_prefix = "Fusion360/include",
    visibility = ["//visibility:public"],
)


cc_library(
    name = "SharedTrackerAPI_core",
    srcs = [],
    hdrs = [
        "SharedTrackerAPI/core/State_Manager.h",
        "SharedTrackerAPI/core/f360_constants.h",
        "SharedTrackerAPI/core/f360_get_wall_time.h",
        "SharedTrackerAPI/core/f360_input_diagnostics.h",
        "SharedTrackerAPI/core/f360_input_faults.h",
        "SharedTrackerAPI/core/f360_ocg_interface.h",
        "SharedTrackerAPI/core/f360_output_diagnostics.h",
        "SharedTrackerAPI/core/f360_previous_cycle_info.h",
        "SharedTrackerAPI/core/f360_safety_control_logic.h",
        "SharedTrackerAPI/core/f360_tracker.h",
        "SharedTrackerAPI/core/f360_tracker_version.h",
        "SharedTrackerAPI/core/f360_variant.h",
        "SharedTrackerAPI/core/f360_variant_definition.h",
    ],
    defines = ["ROT_STANDALONE"],
    strip_include_prefix = "SharedTrackerAPI/core",
    visibility = ["//visibility:public"],
    deps = ["//tracker_emb_lib/F360Tracker/F360TrackerLib/SharedTrackerAPI/Types:SharedTrackerAPI_Types"],
)





# cc_library(
#     name = "SharedTrackerAPI_Types",
#     srcs = [],
#     hdrs = [
#         #  "SharedTrackerAPI/Types/T360_Types.h",
#         "SharedTrackerAPI/Types/f360_core_info.h",
#         # "SharedTrackerAPI/Types/f360_detection.h",
#         # "SharedTrackerAPI/Types/f360_detection_list.h",
#         "SharedTrackerAPI/Types/f360_host.h",
#         "SharedTrackerAPI/Types/f360_host_calib.h",
#         "SharedTrackerAPI/Types/f360_host_raw.h",
#         "SharedTrackerAPI/Types/f360_look_ID.h",
#         "SharedTrackerAPI/Types/f360_look_type.h",
#         "SharedTrackerAPI/Types/f360_mounting_location.h",
#         "SharedTrackerAPI/Types/f360_position.h",
#         "SharedTrackerAPI/Types/f360_radar_sensor.h",
#         # "SharedTrackerAPI/Types/f360_radar_sensor_calib.h",
#         "SharedTrackerAPI/Types/f360_range_type.h",
#         "SharedTrackerAPI/Types/f360_reuse.h",
#         "SharedTrackerAPI/Types/f360_sensor_mounting_position.h",
#         "SharedTrackerAPI/Types/f360_sensor_type.h",
#         "SharedTrackerAPI/Types/f360_trk_fltr_cca_states_logging.h",
#         "SharedTrackerAPI/Types/f360_trk_fltr_cca_states.h",
#         "SharedTrackerAPI/Types/f360_trk_fltr_ccv_states.h",
#         "SharedTrackerAPI/Types/f360_trk_fltr_ctca_states.h",
#         "SharedTrackerAPI/Types/T360_Types.h",
#         "SharedTrackerAPI/Types/f360_variant_type.h",
#         "SharedTrackerAPI/Types/f360_velocity.h",
#         # "//tracker_emb_lib/f360tracker/f360trackerlib/SharedTrackerAPI/Types:f360_reuse.h",
#     ],
#     # includes = ["../Types"],
#     # deps = ["//SharedTrackerAPI/Types"],
#     defines = ["ROT_STANDALONE"],
#     strip_include_prefix = "SharedTrackerAPI/Types",
#     visibility = ["//visibility:public"],
# )