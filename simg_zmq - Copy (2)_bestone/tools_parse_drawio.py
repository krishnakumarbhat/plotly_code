import xml.etree.ElementTree as ET
import sys
p='architecture_agentic_rag.drawio'
try:
    ET.parse(p)
    print('XML parse: OK')
except Exception as e:
    print('XML parse ERROR:', repr(e))
    sys.exit(1)
