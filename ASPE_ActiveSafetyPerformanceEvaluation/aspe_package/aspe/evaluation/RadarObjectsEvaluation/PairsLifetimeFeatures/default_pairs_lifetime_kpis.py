from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.BiasKPI import BiasKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.STDDeviationKPI import STDKPI

DEFAULT_LIFETIME_FEATURES_KPIS = {
    'latency_position_x': (BiasKPI(), STDKPI()),
    'latency_position_y': (BiasKPI(), STDKPI()),
    'latency_velocity_otg_x': (BiasKPI(), STDKPI()),
    'latency_velocity_otg_y': (BiasKPI(), STDKPI()),
    'latency_velocity_rel_x': (BiasKPI(), STDKPI()),
    'latency_velocity_rel_y': (BiasKPI(), STDKPI()),
    'latency_acceleration_otg_x': (BiasKPI(), STDKPI()),
    'latency_acceleration_otg_y': (BiasKPI(), STDKPI()),
    'latency_acceleration_rel_x': (BiasKPI(), STDKPI()),
    'latency_acceleration_rel_y': (BiasKPI(), STDKPI()),
    'latency_bounding_box_orientation': (BiasKPI(), STDKPI()),
    'latency_yaw_rate': (BiasKPI(), STDKPI()),
    'overshoot_acceleration_otg_x_magnitude': (BiasKPI(), STDKPI()),
    'overshoot_acceleration_otg_x_duration': (BiasKPI(), STDKPI()),
    'overshoot_velocity_otg_x_magnitude': (BiasKPI(), STDKPI()),
    'overshoot_velocity_otg_x_duration': (BiasKPI(), STDKPI()),
}

DEF_NO_ACC_REL_LIFETIME_FEATURES_KPIS = {
    'latency_position_x': (BiasKPI(), STDKPI()),
    'latency_position_y': (BiasKPI(), STDKPI()),
    'latency_velocity_otg_x': (BiasKPI(), STDKPI()),
    'latency_velocity_otg_y': (BiasKPI(), STDKPI()),
    'latency_velocity_rel_x': (BiasKPI(), STDKPI()),
    'latency_velocity_rel_y': (BiasKPI(), STDKPI()),
    'latency_acceleration_otg_x': (BiasKPI(), STDKPI()),
    'latency_acceleration_otg_y': (BiasKPI(), STDKPI()),
    'latency_bounding_box_orientation': (BiasKPI(), STDKPI()),
    'latency_yaw_rate': (BiasKPI(), STDKPI()),
}

DEF_ACC_REL_LIFETIME_FEATURES_KPIS = {
    'overshoot_acceleration_otg_x_magnitude': (BiasKPI(), STDKPI()),
    'overshoot_acceleration_otg_x_duration': (BiasKPI(), STDKPI()),
    'overshoot_velocity_otg_x_magnitude': (BiasKPI(), STDKPI()),
    'overshoot_velocity_otg_x_duration': (BiasKPI(), STDKPI()),
}