import numpy as np


class Statistic:
    def __init__(self, name, func, enable):
        self.name = name
        self.compute = func
        self.linestyle = '-'


class Mean(Statistic):
    def __init__(self):
        self.name = "mean"
        self.compute = np.mean
        self.linestyle = '-'
        self.alpha = 1.0


class Std(Statistic):
    def __init__(self):
        self.name = "std"
        self.compute = np.std
        self.linestyle = '-'
        self.alpha = 0.5


class Median(Statistic):
    def __init__(self):
        self.name = "median"
        self.compute = np.median
        self.linestyle = '-'
        self.alpha = 1.0


class StdBound(Statistic):
    def __init__(self, multiplier):
        self.multiplier = multiplier
        self.name = f"{multiplier}std"
        self.compute = self.multi_std
        self.linestyle = '-'
        self.alpha = 0.5

    def multi_std(self, x):
        return np.mean(x) + self.multiplier * np.std(x)


class Quantile(Statistic):
    def __init__(self, quantile):
        self.quantile = quantile
        self.name = f"q{str(int(quantile * 100)).zfill(2)}"
        self.compute = self.calc_quantile
        self.linestyle = '--'
        self.alpha = 0.5

    def calc_quantile(self, x):
        return np.quantile(x, self.quantile)