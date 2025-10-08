# coding=utf-8
"""
F360 Trailer Data Set
"""
from aspe.extractors.Interfaces.ITrailer import ITrailer


class F360Trailer(ITrailer):
    """
    F360 Trailer Data Set class
    """
    def __init__(self):
        super().__init__()
