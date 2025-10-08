# coding=utf-8
"""
Main Builder interface. All project builders shall inherit after IBuilder
"""
from abc import ABC, abstractmethod
from typing import Union
from warnings import warn

import numpy as np

from aspe.extractors.Interfaces import IDataSet
from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription


class IBuilder(ABC):
    """
    Builder interface. This class should be overwritten by any project builder
    :parsed_data: data from parsers
    """

    def __init__(self, parsed_data, **kwargs):
        self._parsed_data = parsed_data
        self.data_set: IDataSet = None

    @abstractmethod
    def build(self):
        """
        Main builder method for data set creation
        :return: filled (extracted) data set
        """
        return self.data_set

    def _extract_mappable_signals(self):
        """
        Extract signals which can be strictly mapped using Mapper object
        :return:
        """
        for aspe_signature, raw_signature in self.signal_mapper.items():
            try:
                self._map_single_signal(aspe_signature, raw_signature)
            except KeyError:
                self._warn_about_missing_signals(aspe_signature, raw_signature)

    def _map_single_signal(self, aspe_signature: Union[SignalDescription, str], raw_signature: str):
        if type(aspe_signature) is SignalDescription:
            self.data_set.signals[aspe_signature.signature] = self._raw_signals[raw_signature].astype(
                aspe_signature.dtype)
        else:
            self.data_set.signals[aspe_signature] = self._raw_signals[raw_signature]

    def _warn_about_missing_signals(self, aspe_signature: str, missing_f360_signature: str):
        warn(
            f'{self.data_set.get_base_name()} extraction warning! There is no signal {missing_f360_signature} inside '
            f'raw_signals DataFrame. Signal {aspe_signature} will be not filled.')

    def _convert_dtypes(self):
        """
        Convert signals which are int / float to np.int32 / np.float32
        :return:
        """
        for signal in self.data_set.signals.columns:
            try:
                signal_description = self.data_set.signals_info.loc[
                    self.data_set.signals_info['signature'].str.contains(signal, case=False)]
                if not signal_description.empty:
                    dtype = signal_description['dtype'].iloc[0]
                    self.data_set.signals[signal] = self.data_set.signals[signal].astype(dtype)
            except KeyError:
                pass
            except ValueError:
                pass
