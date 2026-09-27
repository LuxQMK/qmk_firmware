#!/usr/bin/env python3
"""
LuxQMK Incremental Catalog Sync Utility
Downloads existing compiled binaries and catalog from CDN (files.luxqmk.click)
to enable incremental/delta updates without rebuilding all 3700+ keyboards.
"""

import os
import sys
import json
import urllib.request
import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed

CDN_CATALOG_URL = "https://files.luxqmk.click/firmware/catalog.json"
CDN_BASE_LATEST = "https://files.luxqmk.click/firmware/latest"

def sync_catalog_and_binaries(output_dir, catalog_url=CDN_CATALOG_URL, max_workers=32):
    os.makedirs(output_dir, exist_ok=True)
    print(f"[+] Fetching remote catalog from {catalog_url}...")
    
    try:
        req = urllib.request.Request(catalog_url, headers={"User-Agent": "LuxQMK-Sync-Tool"})
        with urllib.request.urlopen(req, timeout=15) as resp:
            if resp.status != 200:
                print(f"[!] Warning: HTTP {resp.status} when fetching catalog. Skipping delta sync.")
                return 0
            catalog_data = json.loads(resp.read().decode("utf-8"))
    except Exception as e:
        print(f"[!] Warning: Could not download remote catalog ({e}). Starting fresh.")
        return 0

    keyboards = catalog_data.get("keyboards", [])
    print(f"[+] Remote catalog contains {len(keyboards)} keyboards. Checking for missing binaries in {output_dir}...")

    # Identify files that need to be downloaded (i.e. not already in output_dir)
    to_download = []
    for kb in keyboards:
        filename = kb.get("filename")
        if not filename:
            continue
        dest_path = os.path.join(output_dir, filename)
        if not os.path.exists(dest_path) or os.path.getsize(dest_path) == 0:
            url = kb.get("latest_url") or kb.get("download_url") or f"{CDN_BASE_LATEST}/{filename}"
            to_download.append((filename, url, dest_path))

    if not to_download:
        print(f"[+] All {len(keyboards)} binaries already present locally.")
        return len(keyboards)

    print(f"[+] Downloading {len(to_download)} missing binaries in parallel ({max_workers} threads)...")

    def download_one(item):
        fname, url, dest = item
        try:
            r = urllib.request.Request(url, headers={"User-Agent": "LuxQMK-Sync-Tool"})
            with urllib.request.urlopen(r, timeout=20) as u:
                content = u.read()
                if content:
                    with open(dest, "wb") as f:
                        f.write(content)
                    return True
        except Exception:
            return False

    success_count = 0
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        futures = {executor.submit(download_one, item): item for item in to_download}
        for future in as_completed(futures):
            if future.result():
                success_count += 1

    print(f"[+] Successfully synced {success_count}/{len(to_download)} existing binaries from CDN.")
    return len(keyboards)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Sync existing catalog and binaries from CDN")
    parser.add_argument("--output-dir", required=True, help="Directory to download existing binaries to")
    parser.add_argument("--catalog-url", default=CDN_CATALOG_URL, help="Catalog JSON endpoint")
    parser.add_argument("--threads", default=32, type=int, help="Concurrent download threads")
    args = parser.parse_args()

    sync_catalog_and_binaries(args.output_dir, args.catalog_url, args.threads)
