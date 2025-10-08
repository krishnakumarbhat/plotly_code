# coding=utf-8
"""
Interface for F360 MUDP builder selectors
"""
from abc import ABC

from aspe.extractors.F360.Mudp.Exceptions.MissingStreamException import MissingStreamException
from aspe.extractors.F360.Mudp.Exceptions.VersionNotSupportedException import VersionNotSupportedException


class IMudpBuilderSelector(ABC):
    """
    Abstract builder selector for F360 MUDP data.
    Dataset-specific builder factories (e.g IF360MudpObjectsBuilder) should inherit from this class.
    """
    required_stream_numbers = frozenset()
    legacy_stream_numbers = frozenset()
    available_builders = tuple()

    def select_builder(self, parsed_data):
        """
        Chooses an appropriate builder version for supplied parsed data.
        Raises MissingStreamException in case any required stream is missing in parsed data.
        Raises VersionNotSupportedException in case no version is appropriate for provided data.
        Dataset-specific builder factories (e.g IF360MudpObjectsBuilderSelector) should override this method.
        :parsed_data: data from parsers
        :return: a reference to a an appropriate version of dataset builder class, e.g. F360MudpObjectsBuilderV15
        """

        # check if all required streams are present in parsed data
        parsed_stream_numbers = frozenset(parsed_data['header_data'].keys())
        missing_stream_numbers = self.required_stream_numbers - parsed_stream_numbers

        if missing_stream_numbers:
            missing_stream_numbers = set(self.legacy_stream_numbers) - parsed_stream_numbers

        if missing_stream_numbers:
            raise MissingStreamException(missing_stream_numbers)

        # match appropriate version of builder
        for builder_class in self.available_builders:
            if builder_class.can_handle(parsed_data):  # if this line fails, make sure that available_builders
                # contains references to builder classes, not modules (check imports)
                return builder_class
        raise VersionNotSupportedException(self.available_builders)
