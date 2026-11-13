from pathlib import Path
import re, gzip, hashlib, json
import esphome
from importlib.metadata import version

assert version('esphome') == '2026.10.2', 'Webserver-Quelle für diese ESPHome-Version zuerst prüfen'

root=Path.cwd()
header=Path(esphome.__file__).parent/'components/web_server/server_index_v3.h'
source=header.read_text(encoding='utf-8')
array=re.search(r'INDEX_GZ\[\].*?\{(.*?)\};',source,re.S).group(1)
html=gzip.decompress(bytes(int(v,16) for v in re.findall(r'0x([0-9a-fA-F]{2})',array))).decode('utf-8')
module=re.search(r'<script\b[^>]*type=module[^>]*>(.*?)</script>',html,re.S).group(1)
help_js=(root/'web/lwz-help.js').read_text(encoding='utf-8')
combined='// ESPHome webserver v3, bundled with ESPHome 2026.10.2.\n// Upstream license: ESPHOME-WEBSERVER-LICENSE.txt. LWZ help appended below.\n'+module+'\n;\n'+help_js
(root/'web/lwz-webserver-v3-with-help.js').write_text(combined,encoding='utf-8',newline='\n')
manifest={'source':'ESPHome 2026.10.2 components/web_server/server_index_v3.h','upstream':'https://github.com/esphome/esphome-webserver','header_sha256':hashlib.sha256(header.read_bytes()).hexdigest(),'upstream_module_sha256':hashlib.sha256(module.encode()).hexdigest(),'help_sha256':hashlib.sha256(help_js.encode()).hexdigest(),'combined_sha256':hashlib.sha256(combined.encode()).hexdigest()}
(root/'web/SOURCES.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
print('Prepared bundled, offline ESPHome v3 frontend with static LWZ help')
