from c_data_storage.run_state_storage import RunStateStorage
from d_business_layer.da_router_business import DaRouterBusiness


class FakeFactory:
    def __init__(self) -> None:
        self.calls: list[str] = []

    def create(self, project_name: str):
        self.calls.append(project_name)
        return FakeService(project_name)


class FakeService:
    def __init__(self, name: str) -> None:
        self._name = name

    def execute(self, config):
        return {"project": self._name, "return_code": 0, "index_html": "dummy"}


def test_router_selects_udp_for_mudp():
    factory = FakeFactory()
    router = DaRouterBusiness(factory, RunStateStorage())
    results = router.execute({"hdf_source_selection": "MUDP", "run_int": False})
    assert factory.calls == ["udp"]
    assert len(results) == 1


def test_router_selects_can_for_bordnet_with_int():
    factory = FakeFactory()
    router = DaRouterBusiness(factory, RunStateStorage())
    results = router.execute({"hdf_source_selection": "BORDNET", "run_int": True})
    assert factory.calls == ["int", "can"]
    assert len(results) == 2
