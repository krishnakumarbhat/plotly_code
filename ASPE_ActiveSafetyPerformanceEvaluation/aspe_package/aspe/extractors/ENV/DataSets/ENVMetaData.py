from typing import Optional

from aspe.extractors.Interfaces.IMetaData import IMetaData


class ENVMetadata(IMetaData):
    def __init__(self):
        super().__init__()

        self.annotations: Optional[str] = None
        self.conversion_history: Optional[str] = None
        self.creation_date: Optional[str] = None
        self.version_component_resim_interface: Optional[int] = None
        self.version_component_resim_serialization: Optional[int] = None
        self.version_keg: Optional[int] = None
