# coding=utf-8
"""
General builder for Xtrk data sets.
"""
from typing import Union

import pandas as pd

from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription
from aspe.utilities.SupportingFunctions import get_nested_array


class F360XtrkBuilder(IBuilder):
    """
    Builder interface. This class should be overwritten by any project builder
    :parsed_data: data from parsers
    """
    def __init__(self, parsed_data, **kwargs):
        super().__init__(parsed_data, **kwargs)

    def _extract_mappable_signals(self):
        """
        Extract signals which can be strictly mapped using Mapper object
        :return:
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
        """
        Fill internal data set structure using signal detected using list of possible signal signatures
        :param aspe_signature: signature of the signal in aspe
        :param list_of_f360_signal_signatures: possible names of corresponding signal in f360 parsed data
        """
        signal_found = False
        for f360_signature in list_of_f360_signal_signatures:
            if f360_signature in self._raw_signals:
                self._map_single_signal(aspe_signature, f360_signature)
                signal_found = True
                break
        if not signal_found:
            self._warn_about_missing_signals(aspe_signature, list_of_f360_signal_signatures)

    def _get_nested_array_using_possible_signatures(self, data: pd.DataFrame, possible_keys: Union[list, tuple, set]):
        """
        Get signal detected using list of possible signal signatures
        :param data: data structure to be searched
        :param possible_keys: either list representing nested structure of keys or list of lists representing possible
                              alternative names of nested structure
        :return extracted signal value
        """
        # Convert list of keys to list of lists for unified handling
        if all([isinstance(key, str) for key in possible_keys]):
            possible_keys = [possible_keys]

        signal_value = None
        for key in possible_keys:
            signal_value = get_nested_array(data, key)
            if signal_value is not None:
                break
        if signal_value is None:
            self._warn_about_missing_signals('', possible_keys)

        return signal_value

    @staticmethod
    def _get_auxiliary_raw_signals_mapping(auxiliary_signals: dict):
        """
        Convert signal names mapping from absolute names as in parsed data to relative as in raw signals by removing
        the first part of nested signal name
        :param auxiliary_signals: dictionary representing absolute mapping between aspe signals and possible signatures
                                  of signals in f360 parsed data
        :return dictionary representing relative mapping between aspe signals and f360 raw data
        """
        auxiliary_raw_signals = {}
        for key, nested_name_list in auxiliary_signals.items():
            # Convert list to list of lists for unified handling
            if all([isinstance(elem, str) for elem in nested_name_list]):
                nested_name_list = [nested_name_list]

            auxiliary_raw_signals[key] = [elem[1:] if len(elem) > 1 else elem for elem in nested_name_list]

        return auxiliary_raw_signals
