import os

import pytest

from aspe.extractors.ReferenceExtractor.dSpace.datasets.dspace_objects import DspaceObjects
from aspe.extractors.ReferenceExtractor.dSpace.datasets.dspace_metadata import DspaceMetadata
from aspe.extractors.ReferenceExtractor.dSpace.dspace_bin_extractor import DSpaceBinExtractor
from aspe.parsers.BinParser.bin_parser import BinParser

from aspe.examples.evaluation.private.user_data import ExampleData


@pytest.fixture
def parsed_log() -> dict:
    log_path = os.path.join(
        ExampleData.gdsr_dspace_data_path,
        r"Tracker_UC_10_6_E0_T0_DtoT05_I20_B0\REAR_LEFT\Tracker_UC_10_6_E0_T0_DtoT05_I20_lm2_OSIGTInput.bin",
    )
    return BinParser().parse(log_path)


def test_extraction_metadata(parsed_log):
    extracted_data = DSpaceBinExtractor().extract_data(parsed_log)

    assert extracted_data.metadata is not None
    assert isinstance(extracted_data.metadata, DspaceMetadata)
    assert extracted_data.metadata.extractor_type == "dSpace"
    assert extracted_data.metadata.data_path == parsed_log["path"]


def test_extraction_objects_raw_signals_disabled(parsed_log):
    extracted_data = DSpaceBinExtractor(f_extract_raw_signals=False).extract_data(parsed_log)

    assert extracted_data.objects is not None
    assert isinstance(extracted_data.objects, DspaceObjects)
    assert not extracted_data.objects.signals.empty
    assert extracted_data.objects.raw_signals is None


def test_extraction_objects_raw_signals_enabled(parsed_log):
    extracted_data = DSpaceBinExtractor(f_extract_raw_signals=True).extract_data(parsed_log)

    assert extracted_data.objects is not None
    assert isinstance(extracted_data.objects, DspaceObjects)
    assert not extracted_data.objects.signals.empty
    assert not extracted_data.objects.raw_signals.empty
