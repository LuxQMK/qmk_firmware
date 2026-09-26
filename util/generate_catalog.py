#!/usr/bin/env python3
"""
LuxQMK Firmware Catalog & Portal Generator
Generates `catalog.json` and a standalone modern web interface for `browse.luxqmk.click`.
Scans compiled firmware artifacts (.bin, .hex, .uf2), matches metadata from info.json,
and generates SHA-256 checksums and direct download endpoints.
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
    "gmmk_gmmk3_p100_ansi_via": {
        "name": "Glorious GMMK 3 (100% ANSI)",
        "vendor_id": "0x504B",
        "product_id": "0x320F",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater",
        "layout": "100%",
        "tier": "luxqmk_enhanced",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "logo_led", "win_lock_led", "sidelights", "encoder"]
    },
    "gmmk_gmmk3_p75_ansi_via": {
        "name": "Glorious GMMK 3 (75% ANSI)",
        "vendor_id": "0x504B",
        "product_id": "0x320E",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater",
        "layout": "75%",
        "tier": "luxqmk_enhanced",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "logo_led", "win_lock_led", "sidelights", "encoder"]
    },
    "gmmk_gmmk3_p65_ansi_via": {
        "name": "Glorious GMMK 3 (65% ANSI)",
        "vendor_id": "0x504B",
        "product_id": "0x320D",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater",
        "layout": "65%",
        "tier": "luxqmk_enhanced",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "logo_led", "win_lock_led", "sidelights", "encoder"]
    },
    "gmmk_gmmk2_p96_ansi_via": {
        "name": "Glorious GMMK 2 (96% ANSI)",
        "vendor_id": "0x320F",
        "product_id": "0x5044",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater",
        "layout": "96%",
        "tier": "luxqmk_enhanced",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "sidelights"]
    },
    "gmmk_gmmk2_p65_ansi_via": {
        "name": "Glorious GMMK 2 (65% ANSI)",
        "vendor_id": "0x320F",
        "product_id": "0x5045",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater",
        "layout": "65%",
        "tier": "luxqmk_enhanced",
        "features": ["nkro", "debounce", "rgb_matrix", "reactive_layers", "sidelights"]
    },
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

def generate_catalog(artifacts_dir, output_dir, tag_version, repo_slug):
    os.makedirs(output_dir, exist_ok=True)
    base_download_url = f"https://github.com/{repo_slug}/releases/download/{tag_version}"

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
            flasher = meta.get("flasher", "dfu-util" if file.endswith(".bin") or file.endswith(".hex") else "uf2")
            layout = meta.get("layout", "Universal")
            tier = meta.get("tier", "luxqmk_generic")
            features = meta.get("features", ["nkro", "debounce", "rgb_matrix", "reactive_layers"])

            if not kb_name:
                # Try fallback lookup from info.json
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
                "download_url": f"{base_download_url}/{file}",
                "studio_url": f"https://luxqmk.click/?firmware={base_download_url}/{file}&model={stem}"
            }
            entries.append(entry)

    # Sort entries: LuxQMK Enhanced first, then alphabetical
    entries.sort(key=lambda x: (0 if x["tier"] == "luxqmk_enhanced" else 1, x["name"]))

    catalog_data = {
        "version": tag_version.lstrip("v"),
        "release_tag": tag_version,
        "updated_at": datetime.now(timezone.utc).isoformat(),
        "portal_url": "https://browse.luxqmk.click",
        "api_url": "https://browse.luxqmk.click/catalog.json",
        "github_repo": f"https://github.com/{repo_slug}",
        "total_keyboards": len(entries),
        "keyboards": entries
    }

    # Save JSON manifest
    catalog_json_path = os.path.join(output_dir, "catalog.json")
    with open(catalog_json_path, "w", encoding="utf-8") as f:
        json.dump(catalog_data, f, indent=2)
    print(f"[OK] Generated {catalog_json_path} with {len(entries)} keyboard entries.")

    # Generate HTML Website for browse.luxqmk.click
    html_path = os.path.join(output_dir, "index.html")
    generate_html_portal(catalog_data, html_path)
    print(f"[OK] Generated web portal at {html_path}")

def generate_html_portal(catalog, out_file):
    keyboards_json = json.dumps(catalog["keyboards"])
    tag = catalog["release_tag"]

    html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>LuxQMK Firmware Catalog — browse.luxqmk.click</title>
  <meta name="description" content="Official high-performance LuxQMK & QMK firmware repository. 1000Hz polling, Full NKRO, Dual-Layer Reactive RGB, Dynamic Debounce for all keyboards.">
  <link rel="icon" type="image/svg+xml" href="https://luxqmk.click/assets/logo.svg">
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700;800&family=JetBrains+Mono:wght@400;500;600&display=swap" rel="stylesheet">
  <style>
    :root {{
      --bg-dark: #080b11;
      --card-bg: rgba(17, 24, 39, 0.85);
      --card-border: rgba(255, 255, 255, 0.08);
      --accent-cyan: #00f0ff;
      --accent-magenta: #bd00ff;
      --accent-green: #00ff88;
      --accent-amber: #ffaa00;
      --text-main: #f8fafc;
      --text-muted: #94a3b8;
      --font-sans: 'Inter', system-ui, sans-serif;
      --font-mono: 'JetBrains Mono', monospace;
    }}
    * {{ box-sizing: border-box; margin: 0; padding: 0; }}
    body {{
      background-color: var(--bg-dark);
      color: var(--text-main);
      font-family: var(--font-sans);
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      background-image: radial-gradient(circle at 50% 0%, rgba(0, 240, 255, 0.07) 0%, transparent 60%);
    }}
    header {{
      border-bottom: 1px solid var(--card-border);
      background: rgba(8, 11, 17, 0.85);
      backdrop-filter: blur(16px);
      position: sticky;
      top: 0;
      z-index: 50;
      padding: 1rem 2rem;
    }}
    .header-inner {{
      max-width: 1300px;
      margin: 0 auto;
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 1.5rem;
      flex-wrap: wrap;
    }}
    .brand {{
      display: flex;
      align-items: center;
      gap: 0.85rem;
      text-decoration: none;
      color: var(--text-main);
    }}
    .brand-logo {{
      width: 36px;
      height: 36px;
      border-radius: 8px;
      background: linear-gradient(135deg, var(--accent-cyan), var(--accent-magenta));
      display: flex;
      align-items: center;
      justify-content: center;
      font-weight: 800;
      font-size: 1.1rem;
      color: #000;
    }}
    .brand h1 {{
      font-size: 1.25rem;
      font-weight: 800;
      letter-spacing: -0.02em;
    }}
    .brand-badge {{
      background: rgba(0, 240, 255, 0.15);
      color: var(--accent-cyan);
      border: 1px solid rgba(0, 240, 255, 0.3);
      padding: 0.2rem 0.6rem;
      border-radius: 9999px;
      font-size: 0.75rem;
      font-weight: 700;
    }}
    .nav-links {{
      display: flex;
      align-items: center;
      gap: 1rem;
    }}
    .nav-btn {{
      padding: 0.5rem 1rem;
      border-radius: 8px;
      font-size: 0.85rem;
      font-weight: 600;
      text-decoration: none;
      transition: all 0.15s ease;
      display: inline-flex;
      align-items: center;
      gap: 0.4rem;
    }}
    .nav-btn-primary {{
      background: linear-gradient(135deg, var(--accent-cyan), #0099ff);
      color: #000;
      font-weight: 700;
    }}
    .nav-btn-primary:hover {{
      box-shadow: 0 0 16px rgba(0, 240, 255, 0.5);
      transform: translateY(-1px);
    }}
    .nav-btn-secondary {{
      background: rgba(255, 255, 255, 0.06);
      color: var(--text-main);
      border: 1px solid var(--card-border);
    }}
    .nav-btn-secondary:hover {{
      background: rgba(255, 255, 255, 0.12);
    }}
    .hero {{
      max-width: 1300px;
      margin: 2.5rem auto 1.5rem auto;
      padding: 0 2rem;
      text-align: center;
    }}
    .hero h2 {{
      font-size: 2.2rem;
      font-weight: 800;
      margin-bottom: 0.6rem;
      letter-spacing: -0.03em;
      background: linear-gradient(135deg, #ffffff 40%, var(--text-muted));
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }}
    .hero p {{
      color: var(--text-muted);
      font-size: 1.05rem;
      max-width: 720px;
      margin: 0 auto 1.8rem auto;
      line-height: 1.6;
    }}
    .search-container {{
      max-width: 800px;
      margin: 0 auto 2rem auto;
      position: relative;
    }}
    .search-input {{
      width: 100%;
      padding: 1rem 1.25rem 1rem 3rem;
      background: rgba(17, 24, 39, 0.95);
      border: 1.5px solid var(--card-border);
      border-radius: 14px;
      color: #fff;
      font-size: 1.05rem;
      outline: none;
      transition: border-color 0.2s, box-shadow 0.2s;
    }}
    .search-input:focus {{
      border-color: var(--accent-cyan);
      box-shadow: 0 0 20px rgba(0, 240, 255, 0.25);
    }}
    .search-icon {{
      position: absolute;
      left: 1.1rem;
      top: 50%;
      transform: translateY(-50%);
      color: var(--text-muted);
    }}
    .filter-pills {{
      display: flex;
      justify-content: center;
      gap: 0.5rem;
      flex-wrap: wrap;
      margin-bottom: 2.5rem;
    }}
    .pill {{
      background: rgba(255, 255, 255, 0.05);
      border: 1px solid var(--card-border);
      color: var(--text-muted);
      padding: 0.4rem 0.9rem;
      border-radius: 20px;
      font-size: 0.82rem;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.15s ease;
    }}
    .pill.active, .pill:hover {{
      background: rgba(0, 240, 255, 0.15);
      border-color: var(--accent-cyan);
      color: var(--accent-cyan);
    }}
    .grid-container {{
      max-width: 1300px;
      margin: 0 auto;
      padding: 0 2rem 4rem 2rem;
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(370px, 1fr));
      gap: 1.5rem;
    }}
    .kb-card {{
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: 16px;
      padding: 1.4rem;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
      transition: transform 0.2s ease, border-color 0.2s ease, box-shadow 0.2s ease;
      position: relative;
      overflow: hidden;
    }}
    .kb-card:hover {{
      transform: translateY(-3px);
      border-color: rgba(0, 240, 255, 0.4);
      box-shadow: 0 8px 30px rgba(0, 0, 0, 0.5);
    }}
    .kb-card-header {{
      display: flex;
      justify-content: space-between;
      align-items: flex-start;
      margin-bottom: 0.8rem;
    }}
    .kb-card-title {{
      font-size: 1.15rem;
      font-weight: 700;
      line-height: 1.3;
      color: #fff;
    }}
    .badge-tier {{
      font-size: 0.7rem;
      font-weight: 700;
      padding: 0.2rem 0.55rem;
      border-radius: 6px;
      text-transform: uppercase;
      letter-spacing: 0.04em;
    }}
    .tier-enhanced {{
      background: rgba(0, 240, 255, 0.15);
      color: var(--accent-cyan);
      border: 1px solid rgba(0, 240, 255, 0.4);
    }}
    .tier-generic {{
      background: rgba(255, 255, 255, 0.08);
      color: var(--text-muted);
      border: 1px solid var(--card-border);
    }}
    .specs-grid {{
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 0.5rem;
      margin: 1rem 0;
      font-size: 0.82rem;
      background: rgba(0, 0, 0, 0.25);
      padding: 0.75rem;
      border-radius: 8px;
    }}
    .spec-item span:first-child {{
      color: var(--text-muted);
      display: block;
      font-size: 0.72rem;
      text-transform: uppercase;
    }}
    .spec-item span:last-child {{
      font-family: var(--font-mono);
      font-weight: 600;
      color: var(--text-main);
    }}
    .feature-tags {{
      display: flex;
      gap: 0.4rem;
      flex-wrap: wrap;
      margin-bottom: 1.25rem;
    }}
    .feature-tag {{
      background: rgba(255, 255, 255, 0.04);
      border: 1px solid var(--card-border);
      font-size: 0.7rem;
      color: var(--text-muted);
      padding: 0.15rem 0.45rem;
      border-radius: 4px;
    }}
    .card-actions {{
      display: flex;
      gap: 0.6rem;
    }}
    .btn-download {{
      flex: 1;
      background: linear-gradient(135deg, var(--accent-cyan), #0099ff);
      color: #000;
      text-decoration: none;
      font-weight: 700;
      font-size: 0.85rem;
      padding: 0.6rem 0.8rem;
      border-radius: 8px;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 0.4rem;
      transition: all 0.15s ease;
    }}
    .btn-download:hover {{
      box-shadow: 0 0 14px rgba(0, 240, 255, 0.4);
    }}
    .btn-studio {{
      background: rgba(189, 0, 255, 0.15);
      color: #d884ff;
      border: 1px solid rgba(189, 0, 255, 0.4);
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
      <a href="https://browse.luxqmk.click" class="brand">
        <div class="brand-logo">L</div>
        <div>
          <h1>LuxQMK Firmware Catalog</h1>
        </div>
        <span class="brand-badge">{tag}</span>
      </a>

      <div class="nav-links">
        <a href="catalog.json" class="nav-btn nav-btn-secondary" target="_blank">
          <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"></path><polyline points="14 2 14 8 20 8"></polyline><line x1="16" y1="13" x2="8" y2="13"></line><line x1="16" y1="17" x2="8" y2="17"></line><polyline points="10 9 9 9 8 9"></polyline></svg>
          API JSON
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
      <h2>High-Performance QMK & VIA Firmware</h2>
      <p>Instant downloads for keyboards powered by LuxQMK. Featuring Full NKRO, hardware debounce engines, 60 FPS RGB matrix streaming, and custom EEPROM memory.</p>

      <div class="search-container">
        <svg class="search-icon" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><circle cx="11" cy="11" r="8"></circle><line x1="21" y1="21" x2="16.65" y2="16.65"></line></svg>
        <input type="text" id="searchInput" class="search-input" placeholder="Search by model, brand, processor, or layout (e.g. GMMK 3, Keychron, WB32, 75%)...">
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
    <p>Powered by <strong>LuxQMK</strong> &bull; Open Source GNU GPLv3 &bull; <a href="https://github.com/LuxQMK/qmk_firmware" style="color: var(--accent-cyan); text-decoration: none;">GitHub Repository</a></p>
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
                ${{kb.tier === 'luxqmk_enhanced' ? 'LuxQMK Tier 1' : 'Universal'}}
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
    args = parser.parse_args()

    generate_catalog(args.artifacts_dir, args.output_dir, args.tag, args.repo)
