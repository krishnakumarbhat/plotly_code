from typing import List

import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags.IFlag import IFlag
from aspe.evaluation.RadarObjectsEvaluation.Flags.SignalInBoundsFlag import SignalInBoundsFlag


class SignalEnvelopeInBoundsFlag(IFlag):
    def __init__(self, signal_name, min_value=-np.inf, max_value=np.inf, flag_signature='signal_envelope_in_bounds',
                 env_raise_time=0.1, env_decay_time=15.0, columns_to_groupby=('unique_id',), *args, **kwargs):
        """ Class for configurable flag calculation: check if signal envelope is within given bounds. First step is
        calculating signal envelope - envelope parameters are env_raise_time and env_decay_time. Then on signal envelope
        SignalInBoundsFlag flag is used.
        :param signal_name: Name of signal (column) in data frame
        :param min_value: minimum allowed value
        :param max_value: maximum allowed value
        :param env_raise_time: envelope raise/attack time [s]
        :param env_decay_time: envelope decay/release time [s]
        :param columns_to_groupby: use this column/columns to calculate flag on grouped dataframe records, by default
        only 'unique_id' column is used. If columns are missing, flag will be simply calculated on not-grouped dataframe
        :param kwargs:
        """
        super().__init__(flag_signature, *args, **kwargs)
        self.signal_name = signal_name
        self.min_value = min_value
        self.max_value = max_value
        if env_raise_time <= 0.0 or env_decay_time <= 0.0:
            raise AttributeError('env_raise_time and env_decay_time must be non-zero, positive')
        self.envelope_raise_time = env_raise_time
        self.envelope_decay_time = env_decay_time
        self.in_bounds_flag = SignalInBoundsFlag(signal_name, min_value, max_value, 'signal_in_bounds')
        self.columns_to_groupby = list(columns_to_groupby)

    def calc_flag(self, data_frame: pd.DataFrame, *args, **kwargs) -> pd.Series:
        """
        Calculate signal envelope and then pass it to SignalInBoundsFlag.calc_flag function
        and return it's output.
        :param data_frame: DataFrame for which flag should be calculated
        :type data_frame: pandas.DataFrame

        :param args:
        :param kwargs:
        :return: pandas.Series: - series with the same length as data frame with flag (boolean)
        """
        if len(data_frame) > 1:
            if set(self.columns_to_groupby) in set(data_frame.columns):
                f_valid = self._calc_flag_with_groupby(data_frame, self.columns_to_groupby)
            else:
                envelope = self._get_envelope(data_frame)
                f_valid = self.in_bounds_flag.calc_flag(pd.DataFrame({self.signal_name: envelope}))
        else:
            # simple case when only single record is passed - use only in bounds flag
            f_valid = self.in_bounds_flag.calc_flag(data_frame)
        return f_valid

    def _calc_flag_with_groupby(self, data_frame: pd.DataFrame, columns_to_groupby: List[str]):
        f_valid = pd.Series(data=[False] * len(data_frame), dtype=bool, index=data_frame.index)
        for unique_id, group in data_frame.groupby(by=columns_to_groupby):
            envelope = self._get_envelope(group)
            f_valid.loc[group.index] = self.in_bounds_flag.calc_flag(pd.DataFrame({self.signal_name: envelope}))
        return f_valid

    def _get_envelope(self, data_frame: pd.DataFrame):
        timestamp = data_frame['timestamp'].to_numpy()
        signal_abs = np.abs(data_frame[self.signal_name].to_numpy())

        mean_dt = np.median(np.diff(timestamp))
        raise_coef = np.power(0.01, mean_dt / self.envelope_raise_time)
        decay_coef = np.power(0.01, mean_dt / self.envelope_decay_time)

        envelope = np.zeros(len(signal_abs))
        env_curr_state = 0.0
        for index, sample in enumerate(signal_abs):
            sample_diff = env_curr_state - sample
            if sample_diff < 0.0:
                env_curr_state = sample + sample_diff * raise_coef
            else:
                env_curr_state = sample + sample_diff * decay_coef
            envelope[index] = env_curr_state
        return envelope


if __name__ == "__main__":
    import matplotlib.pyplot as plt

    from aspe.utilities.SupportingFunctions import load_from_pkl

    ts = np.linspace(0, 10, 1000)
    signal = np.zeros(len(ts))
    signal[(3.0 < ts) & (ts < 5.0)] = 1.0
    df = pd.DataFrame({'timestamp': ts, 'signal': signal})

    flag = SignalEnvelopeInBoundsFlag('signal', env_raise_time=0.5, env_decay_time=2.0)
    envelope = flag._get_envelope(df)
    plt.plot(ts, signal, label='signal')
    plt.plot(ts, envelope, label='envelope')
    plt.legend()
    plt.title(f"Envelope raise time {flag.envelope_raise_time} [s], decay time: {flag.envelope_decay_time} [s]")
    plt.xlabel('time [s]')
    plt.grid()

    # REAL DATA EXAMPLE
    pe_out = load_from_pkl(r"C:\logs\pe_outs\DS_24_rRf360t5040309v205p50_3p04_pe_output.pickle")
    ref = pe_out.extracted_reference_objects.signals
    ref.sort_values(by=['unique_id', 'timestamp'], inplace=True)
    ref = ref.groupby(by='log_index').get_group(3)

    signal_name = 'yaw_rate'
    flag = SignalEnvelopeInBoundsFlag(signal_name, min_value=0.02)
    envelope = flag._get_envelope(ref)
    is_valid = flag.calc_flag(ref)

    f, axes = plt.subplots(nrows=2)
    axes[0].plot(ref.timestamp, ref.loc[:, signal_name], label='signal')
    axes[0].plot(ref.timestamp, envelope, label='envelope')
    axes[0].hlines(flag.min_value, xmin=ref.timestamp.to_numpy()[0], xmax=ref.timestamp.to_numpy()[-1],
                   label='maneuvering threshold', ls='--', )
    axes[1].plot(ref.timestamp, is_valid, label='is maneuvering')
    for a in axes:
        a.grid()
        a.legend()
    axes[0].set_ylabel(signal_name)
    axes[1].set_ylabel('is_maneuvering')
    axes[1].set_xlabel('time [s]')
