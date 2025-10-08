# coding=utf-8
"""
F360 MUDP execution time info builder (version 11)
"""
from warnings import warn

from aspe.extractors.F360.Mudp.Builders.ExecutionTimeInfo.IF360MudpExecutionTimeInfoBuilder import (
    IF360MudpExecutionTimeInfoBuilder,
)
from aspe.utilities.SupportingFunctions import get_nest_till_array_generator, is_1d_array, values_are_same


class F360MudpExecutionTimeInfoBuilderV11(IF360MudpExecutionTimeInfoBuilder):
    """
    supports objects extraction for stream definitions:
    ch: 9 - strdef_src035_str009_ver011.txt
    """
    required_stream_definitions = {'strdef_src035_str009_ver011'}
    main_stream = 9
    signal_mapper = {
        # ASPE signature    F360 signature
        'scan_index':       'tracker_index',
        'timestamp':        'timestamp',
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data, f_extract_raw_signals=f_extract_raw_signals)

    def build(self):
        """
        Main extraction function.
        :return:
        """
        self._extract_raw_signals()
        self._extract_mappable_signals()
        return self.data_set

    def _extract_raw_signals(self):
        """
        Extracts all raw signals from parsed data into raw signals dataframe.
        :return:
        """
        parsed_raw_signals_gen = get_nest_till_array_generator(self._parsed_data['parsed_data'][self.main_stream])
        for name, values in parsed_raw_signals_gen:
            if is_1d_array(values) and not values_are_same(values):
                self._raw_signals[name] = values

        self._raw_signals.sort_index(axis=1, inplace=True)

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_mappable_signals(self):
        """
        Extract signals which can be strictly mapped using Mapper object
        :return:
        """
        for aspe_signature, raw_signature in self.signal_mapper.items():
            try:
                self.data_set.signals[aspe_signature] = self._raw_signals[raw_signature]
            except KeyError:
                warn(f'Signal {raw_signature} not found, {aspe_signature} will not been extracted')
