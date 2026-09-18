"""
LuxQMK Studio HTML/CSS/JS Inliner / Bundler
Bundles modular files from luxqmk-studio/ into single standalone luxqmk_studio.html (and aliases)
"""

import os
import re
import shutil

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WORKSPACE_DIR = os.path.dirname(ROOT_DIR)
FIRMWARE_DIR = os.path.join(WORKSPACE_DIR, "qmk_firmware")
SRC_DIR = ROOT_DIR
OUTPUT_FILE_PRIMARY = os.path.join(ROOT_DIR, "luxqmk_studio.html")
OUTPUT_FILE_ALIAS = os.path.join(ROOT_DIR, "gmmk_studio.html")

def bundle():
    with open(os.path.join(SRC_DIR, "index.html"), "r", encoding="utf-8") as f:
        html = f.read()

    # Inlining CSS
    css_links = [
        ("css/variables.css", os.path.join(SRC_DIR, "css", "variables.css")),
        ("css/sidebar.css", os.path.join(SRC_DIR, "css", "sidebar.css")),
        ("css/layout.css", os.path.join(SRC_DIR, "css", "layout.css")),
        ("css/components.css", os.path.join(SRC_DIR, "css", "components.css")),
    ]

    all_css = []
    for rel_path, abs_path in css_links:
        if os.path.exists(abs_path):
            with open(abs_path, "r", encoding="utf-8") as f:
                all_css.append(f"/* === {rel_path} === */\n" + f.read())
            html = re.sub(rf'<link\s+rel="stylesheet"\s+href="{re.escape(rel_path)}">', '', html)

    css_tag = f"<style>\n{chr(10).join(all_css)}\n</style>"
    html = html.replace("</head>", f"  {css_tag}\n</head>")

    # Inlining JS (maintaining strict dependency order)
    js_scripts = [
        ("js/layout-data.js", os.path.join(SRC_DIR, "js", "layout-data.js")),
        ("js/devices/gmmk3.js", os.path.join(SRC_DIR, "js", "devices", "gmmk3.js")),
        ("js/devices/gmmk2.js", os.path.join(SRC_DIR, "js", "devices", "gmmk2.js")),
        ("js/devices/generic-via.js", os.path.join(SRC_DIR, "js", "devices", "generic-via.js")),
        ("js/device-manager.js", os.path.join(SRC_DIR, "js", "device-manager.js")),
        ("js/keycodes-db.js", os.path.join(SRC_DIR, "js", "keycodes-db.js")),
        ("js/i18n.js", os.path.join(SRC_DIR, "js", "i18n.js")),
        ("js/hid-protocol.js", os.path.join(SRC_DIR, "js", "hid-protocol.js")),
        ("js/keymap-editor.js", os.path.join(SRC_DIR, "js", "keymap-editor.js")),
        ("js/lighting-controller.js", os.path.join(SRC_DIR, "js", "lighting-controller.js")),
        ("js/audio-visualizer.js", os.path.join(SRC_DIR, "js", "audio-visualizer.js")),
        ("js/key-tester.js", os.path.join(SRC_DIR, "js", "key-tester.js")),
        ("js/backup-manager.js", os.path.join(SRC_DIR, "js", "backup-manager.js")),
        ("js/ui-controller.js", os.path.join(SRC_DIR, "js", "ui-controller.js")),
        ("js/app.js", os.path.join(SRC_DIR, "js", "app.js")),
    ]

    all_js = []
    for rel_path, abs_path in js_scripts:
        if os.path.exists(abs_path):
            with open(abs_path, "r", encoding="utf-8") as f:
                all_js.append(f"/* === {rel_path} === */\n" + f.read())
            html = re.sub(rf'<script\s+src="{re.escape(rel_path)}"></script>', '', html)
    js_tag = f"<script>\n{chr(10).join(all_js)}\n</script>"
    html = html.replace("</body>", f"  {js_tag}\n</body>")

    # Inlining images (icon.png -> Base64 data URI)
    import base64
    icon_path = os.path.join(SRC_DIR, "assets", "icon.png")
    if os.path.exists(icon_path):
        with open(icon_path, "rb") as img_f:
            b64_img = base64.b64encode(img_f.read()).decode("utf-8")
        html = html.replace('src="assets/icon.png"', f'src="data:image/png;base64,{b64_img}"')

    # Write primary bundle
    with open(OUTPUT_FILE_PRIMARY, "w", encoding="utf-8") as f:
        f.write(html)
    print(f"[OK] Primary Bundle -> {OUTPUT_FILE_PRIMARY} ({os.path.getsize(OUTPUT_FILE_PRIMARY):,} bytes)")

    # Write alias bundle
    with open(OUTPUT_FILE_ALIAS, "w", encoding="utf-8") as f:
        f.write(html)
    print(f"[OK] Alias Bundle   -> {OUTPUT_FILE_ALIAS} ({os.path.getsize(OUTPUT_FILE_ALIAS):,} bytes)")

    # Mirror to qmk_firmware root if directory exists
    if os.path.exists(FIRMWARE_DIR):
        fw_primary = os.path.join(FIRMWARE_DIR, "luxqmk_studio.html")
        fw_alias = os.path.join(FIRMWARE_DIR, "gmmk_studio.html")
        shutil.copy2(OUTPUT_FILE_PRIMARY, fw_primary)
        shutil.copy2(OUTPUT_FILE_ALIAS, fw_alias)
        print(f"[OK] Mirrored bundles to qmk_firmware directory.")

if __name__ == "__main__":
    bundle()

