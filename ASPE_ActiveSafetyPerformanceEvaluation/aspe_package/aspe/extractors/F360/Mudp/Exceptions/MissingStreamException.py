# coding=utf-8
import typing


class MissingStreamException(Exception):
    """
    Required stream is not present in parsed data.
    """
    def __init__(self, missing_stream_numbers: typing.Collection[int]):
        self.missing_stream_numbers = missing_stream_numbers

        message = 'Stream' + ('s' if len(missing_stream_numbers) > 1 else '') + ' ' + \
                  ', '.join(map(str, sorted(missing_stream_numbers))) + ' not present in parsed data'
        super().__init__(message)
