class SGLogMetadata:
    def __init__(self) -> None:
        """
        Class which stores various log metadata.
        """
        self._metadata_dict = {
            "Log name": "",
            "Number of samples": 0,
            "First scan index": 0,
            "Last scan index": 0,
            "Number of classified samples": 0,
        }

    @property
    def metadata_dict(self) -> dict:
        return self._metadata_dict

    @metadata_dict.setter
    def metadata_dict(self, results: dict):
        for key, value in results.items():
            if key in self._metadata_dict:
                self._metadata_dict[key] = value
