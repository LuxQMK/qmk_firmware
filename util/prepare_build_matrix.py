#!/usr/bin/env python3
"""
LuxQMK Firmware Build Matrix Generator
Generates GitHub Actions matrix JSON for targeted or mass parallelized keyboard compilation.
"""

import os
import sys
import json
import random
import argparse
import subprocess

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
KEYBOARDS_DIR = os.path.join(ROOT_DIR, "keyboards")

GMMK3_TARGETS = [
    {"mode": "single", "kb": "gmmk/gmmk3/p100/ansi", "km": "via", "name": "gmmk3-100-ansi"},
    {"mode": "single", "kb": "gmmk/gmmk3/p75/ansi", "km": "via", "name": "gmmk3-75-ansi"},
    {"mode": "single", "kb": "gmmk/gmmk3/p65/ansi", "km": "via", "name": "gmmk3-65-ansi"},
    {"mode": "single", "kb": "gmmk/gmmk3/p100/iso", "km": "via", "name": "gmmk3-100-iso"},
    {"mode": "single", "kb": "gmmk/gmmk3/p75/iso", "km": "via", "name": "gmmk3-75-iso"},
    {"mode": "single", "kb": "gmmk/gmmk3/p65/iso", "km": "via", "name": "gmmk3-65-iso"},
]

GMMK2_TARGETS = [
    {"mode": "single", "kb": "gmmk/gmmk2/p96/ansi", "km": "via", "name": "gmmk2-96-ansi"},
    {"mode": "single", "kb": "gmmk/gmmk2/p96/iso", "km": "via", "name": "gmmk2-96-iso"},
    {"mode": "single", "kb": "gmmk/gmmk2/p65/ansi", "km": "via", "name": "gmmk2-65-ansi"},
    {"mode": "single", "kb": "gmmk/gmmk2/p65/iso", "km": "via", "name": "gmmk2-65-iso"},
]

def find_keyboards_with_keymap_filesystem(keymap_name):
    """
    Fallback method to find keyboards containing a specific keymap directory.
    """
    matches = set()
    for root, dirs, _ in os.walk(KEYBOARDS_DIR):
        if "keymaps" in dirs:
            km_dir = os.path.join(root, "keymaps")
            if os.path.exists(os.path.join(km_dir, keymap_name)):
                rel_path = os.path.relpath(root, KEYBOARDS_DIR).replace("\\", "/")
                matches.add(rel_path)
    return sorted(list(matches))

def find_keyboards(keymap_name):
    """
    Finds all keyboards supporting a keymap using qmk find or filesystem fallback.
    """
    try:
        res = subprocess.run(
            ["qmk", "find", "-km", keymap_name],
            capture_output=True,
            text=True,
            check=False
        )
        if res.returncode == 0 and res.stdout.strip():
            boards = [line.strip() for line in res.stdout.strip().splitlines() if line.strip()]
            if boards:
                return sorted(list(set(boards)))
    except Exception:
        pass
    
    return find_keyboards_with_keymap_filesystem(keymap_name)

def generate_matrix(scope, shard_count=16):
    shard_count = max(1, int(shard_count))
    
    if scope == "gmmk3_only":
        targets = GMMK3_TARGETS
    elif scope == "gmmk2_only":
        targets = GMMK2_TARGETS
    elif scope == "tier1_all":
        targets = GMMK3_TARGETS + GMMK2_TARGETS
    elif scope in ("all_via", "all_keyboards"):
        km = "via" if scope == "all_via" else "default"
        found_kbs = find_keyboards(km)
        
        if not found_kbs:
            # Fallback if no targets found
            print(f"[!] Warning: No keyboards found for keymap '{km}'. Falling back to Tier 1.", file=sys.stderr)
            targets = GMMK3_TARGETS + GMMK2_TARGETS
        else:
            # Reproducibly shuffle to balance heavy ARM vs light AVR builds across shards
            random.seed(42)
            random.shuffle(found_kbs)
            
            # Divide into balanced shards
            shards = [[] for _ in range(shard_count)]
            for i, kb in enumerate(found_kbs):
                shards[i % shard_count].append(f"{kb}:{km}")
            
            targets = []
            for idx, shard_items in enumerate(shards, 1):
                if not shard_items:
                    continue
                targets.append({
                    "mode": "shard",
                    "name": f"group-{idx:02d}",
                    "shard_id": f"{idx:02d}",
                    "keymap": km,
                    "count": len(shard_items),
                    "targets": " ".join(shard_items)
                })
    else:
        # Default fallback to Tier 1
        targets = GMMK3_TARGETS + GMMK2_TARGETS

    return {"include": targets}

def main():
    parser = argparse.ArgumentParser(description="Generate GitHub Actions matrix for LuxQMK Firmware builds")
    parser.add_argument("--scope", default="tier1_all", choices=["tier1_all", "gmmk3_only", "gmmk2_only", "all_via", "all_keyboards"], help="Target scope")
    parser.add_argument("--shards", default=16, type=int, help="Number of shards for mass compilation")
    parser.add_argument("--github-output", default=None, help="Path to GITHUB_OUTPUT file")
    parser.add_argument("--output-json", default=None, help="Optional output JSON file")
    
    args = parser.parse_args()
    matrix_data = generate_matrix(args.scope, args.shards)
    compact_json = json.dumps(matrix_data, separators=(",", ":"))
    
    total_entries = len(matrix_data["include"])
    print(f"[+] Prepared build matrix: scope='{args.scope}', shards={args.shards}, total_jobs={total_entries}")
    if matrix_data["include"] and matrix_data["include"][0].get("mode") == "shard":
        total_kbs = sum(item.get("count", 0) for item in matrix_data["include"])
        print(f"[+] Total keyboards distributed: {total_kbs} across {total_entries} runner shards")

    if args.output_json:
        with open(args.output_json, "w", encoding="utf-8") as f:
            json.dump(matrix_data, f, indent=2)

    if args.github_output:
        with open(args.github_output, "a", encoding="utf-8") as f:
            f.write(f"matrix={compact_json}\n")
    else:
        print(compact_json)

if __name__ == "__main__":
    main()
