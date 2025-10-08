import pytest

from aspe.utilities.PathProvider import PathProvider


@pytest.fixture(scope='session')
def path_provider():
    remote_stream_def_path = \
        r"\\10.224.186.68\AD-Shared\ASPE\configurations\F360\MUDP_Stream_Definitions\stream_definitions"
    remote_rt_range_dbc_path = r"\\10.224.186.68\AD-Shared\ASPE\configurations\parsers_config\RtRange3000.dbc"
    return PathProvider(remote_stream_def_path=remote_stream_def_path,
                        remote_rt_range_dbc_path=remote_rt_range_dbc_path)
