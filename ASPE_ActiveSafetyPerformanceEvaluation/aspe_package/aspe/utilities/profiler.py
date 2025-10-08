import cProfile
import io
import pstats
from functools import wraps
from typing import Union


def profile(sort_key: str = 'cumulative',
            n_lines: Union[int, None] = None,
            output_file: Union[str, None] = None,
            strip_dirs: bool = False,
            ):
    """
    Time profiler decorator.

    @param sort_key: Sort will be performed with given key:
        - 'calls' - call count
        - 'cumulative' | 'cumtime' - cumulative time (DEFAULT)
        - 'file' | 'filename' | 'module' - file name
        - 'ncalls' - call count
        - 'pcalls' - primitive call count
        - 'line' - line number
        - 'name' - function name
        - 'nfl' - name/file/line
        - 'stdname' - standard name
        - 'time' | 'tottime' - internal time
    @param n_lines: Number of lines to print. If None is given, all lines will be printed.
    @param output_file: Output fle name. It will be saved in current working directory. ex. "func_profiling.prof"
    @param strip_dirs: If True removes leading path info from output.
    @return: Profile for the decorated function.
    """

    def inner(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            pr = cProfile.Profile()
            pr.enable()
            retval = func(*args, **kwargs)
            pr.disable()

            if output_file is None:
                s = io.StringIO()
                ps = pstats.Stats(pr, stream=s)
                if strip_dirs:
                    ps.strip_dirs()
                ps.sort_stats(sort_key)
                ps.print_stats(n_lines)
                print(s.getvalue())
            else:
                pr.dump_stats(output_file)
                with open(output_file, 'w') as f:
                    ps = pstats.Stats(pr, stream=f)
                    if strip_dirs:
                        ps.strip_dirs()
                    ps.sort_stats(sort_key)
                    ps.print_stats(n_lines)
                    ps.print_stats(n_lines)

            return retval

        return wrapper

    return inner
