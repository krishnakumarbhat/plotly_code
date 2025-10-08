import pandas as pd
import pytest

from aspe.utilities.data_filters import slice_df_based_on_other


class TestSliceDFBasedOnOther:
    def test_empty_left_df(self):
        left_df = pd.DataFrame([], columns=['a', 'b', 'c'])

        right_df = pd.DataFrame([[1, 12, 13],
                                [4, 15, 16]], columns=['x', 'y', 'z'])

        with pytest.raises(KeyError):
            slice_df_based_on_other(left_df, right_df, ['a'], ['x'])

    def test_empty_right_df(self):
        left_df = pd.DataFrame([[1, 2, 3],
                                [4, 5, 6],
                                [7, 8, 9],
                                [10, 11, 12]], columns=['a', 'b', 'c'])

        right_df = pd.DataFrame([], columns=['x', 'y', 'z'])

        sliced_df = slice_df_based_on_other(left_df, right_df, ['a'], ['x'])

        expected_columns = ['a', 'b', 'c']
        assert sliced_df.empty
        assert sliced_df.columns.all(expected_columns)

    def test_simple_df(self):
        left_df = pd.DataFrame([[1, 2, 3],
                                [4, 5, 6],
                                [7, 8, 9],
                                [10, 11, 12]], columns=['a', 'b', 'c'])

        right_df = pd.DataFrame([[1, 12, 13],
                                [4, 15, 16]], columns=['x', 'y', 'z'])

        sliced_df = slice_df_based_on_other(left_df, right_df, ['a'], ['x'])

        expected_df = pd.DataFrame([[1, 2, 3],
                                   [4, 5, 6]], columns=['a', 'b', 'c'])
        pd.testing.assert_frame_equal(sliced_df, expected_df)

    def test_simple_df_missing_key(self):
        left_df = pd.DataFrame([[1, 2, 3],
                                [7, 8, 9],
                                [10, 11, 12]], columns=['a', 'b', 'c'])

        right_df = pd.DataFrame([[1, 12, 13],
                                [4, 15, 16]], columns=['x', 'y', 'z'])

        with pytest.raises(KeyError):
            slice_df_based_on_other(left_df, right_df, ['a'], ['x'])

    def test_duplication_right_df(self):
        left_df = pd.DataFrame([[1, 2, 3],
                                [4, 5, 6],
                                [7, 8, 9],
                                [10, 11, 12]], columns=['a', 'b', 'c'])

        right_df = pd.DataFrame([[1, 2, 13],
                                 [4, 15, 16],
                                 [4, 15, 16]], columns=['x', 'y', 'z'])

        sliced_df = slice_df_based_on_other(left_df, right_df, ['a'], ['x', ])

        expected_df = pd.DataFrame([[1, 2, 3],
                                    [4, 5, 6],
                                    [4, 5, 6]], columns=['a', 'b', 'c'])

        pd.testing.assert_frame_equal(sliced_df, expected_df)

    def test_duplication_right_df_with_different_order(self):
        left_df = pd.DataFrame([[1, 2, 3],
                                [4, 5, 6],
                                [7, 8, 9],
                                [10, 11, 12]], columns=['a', 'b', 'c'])

        right_df = pd.DataFrame([[1, 2, 13],
                                 [4, 15, 16],
                                 [4, 15, 11],
                                 [1, 2, 13]], columns=['x', 'y', 'z'])

        sliced_df = slice_df_based_on_other(left_df, right_df, ['a'], ['x', ])

        expected_df = pd.DataFrame([[1, 2, 3],
                                    [4, 5, 6],
                                    [4, 5, 6],
                                    [1, 2, 3]], columns=['a', 'b', 'c'])

        pd.testing.assert_frame_equal(sliced_df, expected_df)

    def test_duplication_left_df(self):
        left_df = pd.DataFrame([[1, 2, 3],
                                [4, 5, 6],
                                [4, 15, 16],
                                [7, 8, 9],
                                [10, 11, 12]], columns=['a', 'b', 'c'])

        right_df = pd.DataFrame([[1, 2, 13],
                                [4, 15, 16]], columns=['x', 'y', 'z'])

        with pytest.raises(ValueError):
            slice_df_based_on_other(left_df, right_df, ['a'], ['x'])

    def test_duplication_left_df_two_keys(self):
        left_df = pd.DataFrame([[1, 2, 3],
                                [4, 5, 6],
                                [4, 15, 16],
                                [7, 8, 9],
                                [10, 11, 12]], columns=['a', 'b', 'c'])

        right_df = pd.DataFrame([[1, 2, 13],
                                [4, 15, 16]], columns=['x', 'y', 'z'])

        sliced_df = slice_df_based_on_other(left_df, right_df, ['a', 'b'], ['x', 'y'])

        expected_df = pd.DataFrame([[1, 2, 3],
                                    [4, 15, 16]], columns=['a', 'b', 'c'])
        pd.testing.assert_frame_equal(sliced_df, expected_df)

    def test_duplication_df_right_with_different_order(self):
        left_df = pd.DataFrame([[1, 2, 3],
                                [4, 5, 6],
                                [4, 15, 16],
                                [7, 8, 9],
                                [10, 11, 12]], columns=['a', 'b', 'c'])

        right_df = pd.DataFrame([[1, 2, 13],
                                 [10, 11, 3],
                                 [4, 15, 1],
                                 [4, 5, 8]], columns=['x', 'y', 'z'])

        sliced_df = slice_df_based_on_other(left_df, right_df, ['a', 'b'], ['x', 'y'])

        expected_df = pd.DataFrame([[1, 2, 3],
                                    [10, 11, 12],
                                    [4, 15, 16],
                                    [4, 5, 6]], columns=['a', 'b', 'c'])
        pd.testing.assert_frame_equal(sliced_df, expected_df)
