import os
import pickle
from pathlib import Path

import numpy as np
import pytest
from pandas.testing import assert_frame_equal

from aspe.extractors.ENV.Keg.builders.ENVKegStationaryGeometriesBuilder import ENVKegStationaryGeometriesBuilder


@pytest.fixture()
def in_out():
    input_file = Path(os.getenv("ASPE_TEST_LOG_PATH")) / "fixtures" / "SG_in_all.pkl"
    with Path.open(input_file, "rb") as file:
        data = pickle.load(file)
        timestamps = {"timestamp": data["timestamp"]}

    output_file = Path(os.getenv("ASPE_TEST_LOG_PATH")) / "fixtures" / "SG_out.pkl"
    with Path.open(output_file, "rb") as file:
        out = pickle.load(file)

    return [data, out, timestamps]


def test_output(in_out):
    [data, out, timestamps] = in_out
    builder = ENVKegStationaryGeometriesBuilder(data, timestamps)
    data_set = builder.build()
    # TODO DOH-235: Investigate wrong d_types
    assert assert_frame_equal(out.signals, data_set.signals) is None


def test_wrong_grid_shape(in_out):
    error_msg = "Array shape does not match grid shape. drivability_class will not be extracted."
    with pytest.warns(UserWarning, match=error_msg):
        [data, _, timestamps] = in_out
        temp = np.array(data["SG"]["output"]["contours"]["drivability_class"])
        data["SG"]["output"]["contours"]["drivability_class"] = np.concatenate((temp, temp), axis=1)
        builder = ENVKegStationaryGeometriesBuilder(data, timestamps)
        builder.build()
