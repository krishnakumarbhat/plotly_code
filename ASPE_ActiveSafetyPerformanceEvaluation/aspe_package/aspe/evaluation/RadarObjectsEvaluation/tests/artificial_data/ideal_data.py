from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.conversions import \
    create_ideal_est_from_ref
from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.ref_test_vectors import \
    get_ref_test_vectors


def get_ideal_data():
    """
    Get ideal data which means that there are several test vectors for which estimated and reference objects are the
    same.
    Number and name of test vectors are the same as for output from get_ref_test_vectors() function
    :return: dict with named test vectors,
             each test vector contains dict with'ref_data' and 'est_data' containing ExtractedData
    """

    ref_data = get_ref_test_vectors()
    ideal_data = dict()
    for key, ref_test_vector in ref_data.items():
        est_test_vector = create_ideal_est_from_ref(ref_test_vector)
        ideal_data[key] = {'ref_data': ref_test_vector, 'est_data': est_test_vector}
    return ideal_data


