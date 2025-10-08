# coding=utf-8
import typing

from aspe.extractors.Mudp.IMudpBuilder import IMudpBuilder


class VersionNotSupportedException(Exception):
    """
    No dataset builder found for the version of stream definition from parsed data.
    """
    def __init__(self, available_builders: typing.Iterable[IMudpBuilder]):
        builders_names = ', '.join(builder_class.__name__ for builder_class in available_builders)
        message = f'None of {builders_names} can handle this version of parsed data. ' + \
                  'Please verify stream definition versions or implement a custom builder.'
        super().__init__(message)
