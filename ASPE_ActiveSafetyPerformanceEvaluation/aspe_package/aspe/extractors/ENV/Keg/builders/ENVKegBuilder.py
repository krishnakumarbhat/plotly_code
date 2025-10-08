"""
General builder for Bin data sets.
"""
from abc import abstractmethod
from typing import Union

import pandas as pd

from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription


class ENVKegBuilder(IBuilder):
    """
    Builder interface. This class should be overwritten by any ENV builder
    :parsed_data: data from parsers
    """
    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data)
        self._f_extract_raw_signals = f_extract_raw_signals
        self._raw_signals = pd.DataFrame()

    @abstractmethod
    def build(self):
        """
        Extracts signals and properties from parsed data into dataset.
        Dataset-specific builders (e.g IF360MudpHostBuilder, IF360MudpObjectsBuilder) should override this method.
        :return: filled (extracted) data set
        """
        return super().build()

    def _extract_mappable_signals(self):
        """
        Extract signals which can be strictly mapped using Mapper object
        """
        for aspe_signature, raw_signature in self.signal_mapper.items():
            try:
                if isinstance(raw_signature, str):
                    self._map_single_signal(aspe_signature, raw_signature)
                elif isinstance(raw_signature, (list, tuple, set)):
                    self._map_single_signal_using_list_of_possible_signatures(aspe_signature, raw_signature)
            except KeyError:
                self._warn_about_missing_signals(aspe_signature, raw_signature)

    def _map_single_signal_using_list_of_possible_signatures(self, aspe_signature: Union[SignalDescription, str],
                                                             list_of_f360_signal_signatures: Union[list, tuple, set]):
        signal_found = False
        for f360_signature in list_of_f360_signal_signatures:
            if f360_signature in self._raw_signals:
                self._map_single_signal(aspe_signature, f360_signature)
                signal_found = True
                break
        if not signal_found:
            self._warn_about_missing_signals(aspe_signature, list_of_f360_signal_signatures)