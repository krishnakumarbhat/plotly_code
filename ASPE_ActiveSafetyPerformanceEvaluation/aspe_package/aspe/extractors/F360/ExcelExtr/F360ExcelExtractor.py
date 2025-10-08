# coding=utf-8
"""
F360 Main Extractor
"""
import math
from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.F360.DataSets.F360ExcelExtractedData import F360ExcelExtractedData
from aspe.extractors.F360.DataSets.F360Objects import F360Objects
from aspe.extractors.Interfaces.IExtractor import IExtractor


class F360ExcelExtractor(IExtractor):
    """
    ...
    """
    def __init__(self):
        super().__init__()

    def extract_data(self, events_data):
        """
        Calls builder to build F360 metadata and F360 data container.
        :return: F360ExtractedData object
        """
        extracted = F360ExcelExtractedData()
        extracted.objects = self._build_objects(events_data)

        return extracted

    def _build_objects(self, events_data):
        data_set = F360Objects()

        for event_idx, event in events_data.iterrows():
            object_lifetime = pd.Series(np.arange(event['Tracker Idx Start'], event['Tracker Idx End'] + 1))
            obj_df = pd.DataFrame()
            obj_df['scan_index'] = object_lifetime.values
            obj_df['slot_id'] = event_idx
            obj_df['unique_id'] = event_idx
            obj_df['speed'] = event['Speed [m/s]']
            obj_df['bounding_box_orientation'] = math.radians(event['Heading [deg]'])
            data_set.signals = pd.concat([data_set.signals, obj_df], sort=False)

        data_set.signals.reset_index(inplace=True, drop=True)

        return data_set

    @staticmethod
    def _get_extractor_name():
        return 'F360Excel'
