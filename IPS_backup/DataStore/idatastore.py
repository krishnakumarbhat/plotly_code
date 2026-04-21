from abc import ABC, abstractmethod

class IDataStore(ABC):
    @abstractmethod
    def get_data(self, signal, data_source): pass
    @abstractmethod
    def update_data(self, signal, data_source, data): pass
