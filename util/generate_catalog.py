#!/usr/bin/env python3
"""
LuxQMK Firmware Catalog Generator & Asset Manager
Generates `catalog.json` with metadata, SHA-256 checksums, and download endpoints for `files.luxqmk.click/firmware`.
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

    # Find all compiled binaries in artifacts directory
    for root, _, files in os.walk(artifacts_dir):
        for file in files:
            if not file.endswith((".bin", ".hex", ".uf2")):
                continue
            
            filepath = os.path.join(root, file)
            stem = os.path.splitext(file)[0]
            sha256_hash = calculate_sha256(filepath)
            file_size = os.path.getsize(filepath)

            # Metadata matching
            meta = KNOWN_BOARDS.get(stem, {})
            kb_name = meta.get("name")
            vid = meta.get("vendor_id")
            pid = meta.get("product_id")
            mcu = meta.get("mcu", "ARM Cortex / AVR")
            flasher = meta.get("flasher", "dfu-util" if file.endswith((".bin", ".hex")) else "uf2")
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
                "studio_url": f"https://luxqmk.click/?firmware={base_url}/{file}&model={stem}"
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

    # Generate standalone web portal (index.html) as fallback browser UI
    generate_portal_html(entries, tag_version, output_dir)

def generate_portal_html(entries, tag, output_dir):
    out_file = os.path.join(output_dir, "index.html")
    keyboards_json = json.dumps(entries)

    html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>LuxQMK Firmware Files & Catalog — files.luxqmk.click</title>
  <meta name="description" content="Official high-performance QMK & VIA firmware downloads for LuxQMK-powered keyboards. Featuring Full NKRO, hardware debounce engines, and 60 FPS RGB streaming.">
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Plus+Jakarta+Sans:wght@400;500;600;700;800&family=JetBrains+Mono:wght@400;500;600&display=swap" rel="stylesheet">
  <style>
    :root {{
      --bg-dark: #08080c;
      --card-bg: #10111a;
      --card-border: #1e2030;
      --accent-cyan: #00f2fe;
      --accent-purple: #bd00ff;
      --text-main: #f1f5f9;
      --text-muted: #94a3b8;
      --badge-bg: rgba(0, 242, 254, 0.1);
      --badge-border: rgba(0, 242, 254, 0.3);
    }}
    * {{
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      font-family: 'Plus Jakarta Sans', -apple-system, BlinkMacSystemFont, sans-serif;
    }}
    body {{
      background-color: var(--bg-dark);
      color: var(--text-main);
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      background-image: 
        radial-gradient(circle at 15% 15%, rgba(189, 0, 255, 0.08) 0%, transparent 40%),
        radial-gradient(circle at 85% 85%, rgba(0, 242, 254, 0.08) 0%, transparent 40%);
      background-attachment: fixed;
    }}
    header {{
      border-bottom: 1px solid var(--card-border);
      background: rgba(16, 17, 26, 0.85);
      backdrop-filter: blur(12px);
      position: sticky;
      top: 0;
      z-index: 50;
    }}
    .header-inner {{
      max-width: 1400px;
      margin: 0 auto;
      padding: 1rem 2rem;
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 1rem;
    }}
    .brand {{
      display: flex;
      align-items: center;
      gap: 0.75rem;
      text-decoration: none;
      color: var(--text-main);
    }}
    .brand-logo {{
      width: 36px;
      height: 36px;
      background: linear-gradient(135deg, var(--accent-cyan), var(--accent-purple));
      border-radius: 10px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-weight: 800;
      font-size: 1.2rem;
      color: #000;
      box-shadow: 0 0 20px rgba(0, 242, 254, 0.4);
    }}
    .brand h1 {{
      font-size: 1.25rem;
      font-weight: 700;
      letter-spacing: -0.5px;
    }}
    .brand-badge {{
      background: var(--badge-bg);
      border: 1px solid var(--badge-border);
      color: var(--accent-cyan);
      font-size: 0.75rem;
      font-weight: 600;
      padding: 0.2rem 0.5rem;
      border-radius: 20px;
      margin-left: 0.5rem;
    }}
    .nav-links {{
      display: flex;
      align-items: center;
      gap: 0.85rem;
    }}
    .nav-btn {{
      padding: 0.55rem 1.1rem;
      border-radius: 8px;
      font-size: 0.875rem;
      font-weight: 600;
      text-decoration: none;
      transition: all 0.2s ease;
      display: inline-flex;
      align-items: center;
      gap: 0.4rem;
    }}
    .nav-btn-secondary {{
      background: rgba(255, 255, 255, 0.05);
      color: var(--text-muted);
      border: 1px solid var(--card-border);
    }}
    .nav-btn-secondary:hover {{
      color: var(--text-main);
      background: rgba(255, 255, 255, 0.1);
    }}
    .nav-btn-primary {{
      background: linear-gradient(135deg, var(--accent-cyan), var(--accent-purple));
      color: #000;
      font-weight: 700;
      box-shadow: 0 0 15px rgba(0, 242, 254, 0.3);
    }}
    .nav-btn-primary:hover {{
      box-shadow: 0 0 25px rgba(189, 0, 255, 0.5);
      transform: translateY(-1px);
    }}
    main {{
      flex: 1;
      max-width: 1400px;
      width: 100%;
      margin: 0 auto;
      padding: 2.5rem 2rem;
    }}
    .hero {{
      text-align: center;
      margin-bottom: 3rem;
    }}
    .hero h2 {{
      font-size: 2.5rem;
      font-weight: 800;
      letter-spacing: -1px;
      margin-bottom: 0.8rem;
      background: linear-gradient(135deg, #fff 40%, var(--accent-cyan));
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }}
    .hero p {{
      color: var(--text-muted);
      max-width: 650px;
      margin: 0 auto 2rem;
      line-height: 1.6;
      font-size: 1.05rem;
    }}
    .search-container {{
      max-width: 680px;
      margin: 0 auto 1.5rem;
      position: relative;
    }}
    .search-input {{
      width: 100%;
      padding: 1rem 1.25rem 1rem 3.2rem;
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: 12px;
      color: var(--text-main);
      font-size: 1rem;
      outline: none;
      box-shadow: 0 8px 30px rgba(0,0,0,0.3);
      transition: all 0.2s ease;
    }}
    .search-input:focus {{
      border-color: var(--accent-cyan);
      box-shadow: 0 0 20px rgba(0, 242, 254, 0.25);
    }}
    .search-icon {{
      position: absolute;
      left: 1.2rem;
      top: 50%;
      transform: translateY(-50%);
      color: var(--text-muted);
      pointer-events: none;
    }}
    .filter-pills {{
      display: flex;
      justify-content: center;
      flex-wrap: wrap;
      gap: 0.5rem;
      margin-bottom: 2.5rem;
    }}
    .pill {{
      background: rgba(255, 255, 255, 0.04);
      border: 1px solid var(--card-border);
      padding: 0.45rem 1rem;
      border-radius: 20px;
      font-size: 0.85rem;
      color: var(--text-muted);
      cursor: pointer;
      transition: all 0.15s ease;
    }}
    .pill:hover, .pill.active {{
      background: rgba(0, 242, 254, 0.15);
      border-color: var(--accent-cyan);
      color: var(--accent-cyan);
    }}
    .grid-container {{
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(320px, 1fr));
      gap: 1.5rem;
    }}
    .kb-card {{
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: 14px;
      padding: 1.5rem;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
      transition: all 0.2s ease;
      position: relative;
      overflow: hidden;
    }}
    .kb-card:hover {{
      transform: translateY(-4px);
      border-color: rgba(0, 242, 254, 0.4);
      box-shadow: 0 12px 30px rgba(0, 0, 0, 0.4), 0 0 20px rgba(0, 242, 254, 0.1);
    }}
    .kb-card-header {{
      display: flex;
      align-items: flex-start;
      justify-content: space-between;
      margin-bottom: 1rem;
    }}
    .kb-card-title {{
      font-size: 1.15rem;
      font-weight: 700;
      color: var(--text-main);
      line-height: 1.3;
    }}
    .badge-tier {{
      font-size: 0.7rem;
      font-weight: 700;
      text-transform: uppercase;
      padding: 0.2rem 0.5rem;
      border-radius: 6px;
      white-space: nowrap;
    }}
    .tier-enhanced {{
      background: rgba(189, 0, 255, 0.15);
      border: 1px solid rgba(189, 0, 255, 0.4);
      color: #e084fc;
    }}
    .tier-generic {{
      background: rgba(255, 255, 255, 0.05);
      border: 1px solid rgba(255, 255, 255, 0.15);
      color: var(--text-muted);
    }}
    .specs-grid {{
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 0.6rem;
      margin-bottom: 1.25rem;
      font-size: 0.8rem;
    }}
    .spec-item {{
      display: flex;
      flex-direction: column;
      background: rgba(0, 0, 0, 0.25);
      padding: 0.5rem 0.65rem;
      border-radius: 6px;
      border: 1px solid rgba(255, 255, 255, 0.03);
    }}
    .spec-item span:first-child {{
      color: var(--text-muted);
      font-size: 0.7rem;
      text-transform: uppercase;
      margin-bottom: 0.15rem;
    }}
    .spec-item span:last-child {{
      color: var(--text-main);
      font-weight: 600;
      font-family: 'JetBrains Mono', monospace;
    }}
    .feature-tags {{
      display: flex;
      flex-wrap: wrap;
      gap: 0.35rem;
      margin-bottom: 1.5rem;
    }}
    .feature-tag {{
      background: rgba(0, 242, 254, 0.06);
      border: 1px solid rgba(0, 242, 254, 0.2);
      color: var(--accent-cyan);
      font-size: 0.7rem;
      padding: 0.15rem 0.45rem;
      border-radius: 4px;
      text-transform: uppercase;
      font-weight: 600;
    }}
    .card-actions {{
      display: grid;
      grid-template-columns: 1fr auto;
      gap: 0.5rem;
      margin-top: auto;
    }}
    .btn-download {{
      background: linear-gradient(135deg, rgba(0, 242, 254, 0.15), rgba(189, 0, 255, 0.15));
      border: 1px solid rgba(0, 242, 254, 0.4);
      color: var(--text-main);
      font-weight: 700;
      font-size: 0.85rem;
      padding: 0.6rem 1rem;
      border-radius: 8px;
      text-decoration: none;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 0.45rem;
      transition: all 0.15s ease;
    }}
    .btn-download:hover {{
      background: linear-gradient(135deg, var(--accent-cyan), var(--accent-purple));
      color: #000;
      box-shadow: 0 0 15px rgba(0, 242, 254, 0.4);
    }}
    .btn-studio {{
      background: rgba(189, 0, 255, 0.1);
      border: 1px solid rgba(189, 0, 255, 0.3);
      color: #e084fc;
      text-decoration: none;
      font-weight: 600;
      font-size: 0.85rem;
      padding: 0.6rem 0.8rem;
      border-radius: 8px;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 0.35rem;
      transition: all 0.15s ease;
    }}
    .btn-studio:hover {{
      background: rgba(189, 0, 255, 0.25);
      box-shadow: 0 0 14px rgba(189, 0, 255, 0.4);
    }}
    footer {{
      margin-top: auto;
      border-top: 1px solid var(--card-border);
      padding: 2rem;
      text-align: center;
      color: var(--text-muted);
      font-size: 0.85rem;
    }}
  </style>
</head>
<body>

  <header>
    <div class="header-inner">
      <a href="https://files.luxqmk.click" class="brand">
        <div class="brand-logo">L</div>
        <div>
          <h1>LuxQMK Files & Assets</h1>
        </div>
        <span class="brand-badge">{tag}</span>
      </a>

      <div class="nav-links">
        <a href="catalog.json" class="nav-btn nav-btn-secondary" target="_blank">
          <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"></path><polyline points="14 2 14 8 20 8"></polyline><line x1="16" y1="13" x2="8" y2="13"></line><line x1="16" y1="17" x2="8" y2="17"></line><polyline points="10 9 9 9 8 9"></polyline></svg>
          catalog.json
        </a>
        <a href="https://luxqmk.click" class="nav-btn nav-btn-primary" target="_blank">
          Open LuxQMK Studio
          <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5"><polyline points="9 18 15 12 9 6"></polyline></svg>
        </a>
      </div>
    </div>
  </header>

  <main>
    <section class="hero">
      <h2>LuxQMK Firmware & Asset CDN</h2>
      <p>High-speed global binary downloads for LuxQMK-powered keyboards and applications. Serving <code>files.luxqmk.click</code>.</p>

      <div class="search-container">
        <svg class="search-icon" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><circle cx="11" cy="11" r="8"></circle><line x1="21" y1="21" x2="16.65" y2="16.65"></line></svg>
        <input type="text" id="searchInput" class="search-input" placeholder="Search by model, brand, MCU, or layout (e.g. GMMK 3, Keychron, WB32, 75%)...">
      </div>

      <div class="filter-pills">
        <button class="pill active" data-filter="all">All Keyboards</button>
        <button class="pill" data-filter="gmmk">Glorious (GMMK)</button>
        <button class="pill" data-filter="keychron">Keychron</button>
        <button class="pill" data-filter="100%">100% Full</button>
        <button class="pill" data-filter="75%">75%</button>
        <button class="pill" data-filter="65%">65%</button>
        <button class="pill" data-filter="wb32">WB32 MCU</button>
        <button class="pill" data-filter="stm32">STM32 MCU</button>
      </div>
    </section>

    <section class="grid-container" id="keyboardsGrid">
      <!-- Injected by JavaScript -->
    </section>
  </main>

  <footer>
    <p>Powered by <strong>LuxQMK</strong> &bull; <a href="https://files.luxqmk.click/firmware/catalog.json" style="color: var(--accent-cyan); text-decoration: none;">files.luxqmk.click</a> &bull; Open Source GNU GPLv3 &bull; <a href="https://github.com/LuxQMK/qmk_firmware" style="color: var(--accent-purple); text-decoration: none;">GitHub Repository</a></p>
  </footer>

  <script>
    const keyboards = {keyboards_json};
    const grid = document.getElementById('keyboardsGrid');
    const searchInput = document.getElementById('searchInput');
    const pills = document.querySelectorAll('.pill');

    let currentFilter = 'all';

    function renderKeyboards(items) {{
      if (items.length === 0) {{
        grid.innerHTML = '<div style="grid-column: 1/-1; text-align: center; padding: 4rem; color: var(--text-muted);">No keyboards found matching your search.</div>';
        return;
      }}

      grid.innerHTML = items.map(kb => `
        <div class="kb-card">
          <div>
            <div class="kb-card-header">
              <h3 class="kb-card-title">${{kb.name}}</h3>
              <span class="badge-tier ${{kb.tier === 'luxqmk_enhanced' ? 'tier-enhanced' : 'tier-generic'}}">
                ${{kb.tier === 'luxqmk_enhanced' ? 'LuxQMK Enhanced' : 'Universal'}}
              </span>
            </div>

            <div class="specs-grid">
              <div class="spec-item">
                <span>Microcontroller</span>
                <span>${{kb.mcu}}</span>
              </div>
              <div class="spec-item">
                <span>Layout / Form</span>
                <span>${{kb.layout}}</span>
              </div>
              <div class="spec-item">
                <span>USB VID / PID</span>
                <span>${{kb.vendor_id || 'VIA'}} : ${{kb.product_id || 'HID'}}</span>
              </div>
              <div class="spec-item">
                <span>Flasher Tool</span>
                <span>${{kb.flasher}}</span>
              </div>
            </div>

            <div class="feature-tags">
              ${{kb.features.map(f => `<span class="feature-tag">${{f.replace('_', ' ')}}</span>`).join('')}}
            </div>
          </div>

          <div class="card-actions">
            <a href="${{kb.download_url}}" class="btn-download" download>
              <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"></path><polyline points="7 10 12 15 17 10"></polyline><line x1="12" y1="15" x2="12" y2="3"></line></svg>
              Download .bin
            </a>
            <a href="https://luxqmk.click" target="_blank" class="btn-studio" title="Open in LuxQMK Studio">
              Studio
              <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><polygon points="5 3 19 12 5 21 5 3"></polygon></svg>
            </a>
          </div>
        </div>
      `).join('');
    }}

    function filterAndRender() {{
      const query = searchInput.value.toLowerCase().trim();
      const filtered = keyboards.filter(kb => {{
        const matchesQuery = !query || 
          kb.name.toLowerCase().includes(query) || 
          kb.id.toLowerCase().includes(query) || 
          kb.mcu.toLowerCase().includes(query) || 
          kb.layout.toLowerCase().includes(query);

        let matchesPill = true;
        if (currentFilter === 'gmmk') matchesPill = kb.id.includes('gmmk');
        else if (currentFilter === 'keychron') matchesPill = kb.id.includes('keychron');
        else if (currentFilter === '100%') matchesPill = kb.layout.includes('100%');
        else if (currentFilter === '75%') matchesPill = kb.layout.includes('75%');
        else if (currentFilter === '65%') matchesPill = kb.layout.includes('65%');
        else if (currentFilter === 'wb32') matchesPill = kb.mcu.toLowerCase().includes('wb32');
        else if (currentFilter === 'stm32') matchesPill = kb.mcu.toLowerCase().includes('stm32');

        return matchesQuery && matchesPill;
      }});

      renderKeyboards(filtered);
    }}

    searchInput.addEventListener('input', filterAndRender);

    pills.forEach(pill => {{
      pill.addEventListener('click', () => {{
        pills.forEach(p => p.classList.remove('active'));
        pill.classList.add('active');
        currentFilter = pill.getAttribute('data-filter');
        filterAndRender();
      }});
    }});

    renderKeyboards(keyboards);
  </script>
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
