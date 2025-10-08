import pandas as pd
from shapely.geometry import Polygon

from .MathFunctions import calc_position_in_bounding_box


def create_polygon(corner_df: pd.DataFrame) -> Polygon:
    """
    Create Polygon from given bbox.

    :param corner_df: Data Frame, which contain information about positions of bbox Corners

    :return: Polygon from passed bbox.
    """
    obj_polygon = Polygon([(corner_df['front_left_x'], corner_df['front_left_y']),
                           (corner_df['front_right_x'], corner_df['front_right_y']),
                           (corner_df['rear_right_x'], corner_df['rear_right_y']),
                           (corner_df['rear_left_x'], corner_df['rear_left_y'])])
    return obj_polygon

def create_polygons_for_objects(object_df: pd.DataFrame) -> pd.DataFrame:
    """
    Create DataFrame, which contain position on bbox corner and Polygon created from those corners.

    :param corner_df: Data Frame, which contain information  bbox center position, orientation and dimensions.

    :return: DataFrame, which contains information about bbox corner position and created Polygon.
    """
    corner_df = pd.DataFrame()
    corner_df['front_left_x'], corner_df['front_left_y'] = calc_position_in_bounding_box(
        object_df.center_x, object_df.center_y,
        object_df.bounding_box_dimensions_x,
        object_df.bounding_box_dimensions_y,
        object_df.bounding_box_orientation,
        0.5, 0.5, 1.0, 0.0)
    corner_df['front_right_x'], corner_df['front_right_y'] = calc_position_in_bounding_box(
        object_df.center_x, object_df.center_y,
        object_df.bounding_box_dimensions_x,
        object_df.bounding_box_dimensions_y,
        object_df.bounding_box_orientation,
        0.5, 0.5, 1.0, 1.0)
    corner_df['rear_right_x'], corner_df['rear_right_y'] = calc_position_in_bounding_box(
        object_df.center_x, object_df.center_y,
        object_df.bounding_box_dimensions_x,
        object_df.bounding_box_dimensions_y,
        object_df.bounding_box_orientation,
        0.5, 0.5, 0.0, 1.0)
    corner_df['rear_left_x'], corner_df['rear_left_y'] = calc_position_in_bounding_box(
        object_df.center_x, object_df.center_y,
        object_df.bounding_box_dimensions_x,
        object_df.bounding_box_dimensions_y,
        object_df.bounding_box_orientation,
        0.5, 0.5, 0.0, 0.0)
    corner_df['bbox_polygon'] = corner_df.apply(lambda x: create_polygon(x), axis=1, result_type='reduce')
    return corner_df['bbox_polygon']


