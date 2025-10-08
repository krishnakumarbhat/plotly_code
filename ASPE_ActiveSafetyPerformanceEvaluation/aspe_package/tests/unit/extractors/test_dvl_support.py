"""
Test file for Dvl Supporting function
"""
import datetime

import pandas as pd

from aspe.utilities.DvlSupport import general_extract_utc


class TestsGeneralExtractUtc:
    """
    Set of tests for general_extract_utc dvl supporting function
    """
    def test_all_data_provided(self):
        """
        From complete example data checking if privides a proper output
        """
        # Setup
        input_df = pd.DataFrame(data=[[20, 19, 7, 22, 0, 15, 0, 0.15]],
                                columns=[
                                    'TimeCentury',
                                    'TimeYear',
                                    'TimeMonth',
                                    'TimeDay',
                                    'TimeHour',
                                    'TimeMinute',
                                    'TimeSecond',
                                    'TimeHSecond',
                                ])
        mapping = {
            'century': ['parsed_data', 'RTRange', 'Host', 'TimeCentury'],
            'year': ['parsed_data', 'RTRange', 'Host', 'TimeYear'],
            'month': ['parsed_data', 'RTRange', 'Host', 'TimeMonth'],
            'day': ['parsed_data', 'RTRange', 'Host', 'TimeDay'],
            'hour': ['parsed_data', 'RTRange', 'Host', 'TimeHour'],
            'minute': ['parsed_data', 'RTRange', 'Host', 'TimeMinute'],
            'seconds': ['parsed_data', 'RTRange', 'Host', 'TimeSecond'],
            'ms': ['parsed_data', 'RTRange', 'Host', 'TimeHSecond'],
        }

        # Expected
        date_time_str = '2019-07-22 00:15:00.15'
        date_time_dt = datetime.datetime.strptime(date_time_str, '%Y-%m-%d %H:%M:%S.%f')
        expected = pd.Series(date_time_dt)

        # Execute
        results = general_extract_utc(input_df, mapping)

        # Verify
        pd.testing.assert_series_equal(expected, results)
