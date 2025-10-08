from abc import ABC, abstractmethod


class IBaseEvent(ABC):
    def __init__(self, extracted_data, components):
        self.extracted_data = extracted_data
        self.components = components

    @abstractmethod
    def calculate(self):
        raise NotImplementedError

    @abstractmethod
    def _init_supporting_variables(self):
        raise NotImplementedError
