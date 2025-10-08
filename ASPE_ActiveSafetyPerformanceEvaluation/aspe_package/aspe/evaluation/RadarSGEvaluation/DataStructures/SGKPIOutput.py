import numpy as np
import pandas as pd


class SGKPIOutput:
    """
    Class which stores KPI output data.
    """
    def __init__(self, general_results: dict, estimated_results: dict, reference_results: dict) -> None:
        self._general_results_dict = {'estimated_TPs': np.int32,
                                      'reference_TPs': np.int32,
                                      'FPs': np.int32,
                                      'FNs': np.int32,
                                      'TPR': np.float32,
                                      'PPV': np.float32,
                                      'F1_score': np.float32,
                                      'oversegmentation_ratio': np.float32,
                                      'undersegmentation_ratio': np.float32}

        self._samples_dict = {'bias_deviation_x': np.float32,
                              'mean_abs_deviation_x': np.float32,
                              'std_deviation_x': np.float32,
                              'q99_deviation_x': np.float32,
                              'bias_deviation_y': np.float32,
                              'mean_abs_deviation_y': np.float32,
                              'std_deviation_y': np.float32,
                              'q99_deviation_y': np.float32,
                              'q25_distance': np.float32,
                              'q50_distance': np.float32,
                              'q75_distance': np.float32,
                              'q99_distance': np.float32}

        self.general_results = general_results
        self.estimated_results = estimated_results
        self.reference_results = reference_results

    @property
    def general_results(self) -> pd.DataFrame:
        return self._general_results

    @general_results.setter
    def general_results(self, results: dict) -> None:
        if not set(self._general_results_dict.keys()) == set(results.keys()):
            raise AttributeError("Missing data inside general_results!")

        self._general_results = (pd.DataFrame.from_records([results])).astype(self._general_results_dict)

    @property
    def estimated_results(self) -> pd.DataFrame:
        return self._estimated_results

    @estimated_results.setter
    def estimated_results(self, results: dict) -> None:
        if not set(self._samples_dict.keys()) == set(results.keys()):
            raise AttributeError("Missing data inside estimated_results!")

        self._estimated_results = pd.DataFrame.from_records([results]).astype(self._samples_dict)

    @property
    def reference_results(self) -> pd.DataFrame:
        return self._reference_results

    @reference_results.setter
    def reference_results(self, results: dict) -> None:
        if not set(self._samples_dict.keys()) == set(results.keys()):
            raise AttributeError("Missing data inside reference_results!")

        self._reference_results = pd.DataFrame.from_records([results]).astype(self._samples_dict)
