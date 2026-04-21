from __future__ import annotations

import json
from pathlib import Path

import h5py

from a_config_layer.da_config_loader import DaConfigLoader
from b_persistence_layer.hdf_snapshot_repository import HdfSnapshotRepository
from b_persistence_layer.html_report_repository import HtmlReportRepository
from c_data_storage.run_state_storage import RunStateStorage
from d_business_layer.da_router_business import DaRouterBusiness
from d_business_layer.project_analyzer import ProjectAnalyzer
from d_business_layer.project_service_factory import ProjectServiceFactory


def _create_small_hdf(path: Path) -> None:
    with h5py.File(path, "w") as handle:
        sensor = handle.create_group("SENSOR_FRONT")
        stream = sensor.create_group("DETECTION_STREAM")
        stream.create_dataset("ran", data=[1.0, 2.0, 3.0])
        stream.create_dataset("vel", data=[0.1, 0.2, 0.3])


def test_end_to_end_mudp_generates_udp_html(tmp_path: Path):
    input_hdf = tmp_path / "in.h5"
    output_hdf = tmp_path / "out.h5"
    _create_small_hdf(input_hdf)
    _create_small_hdf(output_hdf)

    xml_path = tmp_path / "ConfigInteractivePlots.xml"
    xml_path.write_text(
        """<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<Interactiveplot_Extracter_Configuration>
  <HDF_SOURCE_SELECTION>MUDP</HDF_SOURCE_SELECTION>
  <HDF_FILE>HDF_WITH_ALLSENSOR</HDF_FILE>
  <PLOT_MODE><KPI>1</KPI></PLOT_MODE>
</Interactiveplot_Extracter_Configuration>
""",
        encoding="utf-8",
    )

    inputs_path = tmp_path / "inputs.json"
    inputs_path.write_text(
        json.dumps({"INPUT_HDF": [str(input_hdf)], "OUTPUT_HDF": [str(output_hdf)]}),
        encoding="utf-8",
    )

    config_path = tmp_path / "config.json"
    config_path.write_text(
        json.dumps(
            {
                "hdf_source_selection": "AUTO",
                "xml_config": str(xml_path),
                "inputs_json": str(inputs_path),
                "run_int": False,
            }
        ),
        encoding="utf-8",
    )

    loader = DaConfigLoader(tmp_path)
    config = loader.load(
        config_path=str(config_path),
        output_dir=str(tmp_path / "out_html"),
        source_override=None,
        run_int_override=None,
    )

    state = RunStateStorage()
    analyzer = ProjectAnalyzer(HdfSnapshotRepository(), HtmlReportRepository(), state)
    router = DaRouterBusiness(ProjectServiceFactory(analyzer), state)
    results = router.execute(config)

    assert len(results) == 1
    assert results[0]["project"] == "udp"
    assert results[0]["return_code"] == 0
    assert Path(results[0]["index_html"]).exists()
    assert (tmp_path / "out_html" / "udp" / "udp_pair_1.html").exists()
