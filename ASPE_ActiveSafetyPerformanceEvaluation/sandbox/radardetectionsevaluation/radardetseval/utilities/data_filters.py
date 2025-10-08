from copy import deepcopy
from typing import List
import pandas as pd


def filter_out_single_radar(data_in, radar_id=1):
    data_out = deepcopy(data_in)
    data_out.detections.signals = data_out.detections.signals[data_out.detections.signals.sensor_id == radar_id]
    data_out.sensors.signals = data_out.sensors.signals[data_out.sensors.signals.sensor_id == radar_id]
    data_out.sensors.per_look = data_out.sensors.per_look[data_out.sensors.per_look.sensor_id == radar_id]
    data_out.sensors.per_sensor = data_out.sensors.per_sensor[data_out.sensors.per_sensor.sensor_id == radar_id]
    return data_out


def slice_df_based_on_other(left_df: pd.DataFrame, right_df: pd.DataFrame, left_on: List, right_on: List):
    right_df_index = right_df[right_on]
    right_df_index.columns = left_on
    sliced_left_df = right_df_index.merge(left_df, on=left_on)
    return sliced_left_df
