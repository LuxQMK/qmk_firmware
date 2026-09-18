import os
import re
import sys

sys.stdout.reconfigure(encoding='utf-8')

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
STUDIO_DIR = os.path.dirname(SCRIPT_DIR)
SRC_DIR = STUDIO_DIR

with open(os.path.join(SRC_DIR, "index.html"), "r", encoding="utf-8") as f:
    html = f.read()

data_i18n_keys = set(re.findall(r'data-i18n=["\']([^"\']+)["\']', html))
data_i18n_titles = set(re.findall(r'data-i18n-title=["\']([^"\']+)["\']', html))
data_i18n_placeholders = set(re.findall(r'data-i18n-placeholder=["\']([^"\']+)["\']', html))
all_html_keys = data_i18n_keys | data_i18n_titles | data_i18n_placeholders

with open(os.path.join(SRC_DIR, "js", "i18n.js"), "r", encoding="utf-8") as f:
    i18n_code = f.read()

en_match = re.search(r'en:\s*\{(.*?)\n\s*\},', i18n_code, re.DOTALL)
pl_match = re.search(r'pl:\s*\{(.*?)\n\s*\}\n\s*\};', i18n_code, re.DOTALL)

def get_keys(block):
    return set(re.findall(r'^\s*([a-zA-Z0-9_]+)\s*:', block, re.MULTILINE))

en_keys = get_keys(en_match.group(1)) if en_match else set()
pl_keys = get_keys(pl_match.group(1)) if pl_match else set()

print(f"Total HTML keys: {len(all_html_keys)}")
print(f"EN dictionary keys: {len(en_keys)}")
print(f"PL dictionary keys: {len(pl_keys)}")

missing_in_en = all_html_keys - en_keys
missing_in_pl = all_html_keys - pl_keys
diff_en_pl = en_keys.symmetric_difference(pl_keys)

if missing_in_en:
    print(f"[ERROR] HTML keys missing in EN dictionary: {sorted(list(missing_in_en))}")
else:
    print("[OK] 100% of HTML keys exist in EN dictionary!")

if missing_in_pl:
    print(f"[ERROR] HTML keys missing in PL dictionary: {sorted(list(missing_in_pl))}")
else:
    print("[OK] 100% of HTML keys exist in PL dictionary!")

if diff_en_pl:
    print(f"[ERROR] Discrepancy between EN and PL keys: {diff_en_pl}")
else:
    print("[OK] EN and PL dictionaries are 100% synchronized and identical in key structure!")
