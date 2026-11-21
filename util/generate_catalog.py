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
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "logo_led", "win_lock_led", "sidelights", "encoder"]
    },
    "gmmk_gmmk3_p100_iso_via": {
        "name": "Glorious GMMK 3 (100% ISO)",
        "vendor_id": "0x504B",
        "product_id": "0x321F",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater_cli",
        "layout": "100% Full ISO",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "logo_led", "win_lock_led", "sidelights", "encoder"]
    },
    "gmmk_gmmk3_p75_ansi_via": {
        "name": "Glorious GMMK 3 (75% ANSI)",
        "vendor_id": "0x504B",
        "product_id": "0x320E",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater_cli",
        "layout": "75% Compact ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "logo_led", "win_lock_led", "sidelights", "encoder"]
    },
    "gmmk_gmmk3_p75_iso_via": {
        "name": "Glorious GMMK 3 (75% ISO)",
        "vendor_id": "0x504B",
        "product_id": "0x321E",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater_cli",
        "layout": "75% Compact ISO",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "logo_led", "win_lock_led", "sidelights", "encoder"]
    },
    "gmmk_gmmk3_p65_ansi_via": {
        "name": "Glorious GMMK 3 (65% ANSI)",
        "vendor_id": "0x504B",
        "product_id": "0x320D",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater_cli",
        "layout": "65% Compact ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "logo_led", "win_lock_led", "sidelights", "encoder"]
    },
    "gmmk_gmmk3_p65_iso_via": {
        "name": "Glorious GMMK 3 (65% ISO)",
        "vendor_id": "0x504B",
        "product_id": "0x321D",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater_cli",
        "layout": "65% Compact ISO",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "logo_led", "win_lock_led", "sidelights", "encoder"]
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
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "sidelights"]
    },
    "gmmk_gmmk2_p96_iso_via": {
        "name": "Glorious GMMK 2 (96% ISO)",
        "vendor_id": "0x320F",
        "product_id": "0x5054",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater_cli",
        "layout": "96% ISO",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "sidelights"]
    },
    "gmmk_gmmk2_p65_ansi_via": {
        "name": "Glorious GMMK 2 (65% ANSI)",
        "vendor_id": "0x320F",
        "product_id": "0x5045",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater_cli",
        "layout": "65% ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "sidelights"]
    },
    "gmmk_gmmk2_p65_iso_via": {
        "name": "Glorious GMMK 2 (65% ISO)",
        "vendor_id": "0x320F",
        "product_id": "0x5055",
        "mcu": "WB32F3G71xx",
        "flasher": "wb32-dfu-updater_cli",
        "layout": "65% ISO",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "sidelights"]
    },

    # Keychron V Series (Tier 1 Enhanced)
    "keychron_v1_ansi_encoder_default": {
        "name": "Keychron V1 (75% ANSI Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0311",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "75% ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_v1_iso_encoder_default": {
        "name": "Keychron V1 (75% ISO Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0313",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "75% ISO",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_v2_ansi_encoder_default": {
        "name": "Keychron V2 (65% ANSI Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0321",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "65% ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_v2_iso_encoder_default": {
        "name": "Keychron V2 (65% ISO Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0323",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "65% ISO",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_v3_ansi_encoder_default": {
        "name": "Keychron V3 (80% TKL ANSI Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0331",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "80% TKL ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_v3_iso_encoder_default": {
        "name": "Keychron V3 (80% TKL ISO Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0333",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "80% TKL ISO",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_v4_ansi_default": {
        "name": "Keychron V4 (60% ANSI)",
        "vendor_id": "0x3434",
        "product_id": "0x0340",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "60% ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers"]
    },
    "keychron_v4_iso_default": {
        "name": "Keychron V4 (60% ISO)",
        "vendor_id": "0x3434",
        "product_id": "0x0342",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "60% ISO",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers"]
    },
    "keychron_v5_ansi_encoder_default": {
        "name": "Keychron V5 (1800 Compact ANSI Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0351",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "96% ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_v5_iso_encoder_default": {
        "name": "Keychron V5 (1800 Compact ISO Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0353",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "96% ISO",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_v6_ansi_encoder_default": {
        "name": "Keychron V6 (100% Full ANSI Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0361",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "100% Full ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_v6_iso_encoder_default": {
        "name": "Keychron V6 (100% Full ISO Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0363",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "100% Full ISO",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_v10_ansi_encoder_default": {
        "name": "Keychron V10 (Alice 75% ANSI Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x03A1",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "75% Alice ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_v10_iso_encoder_default": {
        "name": "Keychron V10 (Alice 75% ISO Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x03A3",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "75% Alice ISO",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },

    # Keychron Q Series (Tier 1 Enhanced)
    "keychron_q1v1_ansi_encoder_default": {
        "name": "Keychron Q1 v1 (75% ANSI Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0103",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "75% ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_q1v1_iso_encoder_default": {
        "name": "Keychron Q1 v1 (75% ISO Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0105",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "75% ISO",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_q1v2_ansi_encoder_default": {
        "name": "Keychron Q1 v2 (75% ANSI Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0107",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "75% ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_q1v2_iso_encoder_default": {
        "name": "Keychron Q1 v2 (75% ISO Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0109",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "75% ISO",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_q2_ansi_encoder_default": {
        "name": "Keychron Q2 (65% ANSI Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0111",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "65% ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_q2_iso_encoder_default": {
        "name": "Keychron Q2 (65% ISO Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0113",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "65% ISO",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_q3_ansi_encoder_default": {
        "name": "Keychron Q3 (80% TKL ANSI Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0121",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "80% TKL ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_q3_iso_encoder_default": {
        "name": "Keychron Q3 (80% TKL ISO Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0123",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "80% TKL ISO",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_q4_ansi_v1_default": {
        "name": "Keychron Q4 (60% ANSI)",
        "vendor_id": "0x3434",
        "product_id": "0x0140",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "60% ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers"]
    },
    "keychron_q5_ansi_encoder_default": {
        "name": "Keychron Q5 (1800 Compact ANSI Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0151",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "96% ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_q5_iso_encoder_default": {
        "name": "Keychron Q5 (1800 Compact ISO Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0153",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "96% ISO",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_q6_ansi_encoder_default": {
        "name": "Keychron Q6 (100% Full ANSI Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0161",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "100% Full ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_q6_iso_encoder_default": {
        "name": "Keychron Q6 (100% Full ISO Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x0163",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "100% Full ISO",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_q10_ansi_encoder_default": {
        "name": "Keychron Q10 (Alice 75% ANSI Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x01A1",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "75% Alice ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_q11_ansi_encoder_default": {
        "name": "Keychron Q11 (Split 75% ANSI Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x01B1",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "75% Split ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },
    "keychron_q12_ansi_encoder_default": {
        "name": "Keychron Q12 (Southpaw 100% ANSI Encoder)",
        "vendor_id": "0x3434",
        "product_id": "0x01C1",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "100% Southpaw ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers", "encoder"]
    },

    # Keychron C Pro Series (Tier 1 Enhanced)
    "keychron_c1_pro_ansi_rgb_default": {
        "name": "Keychron C1 Pro (80% TKL ANSI RGB)",
        "vendor_id": "0x3434",
        "product_id": "0x0510",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "80% TKL ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers"]
    },
    "keychron_c2_pro_ansi_rgb_default": {
        "name": "Keychron C2 Pro (100% Full ANSI RGB)",
        "vendor_id": "0x3434",
        "product_id": "0x0520",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "100% Full ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers"]
    },
    "keychron_c3_pro_ansi_rgb_default": {
        "name": "Keychron C3 Pro (80% TKL ANSI RGB)",
        "vendor_id": "0x3434",
        "product_id": "0x0530",
        "mcu": "STM32L432",
        "flasher": "dfu-util",
        "layout": "80% TKL ANSI",
        "tier": "luxqmk_enhanced",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers"]
    },
    "dz60_via": {
        "name": "DZ60 (60% Universal VIA)",
        "vendor_id": "0x445A",
        "product_id": "0x1420",
        "mcu": "ATmega32U4",
        "flasher": "dfu-util",
        "layout": "60%",
        "tier": "luxqmk_generic",
        "via": True,
        "features": ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers"]
    }
}

def calculate_sha256(filepath):
    sha = hashlib.sha256()
    with open(filepath, "rb") as f:
        while chunk := f.read(65536):
            sha.update(chunk)
    return sha.hexdigest()

def build_keyboard_lookup():
    """
    Pre-indexes normalized keyboard path stems to their directory paths for fast O(1) matching.
    """
    lookup = {}
    for root, _, _ in os.walk(KEYBOARDS_DIR):
        rel = os.path.relpath(root, KEYBOARDS_DIR)
        normalized = rel.replace(os.sep, "_").replace("/", "_")
        lookup[normalized] = root
    return lookup

def parse_rules_mk(filepath):
    data = {}
    if not os.path.isfile(filepath):
        return data
    with open(filepath, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue
            k, v = line.split("=", 1)
            k = k.replace("?", "").strip()
            v = v.strip()
            data[k] = v
    return data

def find_keyboard_metadata(stem, kb_lookup, artifacts_meta=None):
    """
    Deeply inspects keyboard directory hierarchy (info.json, keyboard.json, rules.mk)
    to extract hardware profile, MCU, flasher, layout, and firmware features (including VIA).
    """
    clean_stem = stem
    for suffix in ("_default", "_via", "_vial", "_viahybrid", "_ansi", "_iso"):
        if clean_stem.endswith(suffix):
            clean_stem = clean_stem[:-len(suffix)]
            break
            
    matched_dir = None
    if clean_stem in kb_lookup:
        matched_dir = kb_lookup[clean_stem]
    else:
        parts = clean_stem.split("_")
        for i in range(len(parts), 0, -1):
            cand = "_".join(parts[:i])
            if cand in kb_lookup:
                matched_dir = kb_lookup[cand]
                break
                
    is_enhanced = clean_stem.startswith("keychron_") or clean_stem.startswith("gmmk_") or clean_stem.startswith("gmmk2_") or clean_stem.startswith("gmmk3_")

    meta = {
        "name": clean_stem.replace("_", " ").title(),
        "vendor_id": "0x3434" if clean_stem.startswith("keychron_") else None,
        "product_id": None,
        "mcu": "ARM Cortex / AVR",
        "bootloader": None,
        "layout": "Universal",
        "tier": "luxqmk_enhanced" if is_enhanced else "luxqmk_generic",
        "flasher": "dfu-util",
        "via": True,
        "features": ["nkro", "debounce", "via"]
    }
    
    if not matched_dir:
        return meta

    # Collect directory path hierarchy from keyboards/ down to matched_dir
    hierarchy = []
    cur = matched_dir
    while cur and cur != os.path.dirname(KEYBOARDS_DIR):
        hierarchy.append(cur)
        if cur == KEYBOARDS_DIR:
            break
        cur = os.path.dirname(cur)
    hierarchy.reverse()

    # Merge rules.mk, info.json, keyboard.json from top to bottom
    merged_json = {}
    merged_rules = {}

    for d in hierarchy:
        rules_path = os.path.join(d, "rules.mk")
        if os.path.isfile(rules_path):
            merged_rules.update(parse_rules_mk(rules_path))
            
        for jname in ("info.json", "keyboard.json"):
            jpath = os.path.join(d, jname)
            if os.path.isfile(jpath):
                try:
                    with open(jpath, "r", encoding="utf-8") as f:
                        data = json.load(f)
                        for k, v in data.items():
                            if isinstance(v, dict) and k in merged_json and isinstance(merged_json[k], dict):
                                merged_json[k].update(v)
                            else:
                                merged_json[k] = v
                except Exception:
                    pass

    # Extract keyboard name
    if "keyboard_name" in merged_json:
        meta["name"] = merged_json["keyboard_name"]
    elif "keyboard_folder" in merged_json:
        meta["name"] = merged_json["keyboard_folder"].split("/")[-1].replace("_", " ").title()

    # Extract USB VID/PID
    if "usb" in merged_json:
        meta["vendor_id"] = merged_json["usb"].get("vid", meta["vendor_id"])
        meta["product_id"] = merged_json["usb"].get("pid", meta["product_id"])

    # Extract MCU / processor
    if "processor" in merged_json:
        meta["mcu"] = merged_json["processor"]
    elif "MCU" in merged_rules:
        meta["mcu"] = merged_rules["MCU"]

    # Extract bootloader & Flasher Tool
    if "bootloader" in merged_json:
        meta["bootloader"] = merged_json["bootloader"]
    elif "BOOTLOADER" in merged_rules:
        meta["bootloader"] = merged_rules["BOOTLOADER"]

    # Flasher determination
    mcu_upper = meta["mcu"].upper()
    bootloader_lower = (meta["bootloader"] or "").lower()
    if "WB32" in mcu_upper:
        meta["flasher"] = "wb32-dfu-updater_cli"
    elif "RP2040" in mcu_upper or "rp2040" in bootloader_lower:
        meta["flasher"] = "uf2"
    elif "caterina" in bootloader_lower:
        meta["flasher"] = "caterina"
    elif "bootloadhid" in bootloader_lower or "bootloadhid" in mcu_upper:
        meta["flasher"] = "bootloadHID"
    elif "dfu" in bootloader_lower or "STM32" in mcu_upper or "GD32" in mcu_upper:
        meta["flasher"] = "dfu-util"
    else:
        meta["flasher"] = "dfu-util"

    # Layout extraction
    if "layouts" in merged_json:
        layouts_dict = merged_json["layouts"]
        if "LAYOUT_60_ansi" in layouts_dict or "LAYOUT_60_iso" in layouts_dict or "LAYOUT_60" in layouts_dict:
            meta["layout"] = "60%"
        elif "LAYOUT_65_ansi" in layouts_dict or "LAYOUT_65_iso" in layouts_dict or "LAYOUT_65" in layouts_dict:
            meta["layout"] = "65%"
        elif "LAYOUT_75_ansi" in layouts_dict or "LAYOUT_75_iso" in layouts_dict or "LAYOUT_75" in layouts_dict:
            meta["layout"] = "75%"
        elif "LAYOUT_tkl_ansi" in layouts_dict or "LAYOUT_tkl_iso" in layouts_dict or "LAYOUT_tkl" in layouts_dict:
            meta["layout"] = "80% TKL"
        elif "LAYOUT_all" in layouts_dict or "LAYOUT" in layouts_dict:
            meta["layout"] = "Universal"

    # Features detection
    feat_set = {"nkro", "debounce"}
    
    # VIA check: Shard metadata > rules.mk > default True for standard LuxQMK
    has_via = True
    if artifacts_meta and stem in artifacts_meta:
        has_via = artifacts_meta[stem].get("via", True)
    elif merged_rules.get("VIA_ENABLE") in ("no", "NO", "0"):
        has_via = False
    
    if has_via:
        feat_set.add("via")
    meta["via"] = has_via

    # RGB Matrix / Reactive Layers
    json_feats = merged_json.get("features", {})
    if json_feats.get("rgb_matrix") is True or "rgb_matrix" in merged_json or merged_rules.get("RGB_MATRIX_ENABLE") in ("yes", "YES", "1"):
        feat_set.add("rgb_matrix")
        feat_set.add("reactive_layers")

    # RGBLight
    if json_feats.get("rgblight") is True or "rgblight" in merged_json or merged_rules.get("RGBLIGHT_ENABLE") in ("yes", "YES", "1"):
        feat_set.add("rgblight")

    # Encoder
    if json_feats.get("encoder") is True or "encoder" in merged_json or merged_rules.get("ENCODER_ENABLE") in ("yes", "YES", "1"):
        feat_set.add("encoder")

    # OLED
    if json_feats.get("oled") is True or "oled" in merged_json or merged_rules.get("OLED_ENABLE") in ("yes", "YES", "1"):
        feat_set.add("oled")

    # Audio
    if json_feats.get("audio") is True or "audio" in merged_json or merged_rules.get("AUDIO_ENABLE") in ("yes", "YES", "1"):
        feat_set.add("audio")

    # Backlight
    if json_feats.get("backlight") is True or "backlight" in merged_json or merged_rules.get("BACKLIGHT_ENABLE") in ("yes", "YES", "1"):
        feat_set.add("backlight")

    meta["features"] = sorted(list(feat_set))
    return meta

def generate_catalog(artifacts_dir, output_dir, tag_version, repo_slug, base_url=None):
    os.makedirs(output_dir, exist_ok=True)
    kb_lookup = build_keyboard_lookup()
    
    # Base URL for direct binary downloads on files.luxqmk.click/firmware
    if not base_url:
        base_url = f"https://files.luxqmk.click/firmware/{tag_version}"
    else:
        base_url = base_url.rstrip("/")

    # Collect build-time metadata from any *.meta.json files in artifacts
    artifacts_meta = {}
    for root, _, files in os.walk(artifacts_dir):
        for file in files:
            if file.endswith(".meta.json"):
                mpath = os.path.join(root, file)
                mstem = file[:-len(".meta.json")]
                try:
                    with open(mpath, "r", encoding="utf-8") as mf:
                        artifacts_meta[mstem] = json.load(mf)
                except Exception:
                    pass

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

        # Metadata matching: Tier 1 Known Boards vs Deep Firmware Extractor
        if stem in KNOWN_BOARDS:
            meta = KNOWN_BOARDS[stem]
            kb_name = meta["name"]
            vid = meta.get("vendor_id")
            pid = meta.get("product_id")
            mcu = meta.get("mcu", "ARM Cortex / AVR")
            flasher = meta.get("flasher", "wb32-dfu-updater_cli" if "WB32" in mcu else ("dfu-util" if file.endswith((".bin", ".hex")) else "uf2"))
            layout = meta.get("layout", "Universal")
            tier = meta.get("tier", "luxqmk_enhanced")
            via = meta.get("via", True)
            features = list(meta.get("features", ["nkro", "debounce", "via", "rgb_matrix", "reactive_layers"]))
            if via and "via" not in features:
                features.append("via")
        else:
            extracted = find_keyboard_metadata(stem, kb_lookup, artifacts_meta)
            kb_name = extracted["name"]
            vid = extracted["vendor_id"]
            pid = extracted["product_id"]
            mcu = extracted["mcu"]
            flasher = extracted["flasher"]
            layout = extracted["layout"]
            tier = extracted["tier"]
            via = extracted["via"]
            features = extracted["features"]

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
            "via": via,
            "features": features,
            "file_size_bytes": file_size,
            "sha256": sha256_hash,
            "download_url": f"{base_url}/{file}",
            "latest_url": f"https://files.luxqmk.click/firmware/latest/{file}",
            "studio_url": f"https://luxqmk.click/#firmware?model={stem}"
        }
        entries.append(entry)

    # CRITICAL SAFETY CHECK: Refuse to generate an empty catalog
    if len(entries) == 0:
        print("[!] ERROR: CRITICAL SAFETY CHECK FAILED: Found 0 firmware binaries in artifacts directory!")
        print("[!] Refusing to write empty catalog.json to protect production portal from being wiped.")
        sys.exit(1)

    # Parse firmware changelog from CHANGELOG.md
    fw_changelog_path = os.path.join(ROOT_DIR, "CHANGELOG.md")
    fw_changelog = parse_changelog_file(fw_changelog_path)
    latest_fw_bullets = fw_changelog[0]["bullets"] if fw_changelog else []

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
        "latest_changelog": latest_fw_bullets,
        "changelog": fw_changelog,
        "keyboards": entries
    }

    catalog_json_path = os.path.join(output_dir, "catalog.json")
    with open(catalog_json_path, "w", encoding="utf-8") as f:
        json.dump(catalog_data, f, indent=2)

    # Standalone changelog.json endpoint
    changelog_json_path = os.path.join(output_dir, "changelog.json")
    with open(changelog_json_path, "w", encoding="utf-8") as f:
        json.dump({
            "version": tag_version,
            "generated_at": datetime.now(timezone.utc).isoformat(),
            "releases": fw_changelog
        }, f, indent=2)

    if os.path.basename(os.path.normpath(output_dir)).lower() == "firmware":
        root_dist = os.path.dirname(os.path.normpath(output_dir))
        with open(os.path.join(root_dist, "changelog.json"), "w", encoding="utf-8") as f:
            json.dump({
                "version": tag_version,
                "generated_at": datetime.now(timezone.utc).isoformat(),
                "releases": fw_changelog
            }, f, indent=2)

    print(f"[+] Successfully generated catalog.json with {len(entries)} keyboards at {catalog_json_path}")
    print(f"[+] Successfully generated changelog.json with {len(fw_changelog)} release entries at {changelog_json_path}")

    # Generate studio/version.json manifest
    generate_studio_manifest(output_dir, tag_version)

    # Generate Cloudflare Pages redirect assets and index.html fallback
    generate_redirect_assets(output_dir)

def parse_changelog_file(changelog_path):
    """
    Parses Keep-a-Changelog formatted Markdown file into a structured list of releases.
    """
    if not os.path.exists(changelog_path):
        return []

    releases = []
    current_release = None
    current_section = None

    try:
        with open(changelog_path, "r", encoding="utf-8") as f:
            for line in f:
                line_clean = line.strip()
                if line_clean.startswith("## ["):
                    if current_release:
                        releases.append(current_release)
                    
                    header_part = line_clean[3:].strip()
                    ver_end = header_part.find("]")
                    ver = header_part[1:ver_end] if ver_end != -1 else header_part
                    date_part = header_part[ver_end+1:].lstrip(" -").strip() if ver_end != -1 else ""
                    
                    current_release = {
                        "version": ver,
                        "date": date_part,
                        "sections": {},
                        "bullets": []
                    }
                    current_section = "General"
                elif line_clean.startswith("### ") and current_release:
                    sec_name = line_clean[4:].strip()
                    current_section = sec_name
                    if sec_name not in current_release["sections"]:
                        current_release["sections"][sec_name] = []
                elif line_clean.startswith("- ") and current_release:
                    bullet = line_clean[2:].strip()
                    if current_section not in current_release["sections"]:
                        current_release["sections"][current_section] = []
                    current_release["sections"][current_section].append(bullet)
                    current_release["bullets"].append(f"[{current_section}] {bullet}" if current_section != "General" else bullet)
        
        if current_release:
            releases.append(current_release)
    except Exception as e:
        print(f"[!] Warning: Failed to parse changelog from {changelog_path}: {e}")

    return releases

def get_latest_studio_version_info():
    """
    Fetches the latest official LuxQMK Studio release tag from GitHub API,
    or falls back to reading luxqmk_studio/package.json.
    """
    try:
        import urllib.request
        req = urllib.request.Request(
            "https://api.github.com/repos/LuxQMK/luxqmk_studio/releases/latest",
            headers={"User-Agent": "LuxQMK-Catalog-Generator"}
        )
        with urllib.request.urlopen(req, timeout=5) as response:
            if response.status == 200:
                data = json.loads(response.read().decode("utf-8"))
                tag = data.get("tag_name", "v1.4.1")
                return tag
    except Exception:
        pass

    # Fallback to local package.json if present
    pkg_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "luxqmk_studio", "package.json"))
    if os.path.exists(pkg_path):
        try:
            with open(pkg_path, "r", encoding="utf-8") as f:
                pkg = json.load(f)
                ver = pkg.get("version", "1.4.1").replace("-dev", "")
                return f"v{ver}"
        except Exception:
            pass

    return "v1.4.1"


def generate_studio_manifest(output_dir, release_tag=None):
    """
    Generates studio/version.json manifest for LuxQMK Studio OTA update detection.
    Resolves the actual LuxQMK Studio application release version and injects changelog.
    """
    studio_tag = get_latest_studio_version_info()
    clean_version = studio_tag.lstrip("v")
    studio_dir = os.path.join(output_dir, "studio")
    os.makedirs(studio_dir, exist_ok=True)

    # Read LuxQMK Studio changelog
    studio_cl_path = os.path.abspath(os.path.join(ROOT_DIR, "..", "luxqmk_studio", "CHANGELOG.md"))
    studio_cl_entries = parse_changelog_file(studio_cl_path)
    studio_bullets = studio_cl_entries[0]["bullets"] if studio_cl_entries else []

    studio_data = {
        "version": clean_version,
        "release_tag": studio_tag,
        "release_name": f"LuxQMK Studio {studio_tag}",
        "release_date": datetime.now(timezone.utc).isoformat(),
        "min_compatible_firmware": "0.3.2",
        "changelog": studio_bullets,
        "changelog_history": studio_cl_entries,
        "downloads": {
            "windows_installer": f"https://github.com/LuxQMK/luxqmk_studio/releases/download/{studio_tag}/LuxQMK-Studio-Setup-{clean_version}.exe",
            "web_app": "https://studio.luxqmk.click"
        }
    }

    studio_version_path = os.path.join(studio_dir, "version.json")
    with open(studio_version_path, "w", encoding="utf-8") as f:
        json.dump(studio_data, f, indent=2)

    # Also create latest.json alias
    latest_path = os.path.join(studio_dir, "latest.json")
    with open(latest_path, "w", encoding="utf-8") as f:
        json.dump(studio_data, f, indent=2)

    # If output_dir is a subfolder like dist/firmware, also write to dist/studio
    if os.path.basename(os.path.normpath(output_dir)).lower() == "firmware":
        parent_studio_dir = os.path.join(os.path.dirname(os.path.normpath(output_dir)), "studio")
        os.makedirs(parent_studio_dir, exist_ok=True)
        with open(os.path.join(parent_studio_dir, "version.json"), "w", encoding="utf-8") as f:
            json.dump(studio_data, f, indent=2)
    print(f"[+] Generated studio update manifest at {studio_version_path} (Studio version: {clean_version}, {len(studio_bullets)} changelog items)")

def generate_redirect_assets(output_dir):
    """
    Generates Cloudflare Pages _redirects, _headers, and an index.html with meta-refresh
    and JS redirection pointing directly to the main LuxQMK website (https://luxqmk.click/).
    """
    # 1. Cloudflare Pages _redirects (Edge-level 302 redirects for Studio installers & main portal)
    redirect_rules = (
        "# Root redirect to official portal\n"
        "/ https://luxqmk.click/ 302\n\n"
        "# LuxQMK Studio Installers CDN (redirects to GitHub Releases)\n"
        "/studio/latest/* https://github.com/LuxQMK/luxqmk_studio/releases/latest/download/:splat 302\n"
        "/studio/:tag/* https://github.com/LuxQMK/luxqmk_studio/releases/download/:tag/:splat 302\n"
        "/app/studio/latest/* https://github.com/LuxQMK/luxqmk_studio/releases/latest/download/:splat 302\n"
        "/app/studio/:tag/* https://github.com/LuxQMK/luxqmk_studio/releases/download/:tag/:splat 302\n\n"
        "# LuxQMK Firmware Releases fallback\n"
        "/firmware/releases/:tag/* https://github.com/LuxQMK/qmk_firmware/releases/download/:tag/:splat 302\n"
    )

    redirects_file = os.path.join(output_dir, "_redirects")
    with open(redirects_file, "w", encoding="utf-8") as f:
        f.write(redirect_rules)

    # 2. Cloudflare Pages _headers for CORS and binary downloads
    headers_content = (
        "/*\n"
        "  Access-Control-Allow-Origin: *\n"
        "  Access-Control-Allow-Methods: GET, HEAD, OPTIONS\n"
        "  Access-Control-Allow-Headers: *\n\n"
        "/*.html\n"
        "  Content-Type: text/html; charset=utf-8\n"
        "  Content-Disposition: inline\n\n"
        "/*.json\n"
        "  Content-Type: application/json; charset=utf-8\n"
        "  Content-Disposition: inline\n\n"
        "/*.bin\n"
        "  Content-Type: application/octet-stream\n"
        "  Content-Disposition: attachment\n\n"
        "/*.hex\n"
        "  Content-Type: application/octet-stream\n"
        "  Content-Disposition: attachment\n\n"
        "/*.uf2\n"
        "  Content-Type: application/octet-stream\n"
        "  Content-Disposition: attachment\n"
    )
    headers_file = os.path.join(output_dir, "_headers")
    with open(headers_file, "w", encoding="utf-8") as f:
        f.write(headers_content)

    # 3. Fallback index.html with immediate client redirect
    out_file = os.path.join(output_dir, "index.html")
    html_content = """<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta http-equiv="refresh" content="0; url=https://luxqmk.click/">
  <link rel="canonical" href="https://luxqmk.click/">
  <title>LuxQMK Firmware Files</title>
  <script>window.location.replace("https://luxqmk.click/");</script>
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
    <a class="btn" href="https://luxqmk.click/">
      Go to luxqmk.click &rarr;
    </a>
  </div>
</body>
</html>
"""
    with open(out_file, "w", encoding="utf-8") as f:
        f.write(html_content)

    # 4. Standard 404.html page to prevent Cloudflare Pages SPA fallback on missing binaries
    not_found_file = os.path.join(output_dir, "404.html")
    not_found_html = """<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>404 — File Not Found | LuxQMK</title>
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
    .badge {
      display: inline-block;
      padding: 0.25rem 0.75rem;
      border-radius: 9999px;
      background: rgba(255, 68, 68, 0.15);
      border: 1px solid rgba(255, 68, 68, 0.3);
      color: #ff5555;
      font-size: 0.85rem;
      font-weight: 700;
      margin-bottom: 1rem;
    }
    h1 {
      font-size: 1.6rem;
      margin-bottom: 0.75rem;
      color: #fff;
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
    <div class="badge">HTTP 404</div>
    <h1>Firmware File Not Found</h1>
    <p>The requested firmware file or version does not exist on this server. Please check the active catalog or visit our main portal.</p>
    <a class="btn" href="https://luxqmk.click/">
      Open LuxQMK Portal &rarr;
    </a>
  </div>
</body>
</html>
"""
    with open(not_found_file, "w", encoding="utf-8") as f:
        f.write(not_found_html)

    # If output_dir is a subfolder like dist/firmware, also write _redirects, _headers, index.html, and 404.html to root dist
    if os.path.basename(os.path.normpath(output_dir)).lower() == "firmware":
        root_dist = os.path.dirname(os.path.normpath(output_dir))
        with open(os.path.join(root_dist, "_redirects"), "w", encoding="utf-8") as f:
            f.write(redirect_rules)
        with open(os.path.join(root_dist, "_headers"), "w", encoding="utf-8") as f:
            f.write(headers_content)
        with open(os.path.join(root_dist, "index.html"), "w", encoding="utf-8") as f:
            f.write(html_content)
        with open(os.path.join(root_dist, "404.html"), "w", encoding="utf-8") as f:
            f.write(not_found_html)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate LuxQMK Firmware Catalog")
    parser.add_argument("--artifacts-dir", default=ROOT_DIR, help="Directory containing compiled binaries")
    parser.add_argument("--output-dir", default=os.path.join(ROOT_DIR, "catalog_build"), help="Output directory")
    parser.add_argument("--tag", default="v0.3.3", help="Release tag version")
    parser.add_argument("--repo", default="LuxQMK/qmk_firmware", help="GitHub repo slug")
    parser.add_argument("--base-url", default="https://files.luxqmk.click/firmware", help="Base download URL for binaries")
    args = parser.parse_args()

    generate_catalog(args.artifacts_dir, args.output_dir, args.tag, args.repo, args.base_url)
