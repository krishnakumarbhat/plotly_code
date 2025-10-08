from aspe.extractors.GDSR.datasets.gdsr_radar_sensors import GdsrRadarSensors
import pytest
import os

from aspe.extractors.GDSR.datasets.gdsr_objects import GdsrObjects
from aspe.extractors.GDSR.datasets.gdsr_metadata import GdsrMetadata
from aspe.extractors.GDSR.gdsr_bin_extractor import GdsrBinExtractor, GdsrParsedData
from aspe.parsers.BinParser.bin_parser import BinParser

from aspe.examples.evaluation.private.user_data import ExampleData


@pytest.fixture
def parsed_log() -> dict:
    tracker_out_log_path = os.path.join(
        ExampleData.gdsr_dspace_data_path,
        r"Tracker_UC_10_6_E0_T0_DtoT05_I20_B0\FRONT_RIGHT\Tracker_UC_10_6_E0_T0_DtoT05_I20_lm2_TrackerOutput.bin",
    )
    radar_params_log_path = os.path.join(
        ExampleData.gdsr_dspace_data_path,
        r"Tracker_UC_10_6_E0_T0_DtoT05_I20_B0\FRONT_RIGHT\Tracker_UC_10_6_E0_T0_DtoT05_I20_lm2_RadarParams.bin",
    )

    parser = BinParser()
    tracker_out = parser.parse(tracker_out_log_path)
    radar_params = parser.parse(radar_params_log_path)

    return GdsrParsedData(tracker_out, radar_params)


def test_extraction_metadata(parsed_log: GdsrParsedData):
    extracted_data = GdsrBinExtractor().extract_data(parsed_log)

    assert extracted_data.metadata is not None
    assert isinstance(extracted_data.metadata, GdsrMetadata)
    assert extracted_data.metadata.extractor_type == "GDSR"
    assert extracted_data.metadata.data_path == parsed_log.tracker_output["path"]


def test_extraction_objects_raw_signals_disabled(parsed_log: GdsrParsedData):
    extracted_data = GdsrBinExtractor(f_extract_raw_signals=False).extract_data(parsed_log)

    assert extracted_data.objects is not None
    assert isinstance(extracted_data.objects, GdsrObjects)
    assert not extracted_data.objects.signals.empty
    assert extracted_data.objects.raw_signals is None


def test_extraction_objects_raw_signals_enabled(parsed_log: GdsrParsedData):
    extracted_data = GdsrBinExtractor(f_extract_raw_signals=True).extract_data(parsed_log)

    assert extracted_data.objects is not None
    assert isinstance(extracted_data.objects, GdsrObjects)
    assert not extracted_data.objects.signals.empty
    assert not extracted_data.objects.raw_signals.empty


def test_extraction_sensors_raw_signals_disabled(parsed_log: GdsrParsedData):
    extracted_data = GdsrBinExtractor(f_extract_raw_signals=False).extract_data(parsed_log)

    assert extracted_data.sensors is not None
    assert isinstance(extracted_data.sensors, GdsrRadarSensors)
    assert not extracted_data.sensors.signals.empty
    assert not extracted_data.sensors.per_sensor.empty
    assert not extracted_data.sensors.per_look.empty
    assert extracted_data.sensors.raw_signals is None


def test_extraction_sensors_raw_signals_enabled(parsed_log: GdsrParsedData):
    extracted_data = GdsrBinExtractor(f_extract_raw_signals=True).extract_data(parsed_log)

    assert extracted_data.sensors is not None
    assert isinstance(extracted_data.sensors, GdsrRadarSensors)
    assert not extracted_data.sensors.signals.empty
    assert not extracted_data.sensors.per_sensor.empty
    assert not extracted_data.sensors.per_look.empty
    assert not extracted_data.sensors.raw_signals.empty
