import re
import base64
import numpy as np
from pathlib import Path

p = Path(r"outasdoufh_first3_debug_fix5/CCA_9010_3100099_DEBUG_20251006_145301_0004_ORCAS 3 (1)/FL/DETECTION_STREAM/CCA_9010_3100099_DEBUG_20251006_145301_0004_ORCAS 3 (1)_FL_general.html")
s = p.read_text(encoding="utf-8", errors="ignore")

matches = re.findall(r'"name":"OUTPUT".*?"y":\{"dtype":"f8","bdata":"([^"]+)"', s, flags=re.S)
print("output_traces", len(matches))
for idx, bdata in enumerate(matches[:8]):
    payload = bdata.replace("\\u002f", "/")
    arr = np.frombuffer(base64.b64decode(payload), dtype=np.float64)
    finite = np.isfinite(arr)
    print(idx, "len", arr.size, "finite", int(finite.sum()), "nan", int((~finite).sum()))
