import pandas as pd

from aspe.evaluation.RadarSGEvaluation.DataStructures.SGSegments import SGSegments
from aspe.extractors.ENV.DataSets.ENVExtractedData import ENVExtractedData

REQUIRED_COLUMNS_LIST = ['scan_index',
                         'timestamp',
                         'unique_id',
                         'position_x',
                         'position_y']


def build_segments(SG_data: ENVExtractedData) -> SGSegments:
    """
    Copying data structures and creating segments dataframe for further processing.

    :param SG_data: extracted Stationary Geometries
    :type SG_data: ENVExtractedData

    :raises:
        ValueError:  Error is thrown when stationary geometries data are missing.
        AttributeError: Error is thrown when required column cannot be found in input data

    :rtype: SGSegments
    """

    _validate_input(SG_data)

    raw_segments = SG_data.stationary_geometries.signals[REQUIRED_COLUMNS_LIST].copy()

    raw_segments['end_position_x'] = raw_segments['position_x'].shift(-1)
    raw_segments['end_position_y'] = raw_segments['position_y'].shift(-1)

    _drop_last_vertex(raw_segments)

    raw_segments.rename(
        columns={'unique_id': 'contour_unique_id',
                 'position_x': 'start_position_x',
                 'position_y': 'start_position_y'},
        inplace=True)

    raw_segments.reset_index(inplace=True)
    raw_segments['unique_id'] = raw_segments.index

    segments = SGSegments()
    segments.insert_segments(raw_segments)

    return segments


def _validate_input(SG_data: ENVExtractedData):
    if SG_data.stationary_geometries is None:
        raise ValueError('Missing Stationary Geometries Data!')

    if not set(REQUIRED_COLUMNS_LIST).issubset(SG_data.stationary_geometries.signals):
        raise AttributeError('Missing columns inside extracted dataframe!')


def _drop_last_vertex(df: pd.DataFrame) -> pd.DataFrame:
    return df.drop(df.groupby(['unique_id', 'scan_index']).tail(1).index, axis=0, inplace=True)
