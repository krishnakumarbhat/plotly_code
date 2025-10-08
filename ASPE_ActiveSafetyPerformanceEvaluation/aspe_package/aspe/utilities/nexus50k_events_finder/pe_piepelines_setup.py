from aspe.evaluation.RadarObjectsEvaluation.Association.OneToManyAssociation import OneToManyAssociation
from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification.SegmentationClassifier import SegmentationClassifier
from aspe.evaluation.RadarObjectsEvaluation.Flags import (
    DistanceToCSOriginBelowThrFlag,
    IsMovableFlag,
    IsPerpendicularToHost,
    SignalEqualityFlag,
    SignalInBoundsFlag,
)
from aspe.evaluation.RadarObjectsEvaluation.Flags.IsDetectedByRadar import IsDetectedByRadar
from aspe.evaluation.RadarObjectsEvaluation.Flags.TruePositiveFlag import TruePositiveFlag
from aspe.evaluation.RadarObjectsEvaluation.Flags.UnderSegmentationRatioKPI import UnderSegmentationRatioKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.BinaryClassificationKPI.F1ScoreObjectsKPI import (
    F1ScoreBinaryClassificationKPI,
)
from aspe.evaluation.RadarObjectsEvaluation.KPI.BinaryClassificationKPI.OverSegmentationRatioKPI import (
    OverSegmentationRatioKPI,
)
from aspe.evaluation.RadarObjectsEvaluation.KPI.BinaryClassificationKPI.PPVObjectsKPI import PPVBinaryClassificationKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.BinaryClassificationKPI.TPRObjectsKPI import TPRBinaryClassificationKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.BiasKPI import BiasKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.MaxValueKPI import MaxValueKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.MeanKPI import MeanKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.MinValueKPI import MinValueKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.Quantile99KPI import Quantile99KPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.RMSEKPI import RMSEKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.STDDeviationKPI import STDKPI
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures import (
    DimensionsDeviation,
    IntersectionOverUnionRatio,
    OrientationDeviation,
    PositionDeviationCR2CR,
    SpeedDeviation,
)
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.GroundTruthObjectType import GroundTruthObjectType
from aspe.evaluation.RadarObjectsEvaluation.PairsPreBuilders import PEPairedObjectsPreBuilderGating
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.extractors.Interfaces.Enums.Object import ObjectClass


def get_pe_pipeline_for_event_type(event_type: str, evaluate_internal_objects: bool = False):
    if 'close_trucks' in event_type:
        return get_pe_pipeline_for_close_trucks_events(evaluate_internal_objects)

    elif 'overtaking' in event_type:
        return get_pe_pipeline_for_overtaking_events(evaluate_internal_objects)

    elif 'stop_and_go' in event_type:
        return get_pe_pipeline_for_stop_and_go_events(evaluate_internal_objects)

    elif 'cta' in event_type:
        return get_pe_pipeline_for_cta_events(evaluate_internal_objects)

    elif 'close_pedestrians' in event_type:
        return get_pe_pipeline_for_close_pedestrians(evaluate_internal_objects)

    else:
        pe_pipeline = get_default_pipeline_for_50k_eval()
        pe_pipeline.evaluate_internal_objects = evaluate_internal_objects
        return pe_pipeline


def get_default_pipeline_for_50k_eval() -> PEPipeline:
    """
    Return PEPipeline with default configurate which is suitable for 50k data evaluation.
    :return: configured PEPipeline
    """
    # KPIs settings
    features_to_calc = [
        PositionDeviationCR2CR(),
        SpeedDeviation(),
        OrientationDeviation(),
        DimensionsDeviation(),
        IntersectionOverUnionRatio(),
    ]

    kpis_filters = [
        (BiasKPI(), TruePositiveFlag()),
        (STDKPI(), TruePositiveFlag()),
        (RMSEKPI(), TruePositiveFlag()),
        (MaxValueKPI(), TruePositiveFlag()),
        (Quantile99KPI(), TruePositiveFlag()),
    ]

    iou_filters = [
        (MeanKPI(), TruePositiveFlag()),
        (STDKPI(), TruePositiveFlag()),
        (RMSEKPI(), TruePositiveFlag()),
        (MaxValueKPI(), TruePositiveFlag()),
        (MinValueKPI(), TruePositiveFlag()),
    ]

    kpis_to_calc = {
        'dev_position_x': kpis_filters,
        'dev_position_y': kpis_filters,
        'dev_speed': kpis_filters,
        'dev_orientation': kpis_filters,
        'dev_bounding_box_dimensions_x': kpis_filters,
        'dev_bounding_box_dimensions_y': kpis_filters,
        'intersection_over_union': iou_filters,
    }

    detection_rate_kpis = [
        TPRBinaryClassificationKPI(),
        PPVBinaryClassificationKPI(),
        F1ScoreBinaryClassificationKPI(),
        OverSegmentationRatioKPI(),
        UnderSegmentationRatioKPI(),
    ]

    # RELEVANCY SETTINGS
    relevancy_flags_f360 = [IsMovableFlag()]
    relevancy_flags_auto_gt = [DistanceToCSOriginBelowThrFlag(thr=90.0), IsDetectedByRadar()]

    pe_pipeline = PEPipeline(
        pairs_features_list=features_to_calc,
        pairs_features_kpis=kpis_to_calc,
        objects_features_kpis=detection_rate_kpis,
        evaluate_internal_objects=True,
        relevancy_estimated_objects=relevancy_flags_f360,
        relevancy_reference_objects=relevancy_flags_auto_gt,
        binary_classification=SegmentationClassifier(),
        association=OneToManyAssociation(bbox_intersection_condition=True),
        pairs_builder=PEPairedObjectsPreBuilderGating(distance_threshold=3,
                                                      velocity_difference_threshold=3,
                                                      orientation_difference_threshold=45))
    return pe_pipeline


def get_pe_pipeline_for_close_trucks_events(evaluate_internal_objects: bool = False):
    pe_pipeline = get_default_pipeline_for_50k_eval()

    pe_pipeline.relevancy_ref_objs = [SignalEqualityFlag('object_class', ObjectClass.TRUCK),
                                      DistanceToCSOriginBelowThrFlag(thr=50),
                                      SignalInBoundsFlag('velocity_otg_x', min_value=10)]

    pe_pipeline.relevancy_est_objs = [SignalInBoundsFlag('velocity_otg_x', min_value=10),
                                      DistanceToCSOriginBelowThrFlag(thr=50),
                                      ]

    pe_pipeline.pairs_builder.distance_threshold = 25.0
    pe_pipeline.pairs_builder.velocity_difference_threshold = 5.0
    pe_pipeline.pairs_builder.orientation_difference_threshold = 60.0
    pe_pipeline.evaluate_internal_objects = evaluate_internal_objects

    # KPIs settings
    pe_pipeline.pairs_features_list.append(GroundTruthObjectType())

    kpis_filters = [
        (BiasKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),
        (STDKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),
        (RMSEKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),
        (MaxValueKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),
        (Quantile99KPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),

        (BiasKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
        (STDKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
        (RMSEKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
        (MaxValueKPI(),
         TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
        (Quantile99KPI(),
         TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
    ]

    iou_filters = [
        (MeanKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),
        (STDKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),
        (RMSEKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),
        (MaxValueKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),
        (MinValueKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),

        (MeanKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
        (STDKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
        (RMSEKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
        (MaxValueKPI(),
         TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
        (MinValueKPI(),
         TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
    ]

    kpis_to_calc = {
        'dev_position_x': kpis_filters,
        'dev_position_y': kpis_filters,
        'dev_speed': kpis_filters,
        'dev_orientation': kpis_filters,
        'dev_bounding_box_dimensions_x': kpis_filters,
        'dev_bounding_box_dimensions_y': kpis_filters,
        'intersection_over_union': iou_filters,
    }

    pe_pipeline.kpi_manager_pairs_features.kpis_to_calculate = kpis_to_calc
    return pe_pipeline


def get_pe_pipeline_for_overtaking_events(evaluate_internal_objects: bool = False):
    pe_pipeline = get_default_pipeline_for_50k_eval()
    pe_pipeline.relevancy_ref_objs = [DistanceToCSOriginBelowThrFlag(thr=110),
                                      SignalInBoundsFlag('velocity_rel_x', min_value=5),
                                      SignalInBoundsFlag('velocity_otg_x', min_value=15),
                                      IsDetectedByRadar()]

    pe_pipeline.relevancy_est_objs = [SignalInBoundsFlag('velocity_rel_x', min_value=5),
                                      SignalInBoundsFlag('velocity_otg_x', min_value=15),
                                      DistanceToCSOriginBelowThrFlag(thr=110),
                                      ]
    pe_pipeline.evaluate_internal_objects = evaluate_internal_objects
    pe_pipeline.pairs_builder.distance_threshold = 15.0
    pe_pipeline.pairs_builder.velocity_difference_threshold = 5.0
    pe_pipeline.pairs_builder.orientation_difference_threshold = 60.0
    return pe_pipeline


def get_pe_pipeline_for_stop_and_go_events(evaluate_internal_objects: bool = False):
    pe_pipeline = get_default_pipeline_for_50k_eval()

    pe_pipeline.pairs_builder.distance_threshold = 6.0
    pe_pipeline.pairs_builder.velocity_difference_threshold = 2.0
    pe_pipeline.evaluate_internal_objects = evaluate_internal_objects

    pe_pipeline.relevancy_ref_objs.extend(
        [SignalInBoundsFlag(signal_name='position_x', min_value=-20.0, max_value=20.0, flag_signature='long_close'),
         SignalInBoundsFlag(signal_name='position_y', min_value=-10.0, max_value=10.0, flag_signature='lat_close')])

    pe_pipeline.relevancy_est_objs.extend(
        [SignalInBoundsFlag(signal_name='position_x', min_value=-20.0, max_value=20.0, flag_signature='long_close'),
         SignalInBoundsFlag(signal_name='position_y', min_value=-10.0, max_value=10.0, flag_signature='lat_close')])
    return pe_pipeline


def get_pe_pipeline_for_cta_events(evaluate_internal_objects: bool = False):
    pe_pipeline = get_default_pipeline_for_50k_eval()

    pe_pipeline.relevancy_ref_objs = [IsPerpendicularToHost(),
                                      SignalInBoundsFlag('position_x', min_value=-60, max_value=60),
                                      SignalInBoundsFlag('position_y', min_value=-100, max_value=100)]

    pe_pipeline.relevancy_est_objs = [IsPerpendicularToHost(),
                                      SignalInBoundsFlag('position_x', min_value=-60, max_value=60),
                                      SignalInBoundsFlag('position_y', min_value=-100, max_value=100)]

    pe_pipeline.pairs_builder.distance_threshold = 6.0
    pe_pipeline.pairs_builder.velocity_difference_threshold = 5.0
    pe_pipeline.evaluate_internal_objects = evaluate_internal_objects

    # KPIs settings
    pe_pipeline.pairs_features_list.append(GroundTruthObjectType())

    kpis_filters = [
        (BiasKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),
        (STDKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),
        (RMSEKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),
        (MaxValueKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),
        (Quantile99KPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),

        (BiasKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
        (STDKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
        (RMSEKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
        (MaxValueKPI(),
         TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
        (Quantile99KPI(),
         TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
    ]

    iou_filters = [
        (MeanKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),
        (STDKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),
        (RMSEKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),
        (MaxValueKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),
        (MinValueKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.CAR, 'object_class_car')),

        (MeanKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
        (STDKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
        (RMSEKPI(), TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
        (MaxValueKPI(),
         TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
        (MinValueKPI(),
         TruePositiveFlag() & SignalEqualityFlag('object_class', ObjectClass.TRUCK, 'object_class_truck')),
    ]

    kpis_to_calc = {
        'dev_position_x': kpis_filters,
        'dev_position_y': kpis_filters,
        'dev_speed': kpis_filters,
        'dev_orientation': kpis_filters,
        'dev_bounding_box_dimensions_x': kpis_filters,
        'dev_bounding_box_dimensions_y': kpis_filters,
        'intersection_over_union': iou_filters,
    }

    pe_pipeline.kpi_manager_pairs_features.kpis_to_calculate = kpis_to_calc
    return pe_pipeline


def get_pe_pipeline_for_close_pedestrians(evaluate_internal_objects: bool = False):
    pe_pipeline = get_default_pipeline_for_50k_eval()
    pe_pipeline.relevancy_ref_objs = [SignalEqualityFlag('object_class', ObjectClass.PEDESTRIAN),
                                      DistanceToCSOriginBelowThrFlag(thr=50)]

    pe_pipeline.pairs_builder.distance_threshold = 3.0
    pe_pipeline.pairs_builder.velocity_difference_threshold = 2.0
    pe_pipeline.evaluate_internal_objects = evaluate_internal_objects
    return pe_pipeline
