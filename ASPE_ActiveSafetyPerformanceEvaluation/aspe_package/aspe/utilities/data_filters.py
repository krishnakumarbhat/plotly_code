from typing import List

import pandas as pd


def slice_df_based_on_other(left_df: pd.DataFrame, right_df: pd.DataFrame, left_on: List, right_on: List) \
        -> pd.DataFrame:
    """
    Wrapper for data-frame merge functionality when the need is to take signals from one data-frame based on keys from
    other data-frame
    :param left_df: DataFrame which should be be sliced
    :param right_df: DataFrame from which keys are used for slicing
    :param left_on: Keys (columns) from left DataFrame
    :param right_on: Keys (columns) from right DataFrame
    :return: sliced left DataFrame
    """

    if len(left_on) != len(right_on):
        raise ValueError(f'Left: {left_on} and Right: {right_on} keys need to have the same length')
    right_df_index = right_df.loc[:, right_on]
    right_df_index.columns = left_on
    right_m_index = pd.MultiIndex.from_frame(right_df_index)
    left_df_copy = left_df.copy()
    left_df_copy.index = pd.MultiIndex.from_frame(left_df.loc[:, left_on])
    sliced_left_df = left_df_copy.loc[right_m_index]

    # This line may cause error when there are duplicates in left_df[left_on]. However this line guarantee that indexes
    # will match after slice operation
    sliced_left_df.index = right_df.index
    return sliced_left_df
