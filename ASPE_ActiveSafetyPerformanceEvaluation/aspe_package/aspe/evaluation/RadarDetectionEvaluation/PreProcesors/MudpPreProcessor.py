from copy import deepcopy

import pandas as pd

from aspe.evaluation.RadarDetectionEvaluation.PreProcesors.IPreProcessor import IPreProcessor
from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.extractors.Transform.CrossDataSet.sensor2object import calc_object_fov_info


class MudpPreProcessor(IPreProcessor):

    def pre_process_data(self, estimated_data: ExtractedData, reference_data: ExtractedData, sensor_id: int) \
            -> (ExtractedData, ExtractedData):
        """
        Pre-process data:
            - copy data
            - filter single sensor data
            - convert from vcs to scs
                - estimated detections
                - reference objects
                - reference sensor
            - assign FoV expectation to object data

        :param estimated_data:
        :type estimated_data: ExtractedData
        :param reference_data:
        :type reference_data: ExtractedData
        :param sensor_id: sensor which should be filtered
        :type sensor_id: int
        :return: (ExtractedData, ExtractedData)
        """

        # Copy data
        est_synch = deepcopy(estimated_data)
        ref_synch = deepcopy(reference_data)

        # Filter only single sensor
        est_synch.detections.filter_single_radar(sensor_id)
        ref_synch.sensors.filter_single_radar(sensor_id)

        # VCS to SCS transformation
        if ref_synch.sensors.per_sensor.empty:
            ref_synch.objects.signals.drop(ref_synch.objects.signals.index, inplace=True)
            ref_synch.sensors.signals.drop(ref_synch.sensors.signals.index, inplace=True)
            est_synch.detections.signals.drop(est_synch.detections.signals.index, inplace=True)

        else:
            sensor_mounting_loc = ref_synch.sensors.per_sensor.iloc[0][:]
            est_synch.detections.vcs2scs(sensor_mounting_loc)
            ref_synch.objects.vcs2scs(sensor_mounting_loc)
            ref_synch.sensors.vcs2scs()

            # Additional information
            object_fov_info = calc_object_fov_info(ref_synch.objects, ref_synch.sensors)
            ref_synch.objects.signals = pd.concat([ref_synch.objects.signals, object_fov_info], axis=1)

        return est_synch, ref_synch
