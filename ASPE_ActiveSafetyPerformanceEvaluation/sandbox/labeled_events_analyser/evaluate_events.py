import copy
import glob

import math
import numpy as np
import pandas as pd
from matplotlib.path import Path

from AptivDataExtractors.F360.ExcelExtr.F360ExcelExtractor import F360ExcelExtractor
from AptivDataExtractors.F360.Mudp.F360MUDPExtractor import F360MUDPExtractor
from AptivDataExtractors.utilities.MathFunctions import lcs2vcs
from AptivDataExtractors.utilities.SupportingFunctions import save_to_pkl
from AptivDataParser import MudpParser
from AptivPerformanceEvaluation.RadarObjectsEvaluation.Association.RelEstToRefNN import RelEstToRefNN
from AptivPerformanceEvaluation.RadarObjectsEvaluation.DataStructures.PEPairedObjectsPreBuilderGating import \
    PEPairedObjectsPreBuilderGating
from AptivPerformanceEvaluation.RadarObjectsEvaluation.Flags.EventSpeedOrientMovableFlag import \
    EventSpeedOrientMovableFlag
from AptivPerformanceEvaluation.RadarObjectsEvaluation.Flags.EventInitializationSIFlag import \
    EventInitializationSIFlag
from AptivPerformanceEvaluation.RadarObjectsEvaluation.Metrics.OrientationSpeedNormErrorSum import \
    OrientationSpeedNormErrorSum
from AptivPerformanceEvaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from sandbox.labeled_events_analyser.event_kpis import calc_event_kpis

extension = 'mudp'
mudp_parser_config_path = r'\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\Extractors_refactoring\parsers_config\mudp_data_parser_config.json'
mudp_stream_def_path = r'\\10.224.186.68\AD-Shared\ASPE\configurations\F360\MUDP_Stream_Definitions\stream_definitions'
logs_main_path = r'C:\Users\mj2snl\Documents\logs\dft-44'
excel_path = r"C:\Users\mj2snl\Downloads\AIT-00000_log_pointers_golden_data_set_initialization.xlsx"

resim_tag = 'rRf360t4150309v205p50'  # '' means it will be original log

orientation = 'bounding_box_orientation'
speed = 'speed'


def handle_est_obj_by_events_si_roi_speed_orientation(est_object_df, event):
    # si handle

    start_sidexes = event['Tracker Idx Start']
    end_sidexes = event['Tracker Idx End']

    objects_event_si_mask = est_object_df.scan_index.between(start_sidexes, end_sidexes)
    est_object_df = est_object_df[objects_event_si_mask]

    # roi handle
    roi_center_x = event['Center long VCS [m]']
    roi_center_y = event['Center lat VCS [m]']
    roi_length = event['Length [m]']
    roi_width = event['Width [m]']
    roi_rotation = math.radians(event['Oritnation VCS [deg]'])  # typo from excel file

    roi_x_pos = np.array([roi_length, roi_length, -roi_length, -roi_length]) * 0.5
    roi_y_pos = np.array([-roi_width, roi_width, roi_width, -roi_width]) * 0.5
    roi_x_pos_rot, roi_y_pos_rot = lcs2vcs(roi_x_pos, roi_y_pos, roi_center_x, roi_center_y, roi_rotation)

    poly = Path([(roi_y_pos_rot[0], roi_x_pos_rot[0]), (roi_y_pos_rot[1], roi_x_pos_rot[1]),
                 (roi_y_pos_rot[2], roi_x_pos_rot[2]), (roi_y_pos_rot[3], roi_x_pos_rot[3]),
                 (roi_y_pos_rot[0], roi_x_pos_rot[0])])

    objects_roi_mask = poly.contains_points(est_object_df[['position_y', 'position_x']])

    est_object_df = est_object_df[objects_roi_mask]
    return est_object_df


log_pointers = pd.read_excel(excel_path, sheet_name='Log pointers', header=1)
log_pointers_grpd = log_pointers.groupby("Log base name")

log_pointers_name_list = list(log_pointers["Log base name"].dropna().unique())

logs_path = {}
log_list = sorted(glob.glob(f'{logs_main_path}\\**/*{resim_tag}*.{extension}', recursive=True))
for log_path in log_list:
    for log_pointer_name in log_pointers_name_list:
        if log_pointer_name in log_path:
            logs_path[log_pointer_name] = log_path

aggr_df = pd.DataFrame()

for log_name, log_events_df in log_pointers_grpd:
    try:
        mudp_log_path = logs_path[log_name]
    except KeyError:
        continue

    save_path = mudp_log_path.replace('.mudp', '_MUDP_Extracted.pickle')

    try:
        f360_extracted_data = load_from_pkl(path_to_pickled_data)
    except NameError:
        # Parsing
        parser = MudpParser.MudpHandler(mudp_log_path, mudp_parser_config_path, mudp_stream_def_path)
        parsed_data = parser.decode()
        parser.save_to_pickle()

        # Extracting
        f360_extractor_object = F360MUDPExtractor()
        f360_extracted_data = f360_extractor_object.extract_data(parsed_data)
        save_to_pkl(f360_extracted_data, save_path)

    event_reference_object = F360ExcelExtractor()
    event_reference_data = event_reference_object.extract_data(log_events_df)  # add all si for event

    # copy for events handling
    extracted_est_data = copy.deepcopy(f360_extracted_data)
    extracted_ref_data = copy.deepcopy(event_reference_data)

    for event_idx, event in log_events_df.iterrows():  # implement it this way
        # one event handle
        # each event is filtering for si, roi, orientation and speed

        ref_objects = extracted_ref_data.objects.signals[extracted_ref_data.objects.signals['slot_id'] == event_idx]
        est_objects = extracted_est_data.objects.signals
        est_objects = handle_est_obj_by_events_si_roi_speed_orientation(est_objects, event)

        f360_extracted_data.objects.signals = est_objects
        event_reference_data.objects.signals = ref_objects

        # - TODO: time to met error threshold
        pe = PEPipeline(pairs_builder=PEPairedObjectsPreBuilderGating(f_distance_gating=False),
                        relevancy_estimated_objects=EventSpeedOrientMovableFlag(events_data=event),
                        relevancy_reference_objects=EventInitializationSIFlag(events_data=event),
                        association_distance=OrientationSpeedNormErrorSum(events_data=event),
                        association=RelEstToRefNN(assoc_distance_threshold=1.0))
        out = pe.evaluate(estimated_data=f360_extracted_data, reference_data=event_reference_data)

        event_kpis_df = calc_event_kpis(out, log_name)

        aggr_df = pd.concat([aggr_df, event_kpis_df], sort=False)

aggr_df.to_excel(logs_main_path + '/aggr_init_results.xlsx')
