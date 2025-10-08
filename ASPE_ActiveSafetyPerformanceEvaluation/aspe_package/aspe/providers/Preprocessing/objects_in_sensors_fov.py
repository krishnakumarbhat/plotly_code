from copy import deepcopy

import pandas as pd

from aspe.extractors.Interfaces.IObjects import IObjects
from aspe.extractors.Interfaces.IRadarSensors import IRadarSensors
from aspe.extractors.Transform.CrossDataSet.sensor2object import calc_object_fov_info


def are_objects_in_fov(objects: IObjects, sensors: IRadarSensors) -> pd.DataFrame:
    """Checks if any and all reference points of each object are inside the FoV of at least one sensor.

    If an object is inside the FoV of a sensor but that sensor is in invalid state, the object is considered to be
    outside the FoV.

    :param objects: the objects dataset
    :param sensors: the sensors dataset
    :return: pd.Series with the same number of elements as the number of objects. Each index is set to True if the
        corresponding object is inside the FoV; otherwise, it's set to False.
    """
    are_objects_in_fov_df = None
    for sensor_id in sensors.per_sensor["sensor_id"]:
        objects_copy = deepcopy(objects)
        sensors_copy = deepcopy(sensors)

        # It's assumed that sensor ids are unique in the 'per_sensor' dataframe and therefore the dataframe must have
        # one row after the filtering
        sensors_copy.filter_single_radar(sensor_id)
        assert sensors_copy.per_sensor.shape[0] == 1, "sensor id is not unique in the 'per_sensor' dataframe"
        single_sensor_prop = sensors_copy.per_sensor.iloc[0][:]  # Convert from dataframe to series

        objects_copy.vcs2scs(single_sensor_prop)
        sensors_copy.vcs2scs()

        objects_fov_info_df = calc_object_fov_info(objects_copy, sensors_copy)
        in_fov_of_sensor_df = objects_fov_info_df.filter(["f_any_ref_points_in_fov", "f_all_ref_points_in_fov"])

        # If the sensor is not valid then the object is considered to be outside the FoV
        in_fov_of_sensor_df["f_any_ref_points_in_fov"] = (
            in_fov_of_sensor_df["f_any_ref_points_in_fov"] & objects_fov_info_df["sensor_valid"]
        )
        in_fov_of_sensor_df["f_all_ref_points_in_fov"] = (
            in_fov_of_sensor_df["f_all_ref_points_in_fov"] & objects_fov_info_df["sensor_valid"]
        )

        if are_objects_in_fov_df is None:
            are_objects_in_fov_df = in_fov_of_sensor_df
        else:
            # An object is considered inside the FoV if it's inside the FoV of at least one of the sensors; this
            # corresponds to an element-wise logical-or operation
            are_objects_in_fov_df = are_objects_in_fov_df | in_fov_of_sensor_df

    return are_objects_in_fov_df
