import numpy as np
from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification.IBinaryClassifier import BCType

def calculate_binary_classification_kpis(ref_data, est_data):
    """
    Calculate all binary classification kpis.
    :param ref_data: DataFrame column with binary classification result for reference data
    :param est_data: DataFrame column with binary classification result for estimated data
    :return: dictionary with calculated kpis
    """
    tpr, tp_ref_count, fn_count = calculate_tpr(ref_data)
    ppv, tp_est_count, fp_count = calculate_ppv(est_data)
    f1_score = calculate_f1_score(ref_data, est_data)
    kpis = {'TPR': tpr, 'PPV': ppv, 'F1 score': f1_score,
            'TP_NUM': tp_ref_count, 'FP_NUM': fp_count, 'FN_NUM': fn_count}
    return kpis


def calculate_deviation_calculation_kpis(deviation_data):
    """
    Calculate all deviation calculation kpis
    :param deviation_data: DataFrame column with calculated deviation
    :return: dictionary with calculated kpis
    """
    deviation_data = deviation_data.astype('float64')
    dev_data = deviation_data[~np.isnan(deviation_data)]  # delete nan from vector
    accuracy = calculate_accuracy(dev_data)
    precision = calculate_precision(dev_data)
    rmse = calculate_rmse(dev_data)
    mae = calculate_mae(dev_data)
    aae = calculate_aae(dev_data)
    kpis = {'Accuracy': accuracy,
            'Precision': precision,
            'RMSE': rmse,
            'MAE': mae,
            'AAE': aae}
    return kpis


def calculate_tpr(ref_data):
    """
    Calculate TPR - True Positive Ratio
    :param ref_data: DataFrame column with binary classification result for reference data
    :return: tuple, (TPR value, True Positive objects count, False Negative objects count)
    """
    tp_count = ref_data[ref_data == BCType.TruePositive].count()
    fn_count = ref_data[ref_data == BCType.FalseNegative].count()
    ref_objs_count = tp_count + fn_count
    if ref_objs_count > 0:
        tpr = tp_count / ref_objs_count
    else:
        tpr = np.nan
    return tpr, tp_count, fn_count


def calculate_ppv(est_data):
    """
    Calculate Positive Predictive Value
    :param est_data: DataFrame column with binary classification result for estimated data
    :return: tuple, (PPV value, True Positive objects count)
    """
    tp_count = est_data[est_data == BCType.TruePositive].count()
    fp_count = est_data[est_data == BCType.FalsePositive].count()
    est_objs_count = tp_count + fp_count
    if est_objs_count > 0:
        ppv = tp_count / est_objs_count
    else:
        ppv = np.nan
    return ppv, tp_count, fp_count


def calculate_f1_score(ref_data, est_data):
    """
    Calculate F1 score
    https://en.wikipedia.org/wiki/F1_score
    :param ref_data: DataFrame column with binary classification result for reference data
    :param est_data: DataFrame column with binary classification result for estimated data
    :return: f1 score value
    """
    ppv, _, _ = calculate_ppv(est_data)
    tpr, _, _ = calculate_tpr(ref_data)
    if not np.isnan(ppv) and not np.isnan(tpr) and not ppv + tpr == 0:
        f1_score = 2 * (tpr * ppv) / (tpr + ppv)
    else:
        f1_score = np.nan
    return f1_score


def calculate_accuracy(dev_data):
    """
    Calculate accuracy - defined as mean value
    :param dev_data: DataFrame column with calculated deviation
    :return: accuracy value
    """
    return np.mean(dev_data)


def calculate_precision(dev_data):
    """
    Calculate precision value, defined as standard deviation
    :param dev_data: DataFrame column with calculated deviation
    :return: precision value
    """
    return np.std(dev_data)


def calculate_rmse(dev_data):
    """
    Calculate Root Mean Square Error value
    :param dev_data: DataFrame column with calculated deviation
    :return: rmse value
    """
    return np.sqrt(np.mean(np.square(dev_data)))


def calculate_mae(dev_data):
    """
    Calculate Median Absolute Error
    :param dev_data: DataFrame column with calculated deviation
    :return: mae value
    """
    return np.median(np.abs(dev_data))


def calculate_aae(dev_data):
    """
    Calculate Average Absolute Error
    :param dev_data: DataFrame column with calculated deviation
    :return: aae value
    """
    return np.mean(np.abs(dev_data))
