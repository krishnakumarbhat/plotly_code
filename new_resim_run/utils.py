import re
import os
from typing import Optional, Tuple


def extract_project_from_path(path: str) -> Tuple[Optional[str], Optional[str]]:
    """Extract project name and project root directory from a file path.

    Supports:
      - Krakow:  /net/.../PLKRA-PROJECTS/<PROJECT>/
      - Southfield: /mnt/.../projects/<PROJECT>/  (case-insensitive)

    Returns (project_name, project_root_dir) or (None, None).
    """
    if not path:
        return None, None

    path = path.replace('\\', '/')  # normalize Windows separators

    # Krakow: /net/.../PLKRA-PROJECTS/<PROJECT>/
    m = re.search(r'(/net/.+?)PLKRA-PROJECTS/([^/]+)', path, re.IGNORECASE)
    if m:
        root = os.path.join(m.group(1) + 'PLKRA-PROJECTS', m.group(2))
        return m.group(2), root

    # Southfield: /mnt/.../projects/<PROJECT>/  (case-insensitive)
    m = re.search(r'(/mnt/.+?)projects/([^/]+)', path, re.IGNORECASE)
    if m:
        root = os.path.join(m.group(1) + 'projects', m.group(2))
        return m.group(2), root

    return None, None


def cluster_from_path(path: str) -> Optional[str]:
    """Infer cluster name from a Unix-style path prefix."""
    if not path:
        return None
    if path.startswith('/net'):
        return 'krakow'
    if path.startswith('/mnt'):
        return 'southfield'
    return None
