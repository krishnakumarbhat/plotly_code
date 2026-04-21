from pathlib import Path

from a_config_layer.da_config_loader import DaConfigLoader


def test_loader_reads_xml_source_and_file_mode():
    workspace_root = Path(__file__).resolve().parents[1]
    loader = DaConfigLoader(workspace_root)
    config = loader.load(
        config_path="config.json",
        output_dir="out_test_loader",
        source_override=None,
        run_int_override=None,
    )
    assert config["hdf_source_selection"] in {"MUDP", "BORDNET"}
    assert config["hdf_file_mode"] == "HDF_WITH_ALLSENSOR"


def test_loader_builds_pairs_from_inputs_json():
    workspace_root = Path(__file__).resolve().parents[1]
    loader = DaConfigLoader(workspace_root)
    config = loader.load(
        config_path="config.json",
        output_dir="out_test_loader_pairs",
        source_override="MUDP",
        run_int_override=False,
    )
    assert len(config["pairs"]) >= 1
    first = config["pairs"][0]
    assert "input" in first and "output" in first
