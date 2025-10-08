# coding=utf-8
"""
F360 Host Data Set
"""
from aspe.extractors.Interfaces.IHost import IHost


class F360Host(IHost):
    """
    F360 Host Data Set class
    """
    def __init__(self):
        super().__init__()
