#!/usr/bin/env python3
"""
LuxQMK Firmware Catalog Generator & Asset Manager
Generates `catalog.json` with metadata, SHA-256 checksums, and download endpoints for `files.luxqmk.click/firmware`.
Also creates Cloudflare Pages `_redirects`, `_headers`, and fallback `index.html` pointing to `luxqmk.click`.
"""

import os
import sys
import json
import hashlib
import argparse
from datetime import datetime, timezone

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
KEYBOARDS_DIR = os.path.join(ROOT_DIR, "keyboards")

KNOWN_BOARDS = {
    # GMMK 3 Series (ANSI & ISO)
    "gmmk_gmmk3_p100_ansi_via": {
        "name": "Glorious GMMK 3 (100% ANSI)",
        "vendor_id": "0x504B",
        "product_id": "0x320F",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater_cli",
        "layout": "100% Full ANSI",
        "tier": "luxqmk_enhanced",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "logo_led", "win_lock_led", "sidelights", "encoder"]
    },
    "gmmk_gmmk3_p100_iso_via": {
        "name": "Glorious GMMK 3 (100% ISO)",
        "vendor_id": "0x504B",
        "product_id": "0x321F",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater_cli",
        "layout": "100% Full ISO",
        "tier": "luxqmk_enhanced",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "logo_led", "win_lock_led", "sidelights", "encoder"]
    },
    "gmmk_gmmk3_p75_ansi_via": {
        "name": "Glorious GMMK 3 (75% ANSI)",
        "vendor_id": "0x504B",
        "product_id": "0x320E",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater_cli",
        "layout": "75% Compact ANSI",
        "tier": "luxqmk_enhanced",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "logo_led", "win_lock_led", "sidelights", "encoder"]
    },
    "gmmk_gmmk3_p75_iso_via": {
        "name": "Glorious GMMK 3 (75% ISO)",
        "vendor_id": "0x504B",
        "product_id": "0x321E",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater_cli",
        "layout": "75% Compact ISO",
        "tier": "luxqmk_enhanced",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "logo_led", "win_lock_led", "sidelights", "encoder"]
    },
    "gmmk_gmmk3_p65_ansi_via": {
        "name": "Glorious GMMK 3 (65% ANSI)",
        "vendor_id": "0x504B",
        "product_id": "0x320D",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater_cli",
        "layout": "65% Compact ANSI",
        "tier": "luxqmk_enhanced",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "logo_led", "win_lock_led", "sidelights", "encoder"]
    },
    "gmmk_gmmk3_p65_iso_via": {
        "name": "Glorious GMMK 3 (65% ISO)",
        "vendor_id": "0x504B",
        "product_id": "0x321D",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater_cli",
        "layout": "65% Compact ISO",
        "tier": "luxqmk_enhanced",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "logo_led", "win_lock_led", "sidelights", "encoder"]
    },

    # GMMK 2 Series (ANSI & ISO)
    "gmmk_gmmk2_p96_ansi_via": {
        "name": "Glorious GMMK 2 (96% ANSI)",
        "vendor_id": "0x320F",
        "product_id": "0x5044",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater_cli",
        "layout": "96% ANSI",
        "tier": "luxqmk_enhanced",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "sidelights"]
    },
    "gmmk_gmmk2_p96_iso_via": {
        "name": "Glorious GMMK 2 (96% ISO)",
        "vendor_id": "0x320F",
        "product_id": "0x5054",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater_cli",
        "layout": "96% ISO",
        "tier": "luxqmk_enhanced",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "sidelights"]
    },
    "gmmk_gmmk2_p65_ansi_via": {
        "name": "Glorious GMMK 2 (65% ANSI)",
        "vendor_id": "0x320F",
        "product_id": "0x5045",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater_cli",
        "layout": "65% ANSI",
        "tier": "luxqmk_enhanced",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "sidelights"]
    },
    "gmmk_gmmk2_p65_iso_via": {
        "name": "Glorious GMMK 2 (65% ISO)",
        "vendor_id": "0x320F",
        "product_id": "0x5055",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater_cli",
        "layout": "65% ISO",
        "tier": "luxqmk_enhanced",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "sidelights"]
    },

    # Universal QMK & Keychron Reference Boards
    "keychron_q1_ansi_via": {
        "name": "Keychron Q1 (ANSI VIA)",
        "vendor_id": "0x3434",
        "product_id": "0x0101",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "75%",
        "tier": "luxqmk_generic",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_q2_ansi_via": {
        "name": "Keychron Q2 (ANSI VIA)",
        "vendor_id": "0x3434",
        "product_id": "0x0103",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "65%",
        "tier": "luxqmk_generic",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_q3_ansi_via": {
        "name": "Keychron Q3 (ANSI VIA)",
        "vendor_id": "0x3434",
        "product_id": "0x0105",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "80% TKL",
        "tier": "luxqmk_generic",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_v1_ansi_via": {
        "name": "Keychron V1 (ANSI VIA)",
        "vendor_id": "0x3434",
        "product_id": "0x0111",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "75%",
        "tier": "luxqmk_generic",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "dz60_via": {
        "name": "DZ60 (60% Universal VIA)",
        "vendor_id": "0x445A",
        "product_id": "0x1420",
        "mcu": "ATmega32U4",
        "flasher": "dfu-util",
        "layout": "60%",
        "tier": "luxqmk_generic",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers"]
    }
}

def calculate_sha256(filepath):
    sha = hashlib.sha256()
    with open(filepath, "rb") as f:
        while chunk := f.read(65536):
            sha.update(chunk)
    return sha.hexdigest()

def find_info_json(kb_target):
    parts = kb_target.replace("-", "/").split("_")
    for i in range(len(parts), 0, -1):
        candidate_path = os.path.join(KEYBOARDS_DIR, *parts[:i], "info.json")
        if os.path.exists(candidate_path):
            try:
                with open(candidate_path, "r", encoding="utf-8") as f:
                    return json.load(f)
            except Exception:
                pass
    return {}

def generate_catalog(artifacts_dir, output_dir, tag_version, repo_slug, base_url=None):
    os.makedirs(output_dir, exist_ok=True)
    
    # Base URL for direct binary downloads on files.luxqmk.click/firmware
    if not base_url:
        base_url = f"https://files.luxqmk.click/firmware/{tag_version}"
    else:
        base_url = base_url.rstrip("/")

    entries = []
    candidates = {}

    # Find all compiled binaries in artifacts directory and group by base model stem
    for root, _, files in os.walk(artifacts_dir):
        for file in files:
            if not file.endswith((".bin", ".hex", ".uf2")):
                continue
            
            filepath = os.path.join(root, file)
            stem = os.path.splitext(file)[0]
            if stem not in candidates:
                candidates[stem] = []
            candidates[stem].append((file, filepath))

    # For each keyboard model, pick canonical format (.bin for ARM > .uf2 for RP2040 > .hex for AVR)
    for stem, file_list in candidates.items():
        # Prefer .bin over .uf2 over .hex
        file_list.sort(key=lambda x: (
            0 if x[0].endswith(".bin") else (
                1 if x[0].endswith(".uf2") else 2
            )
        ))
        file, filepath = file_list[0]

        sha256_hash = calculate_sha256(filepath)
        file_size = os.path.getsize(filepath)

        # Metadata matching
        meta = KNOWN_BOARDS.get(stem, {})
        kb_name = meta.get("name")
        vid = meta.get("vendor_id")
        pid = meta.get("product_id")
        mcu = meta.get("mcu", "ARM Cortex / AVR")
        flasher = meta.get("flasher", "wb32-dfu-updater_cli" if "WB32" in mcu else ("dfu-util" if file.endswith((".bin", ".hex")) else "uf2"))
        layout = meta.get("layout", "Universal")
        tier = meta.get("tier", "luxqmk_generic")
        features = meta.get("features", ["nkro", "debounce", "rgb_matrix", "reactive_layers"])

        if not kb_name:
            # Fallback lookup from info.json
            info = find_info_json(stem)
            kb_name = info.get("keyboard_name", stem.replace("_", " ").title())
            if "usb" in info:
                vid = info["usb"].get("vid", vid)
                pid = info["usb"].get("pid", pid)
            if "processor" in info:
                mcu = info.get("processor", mcu)

        entry = {
            "id": stem,
            "name": kb_name,
            "filename": file,
            "version": tag_version.lstrip("v"),
            "release_tag": tag_version,
            "vendor_id": vid,
            "product_id": pid,
            "mcu": mcu,
            "flasher": flasher,
            "layout": layout,
            "tier": tier,
            "features": features,
            "file_size_bytes": file_size,
            "sha256": sha256_hash,
            "download_url": f"{base_url}/{file}",
            "latest_url": f"https://files.luxqmk.click/firmware/latest/{file}",
            "studio_url": f"https://luxqmk.click/#firmware?model={stem}"
        }
        entries.append(entry)

    # Sort entries: LuxQMK Enhanced first, then alphabetical
    entries.sort(key=lambda x: (0 if x["tier"] == "luxqmk_enhanced" else 1, x["name"]))

    # Output catalog.json
    catalog_data = {
        "version": tag_version,
        "repo": repo_slug,
        "base_url": base_url,
        "domain": "files.luxqmk.click",
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "total_keyboards": len(entries),
        "keyboards": entries
    }

    catalog_json_path = os.path.join(output_dir, "catalog.json")
    with open(catalog_json_path, "w", encoding="utf-8") as f:
        json.dump(catalog_data, f, indent=2)

    print(f"[+] Successfully generated catalog.json with {len(entries)} keyboards at {catalog_json_path}")

    # Generate Cloudflare Pages redirect assets and index.html fallback
    generate_redirect_assets(output_dir)

def generate_redirect_assets(output_dir):
    """
    Generates Cloudflare Pages _redirects, _headers, and an index.html with meta-refresh
    and JS redirection pointing to the main LuxQMK website (https://luxqmk.click/#firmware).
    """
    # 1. Cloudflare Pages _redirects (Edge-level 302 redirect for root)
    redirects_file = os.path.join(output_dir, "_redirects")
    with open(redirects_file, "w", encoding="utf-8") as f:
        f.write("/ https://luxqmk.click/#firmware 302\n")

    # 2. Cloudflare Pages _headers for CORS
    headers_file = os.path.join(output_dir, "_headers")
    with open(headers_file, "w", encoding="utf-8") as f:
        f.write("/*\n  Access-Control-Allow-Origin: *\n  Access-Control-Allow-Methods: GET, HEAD, OPTIONS\n  Access-Control-Allow-Headers: *\n")

    # 3. Fallback index.html with immediate client redirect
    out_file = os.path.join(output_dir, "index.html")
    html_content = """<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta http-equiv="refresh" content="0; url=https://luxqmk.click/#firmware">
  <link rel="canonical" href="https://luxqmk.click/#firmware">
  <title>LuxQMK Firmware Files</title>
  <script>window.location.replace("https://luxqmk.click/#firmware");</script>
  <style>
    :root {
      --bg: #08080c;
      --card: #10111a;
      --border: #1e2030;
      --text: #f1f5f9;
      --muted: #94a3b8;
      --cyan: #00f2fe;
      --purple: #bd00ff;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      background-color: var(--bg);
      color: var(--text);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      display: flex;
      align-items: center;
      justify-content: center;
      min-height: 100vh;
      padding: 1.5rem;
    }
    .card {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 16px;
      padding: 2.5rem;
      text-align: center;
      max-width: 480px;
      box-shadow: 0 20px 40px rgba(0,0,0,0.6);
    }
    h1 {
      font-size: 1.5rem;
      margin-bottom: 0.75rem;
      background: linear-gradient(135deg, var(--cyan) 0%, var(--purple) 100%);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }
    p {
      color: var(--muted);
      margin-bottom: 1.75rem;
      font-size: 0.95rem;
      line-height: 1.5;
    }
    a.btn {
      display: inline-flex;
      align-items: center;
      gap: 0.5rem;
      background: linear-gradient(135deg, var(--cyan) 0%, var(--purple) 100%);
      color: #08080c;
      font-weight: 700;
      padding: 0.75rem 1.5rem;
      border-radius: 9999px;
      text-decoration: none;
      transition: transform 0.2s, box-shadow 0.2s;
    }
    a.btn:hover {
      transform: translateY(-2px);
      box-shadow: 0 0 20px rgba(0, 242, 254, 0.4);
    }
  </style>
</head>
<body>
  <div class="card">
    <h1>Redirecting to LuxQMK Portal</h1>
    <p>For firmware downloads, WebHID flasher, and keymap customization, visit our main website.</p>
    <a class="btn" href="https://luxqmk.click/#firmware">
      Go to luxqmk.click &rarr;
    </a>
  </div>
</body>
</html>
"""
    with open(out_file, "w", encoding="utf-8") as f:
        f.write(html_content)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate LuxQMK Firmware Catalog")
    parser.add_argument("--artifacts-dir", default=ROOT_DIR, help="Directory containing compiled binaries")
    parser.add_argument("--output-dir", default=os.path.join(ROOT_DIR, "catalog_build"), help="Output directory")
    parser.add_argument("--tag", default="v0.3.1", help="Release tag version")
    parser.add_argument("--repo", default="LuxQMK/qmk_firmware", help="GitHub repo slug")
    parser.add_argument("--base-url", default="https://files.luxqmk.click/firmware", help="Base download URL for binaries")
    args = parser.parse_args()

    generate_catalog(args.artifacts_dir, args.output_dir, args.tag, args.repo, args.base_url)
