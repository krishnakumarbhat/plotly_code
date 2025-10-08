from aspe.extractors.Interfaces.ExtractedData import ExtractedData


def check_if_data_set_exists_and_is_not_empty(extracted_data: ExtractedData, data_set_name: str):
    """
    Check if data set of given name exists within extracted_data and is not empty.
    :param extracted_data:
    :param data_set_name:
    :return:
    """
    try:
        data_set = getattr(extracted_data, data_set_name)
        return bool(data_set is not None and len(data_set.signals) > 0)
    except AttributeError:
        return False
