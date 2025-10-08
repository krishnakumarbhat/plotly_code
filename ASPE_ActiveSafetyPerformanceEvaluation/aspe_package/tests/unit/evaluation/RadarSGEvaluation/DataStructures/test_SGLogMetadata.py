from aspe.evaluation.RadarSGEvaluation.DataStructures.SGLogMetadata import SGLogMetadata


def test_SGLogMetadata_init():
    expected_out_dict = {
            "Log name": "",
            "Number of samples": 0,
            "First scan index": 0,
            "Last scan index": 0,
            "Number of classified samples": 0,
        }
    log_metadata = SGLogMetadata()
    assert log_metadata.metadata_dict == expected_out_dict


def test_SGLogMetadata_setters():
    log_metadata = SGLogMetadata()

    # Try to assign one valid and one invalid value which should be skipped
    new_metadata_values = {"Log name": "Test passed", "log_name": "SG_KPI"}
    log_metadata.metadata_dict = new_metadata_values
    expected_out_dict = {
        "Log name": "Test passed",
        "Number of samples": 0,
        "First scan index": 0,
        "Last scan index": 0,
        "Number of classified samples": 0,
    }
    assert log_metadata.metadata_dict == expected_out_dict
