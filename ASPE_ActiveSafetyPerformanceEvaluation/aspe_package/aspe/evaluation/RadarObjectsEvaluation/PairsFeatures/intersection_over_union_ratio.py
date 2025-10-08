import pandas as pd
from shapely.geometry import Polygon

from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.IPairsFeature import IPairsFeature
from aspe.utilities.PolygonSupportingFunctions import create_polygons_for_objects

""" 
Used Acronyms: 
    IoU - Intersection Over Union Ratio:
    It calculates overapping area of estimated and reference bboxes in respect to joined bboxed area.
    1 - is perfect match, reference and estimated bboxes have identical parameters.
    0 - is the worsest result, estimated and reference bboxes doesn't have shared area at all.   
"""


class IntersectionOverUnionRatio(IPairsFeature):

    def calculate_IoU(self, bboxes_df: pd.DataFrame) -> pd.DataFrame:
        """
        Calculate Iou from given bboxes.

        :param bboxes_df: Data Frame, which contain information about estimated and reference polygons

        :return: Intersection over Union ratio
        """
        intersection_area = bboxes_df['ground_truth_bbox'].intersection(bboxes_df['estimated_bbox']).area
        sum_of_bboxes_area = bboxes_df['ground_truth_bbox'].area + bboxes_df['estimated_bbox'].area
        return intersection_area / (sum_of_bboxes_area - intersection_area)

    def calculate_intersection_over_union(self, ground_truth_bbox: pd.DataFrame, estimated_bbox: pd.DataFrame):
        """
        Calculate Intersection over Union ratio for two polygons.
        :param: Ground_truth_bbox: bbox of groud truth object passed as Polygon.
        :param: Estimated_bbox: estimated bbox passed as Polygon.
        :return: IoU ratio. Overlapping area to area of union.
        """
        IoU_df = pd.DataFrame({'ground_truth_bbox': ground_truth_bbox, 'estimated_bbox': estimated_bbox})
        return IoU_df.apply(lambda x: self.calculate_IoU(x), axis=1, result_type='reduce')

    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        polygon_of_estimated_data = create_polygons_for_objects(estimated_data)
        polygon_of_reference_data = create_polygons_for_objects(reference_data)
        IoU = self.calculate_intersection_over_union(polygon_of_reference_data, polygon_of_estimated_data)
        return pd.DataFrame({'intersection_over_union': IoU})

