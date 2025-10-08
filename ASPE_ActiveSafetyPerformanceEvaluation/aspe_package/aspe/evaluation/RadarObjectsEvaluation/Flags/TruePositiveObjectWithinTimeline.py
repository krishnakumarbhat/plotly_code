import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags import IFlag
from aspe.evaluation.RadarObjectsEvaluation.Flags.TruePositiveFlag import TruePositiveFlag


class TruePositiveObjectWithinTimeline(IFlag):
    """
    Calculating validity flag for pair based on TruePositive Flag and given scan index boundaries for lifetime
    """
    def __init__(self, lower_si_limit=0, upper_si_limit=1, flag_signature='TP_TimeLine'):
        """
        Behaviour of args is equivalent to python's list indexes
        :param lower_si_limit: from which scan index flag will be calculated - 0 means it will take the first one
        :param upper_si_limit: up to which scan index flag will be calculated - 5 means it will take the
            one less index than 5
        """
        if lower_si_limit >= upper_si_limit:
            raise Exception("lower limit shall not be higher or equal than upper limit")
        super().__init__()
        self.flag_signature = flag_signature + f'_from_{lower_si_limit}_to_{upper_si_limit}'
        self.lower_si_limit = lower_si_limit
        self.upper_si_limit = upper_si_limit
        self.true_positive_flag = TruePositiveFlag()

    def calc_flag(self, data_frame: pd.DataFrame, *args, **kwargs) -> pd.Series:
        tp_output = self.true_positive_flag.calc_flag(data_frame)

        early_obj_ind = pd.Series(data=False, index=data_frame.index)
        for _, est_obj_df in data_frame.groupby('unique_id'):
            if est_obj_df.shape[0] >= self.upper_si_limit:
                early_obj_ind.loc[est_obj_df[self.lower_si_limit:self.upper_si_limit].index] = True
            else:
                early_obj_ind.loc[est_obj_df.index] = True

        return tp_output & early_obj_ind
