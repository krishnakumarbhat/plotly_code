import os

import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.DataStructures import PESingleObjects
from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PEPairedObjects import PEPairedObjects
from aspe.extractors.F360.DataSets.F360BmwDetectionList import F360BmwDetectionList
from aspe.extractors.F360.DataSets.F360BmwDetectionListHeader import F360BmwDetectionListHeader
from aspe.extractors.F360.DataSets.F360BmwObjectList import F360BmwObjectList
from aspe.extractors.F360.DataSets.F360BmwObjectListHeader import F360BmwObjectListHeader
from aspe.extractors.F360.DataSets.F360BmwRadarSensors import F360BmwRadarSensors
from aspe.extractors.F360.DataSets.F360Host import F360Host
from aspe.extractors.F360.DataSets.F360InternalObjects import F360InternalObjects
from aspe.extractors.F360.DataSets.F360OALObjectListHeader import F360OALObjectsHeader
from aspe.extractors.F360.DataSets.F360Objects import F360Objects
from aspe.extractors.F360.DataSets.F360RadarDetections import F360RadarDetections
from aspe.extractors.F360.DataSets.F360RadarSensors import F360RadarSensors
from aspe.extractors.F360.DataSets.F360TrackerInfo import F360TrackerInfo
from aspe.extractors.Interfaces.IHost import IHost
from aspe.extractors.Interfaces.IObjects import IObjects
from aspe.extractors.Interfaces.IRadarDetections import IRadarDetections
from aspe.extractors.Interfaces.IRadarSensors import IRadarSensors
from aspe.extractors.ReferenceExtractor.RtRange.DataSets.RtHost import RtHost
from aspe.extractors.ReferenceExtractor.RtRange.DataSets.RtObjects import RtObjects
from aspe.extractors.ReferenceExtractor.SDB.DataSets.SDB_ObjectList import SDB_ObjectList

base_data_sets = {
    'host': IHost(),
    'objects': IObjects(),
    'radar_detections': IRadarDetections(),
    'radar_sensor': IRadarSensors(),
}

object_evaluation = {
    'reference_objects': PESingleObjects(),
    'estimated_objects': PESingleObjects(),
    'pairs': PEPairedObjects(),
}

f360_core_data_sets = {
    'host': F360Host(),
    'reduced_objects': F360Objects(),
    'internal_objects': F360InternalObjects(),
    'radar_detections': F360RadarDetections(),
    'radar_sensor': F360RadarSensors(),
    'tracker_info': F360TrackerInfo(),
    'oal_object_list_header': F360OALObjectsHeader(),
}

f360_bmw_mid_data_sets = {
    'detection_list': F360BmwDetectionList(),
    'detection_list_header': F360BmwDetectionListHeader(),
    'object_list': F360BmwObjectList(),
    'object_list_header': F360BmwObjectListHeader(),
    'radar_sensor': F360BmwRadarSensors(),
}

rt_range_data_sets = {
    'host': RtHost(),
    'objects': RtObjects(),
}

sdb_data_sets = {
    'objects': SDB_ObjectList(),
}


def data_sets_signal_info_to_excel(ds_dict, path = 'filepath'):
    # Write dictionary of dataframes to separate sheets, within 1 file.

    writer = pd.ExcelWriter(path, engine='xlsxwriter')

    for ds_name, single_ds in ds_dict.items():
        single_ds.signals_info.to_excel(writer, sheet_name=ds_name)

    writer.save()


if __name__ == '__main__':
    try:
        os.mkdir('private')
    except OSError as error:
        print(error)

    data_sets_signal_info_to_excel(base_data_sets, r'private\base_data_sets.xlsx')
    data_sets_signal_info_to_excel(object_evaluation, r'private\object_evaluation.xlsx')
    data_sets_signal_info_to_excel(f360_core_data_sets, r'private\f360_core_data_sets.xlsx')
    data_sets_signal_info_to_excel(f360_bmw_mid_data_sets, r'private\f360_bmw_mid_data_sets.xlsx')
    data_sets_signal_info_to_excel(rt_range_data_sets, r'private\rt_range_data_sets.xlsx')
    data_sets_signal_info_to_excel(sdb_data_sets, r'private\sdb_data_sets.xlsx')

