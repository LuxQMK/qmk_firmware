#!/usr/bin/env python3
"""
LuxQMK Firmware Build Matrix Generator
Generates GitHub Actions matrix JSON for targeted or mass parallelized keyboard compilation.
Supports deterministic sharding so matrix JSON remains lightweight while runners dynamically
slice the targets.
"""

import os
import sys
import glob
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

def list_leaf_keyboards():
    """
    Finds all compilable leaf keyboards in the repository.
    Searches for keyboard.json and rules.mk while resolving hierarchy.
    """
    try:
        res = subprocess.run(["qmk", "list-keyboards"], capture_output=True, text=True, check=False)
        if res.returncode == 0 and res.stdout.strip():
            kbs = [line.strip() for line in res.stdout.strip().splitlines() if line.strip()]
            if kbs:
                return sorted(set(kbs))
    except Exception:
        pass

    # Filesystem discovery of keyboard definitions
    kb_wildcard = os.path.join(KEYBOARDS_DIR, "**", "keyboard.json")
    paths = [p for p in glob.glob(kb_wildcard, recursive=True) if os.path.sep + "keymaps" + os.path.sep not in p]
    found = [os.path.relpath(os.path.dirname(p), KEYBOARDS_DIR).replace("\\", "/") for p in paths]
    if not found:
        rules_wildcard = os.path.join(KEYBOARDS_DIR, "**", "rules.mk")
        r_paths = [p for p in glob.glob(rules_wildcard, recursive=True) if os.path.sep + "keymaps" + os.path.sep not in p]
        found = [os.path.relpath(os.path.dirname(p), KEYBOARDS_DIR).replace("\\", "/") for p in r_paths]
    
    return sorted(set(found))

def has_keymap_in_hierarchy(kb: str, keymap_name: str) -> bool:
    """
    Checks if a keymap folder exists directly on the keyboard or in any of its parent folders.
    """
    cur = os.path.join(KEYBOARDS_DIR, kb)
    while True:
        km_dir = os.path.join(cur, "keymaps", keymap_name)
        if os.path.isdir(km_dir):
            return True
        if cur == KEYBOARDS_DIR or not cur.startswith(KEYBOARDS_DIR):
            break
        cur = os.path.dirname(cur)
    return False

def find_all_targets(preferred_keymap="via", fallback_keymap="default"):
    """
    Finds all keyboard targets in the repository.
    For each valid leaf keyboard:
    - If `preferred_keymap` exists in its hierarchy, use `kb:preferred_keymap`
    - Otherwise, use `kb:fallback_keymap`
    """
    leaf_keyboards = list_leaf_keyboards()
    targets = []
    for kb in leaf_keyboards:
        if has_keymap_in_hierarchy(kb, preferred_keymap):
            targets.append(f"{kb}:{preferred_keymap}")
        else:
            targets.append(f"{kb}:{fallback_keymap}")
            
    return targets

def get_deterministic_targets(scope, custom_targets=None):
    if scope == "custom" and custom_targets:
        raw = [t.strip() for t in custom_targets.replace(",", " ").split() if t.strip()]
        targets = []
        for t in raw:
            if ":" not in t:
                if has_keymap_in_hierarchy(t, "via"):
                    targets.append(f"{t}:via")
                else:
                    targets.append(f"{t}:default")
            else:
                targets.append(t)
        return targets
    elif scope in ("all_via", "all_keyboards"):
        targets = find_all_targets(preferred_keymap="via", fallback_keymap="default")
        random.seed(42)
        random.shuffle(targets)
        return targets
    elif scope == "gmmk3_only":
        return [f"{t['kb']}:{t['km']}" for t in GMMK3_TARGETS]
    elif scope == "gmmk2_only":
        return [f"{t['kb']}:{t['km']}" for t in GMMK2_TARGETS]
    elif scope in ("tier1_all", "tier1_only"):
        return [f"{t['kb']}:{t['km']}" for t in (GMMK3_TARGETS + GMMK2_TARGETS)]
    else:
        return [f"{t['kb']}:{t['km']}" for t in (GMMK3_TARGETS + GMMK2_TARGETS)]

MAX_OCI_WORKERS = 4
OCI_WEIGHT = 0.25  # OCI 1-thread ARM workers receive 25% the load of 2-vCPU x86 GitHub runners
GH_WEIGHT = 1.0

def get_runner_for_job(idx, pool="hybrid", max_oci_workers=MAX_OCI_WORKERS):
    """
    Returns runner labels and pool identifier based on allocation strategy.
    In hybrid mode:
    - First N jobs (up to MAX_OCI_WORKERS=4) are allocated to dedicated 1:1 OCI cloud nodes.
    - All remaining jobs (up to 20) are allocated to GitHub-hosted runners (ubuntu-latest).
    This guarantees 100% immediate parallel job execution across both clusters with zero queue wait.
    """
    if pool == "oci_only":
        return ["self-hosted", "oci-builder"], "oci"
    elif pool == "github_only":
        return ["ubuntu-latest"], "github"
    else:  # hybrid
        if idx <= max_oci_workers:
            return ["self-hosted", "oci-builder"], "oci"
        else:
            return ["ubuntu-latest"], "github"

def get_weighted_shard_bounds(total_items, shard_count, shard_id, runner_pool="hybrid"):
    """
    Calculates deterministic start and end slice indices for a shard using weighted load balancing.
    OCI shards get weight 0.25 (quarter load), while GitHub runners get weight 1.0.
    """
    weights = []
    for i in range(1, shard_count + 1):
        _, pool = get_runner_for_job(i, runner_pool)
        if pool == "oci":
            weights.append(OCI_WEIGHT)
        else:
            weights.append(GH_WEIGHT)

    total_weight = sum(weights)
    shard_idx = shard_id - 1

    start_ratio = sum(weights[:shard_idx]) / total_weight
    end_ratio = sum(weights[:shard_idx + 1]) / total_weight

    start_idx = round(start_ratio * total_items)
    end_idx = round(end_ratio * total_items)
    return start_idx, end_idx

def get_shard_targets(scope, shard_id, shard_count, runner_pool="hybrid", custom_targets=None):
    all_targets = get_deterministic_targets(scope, custom_targets)
    total_items = len(all_targets)
    shard_count = max(1, int(shard_count))
    shard_id = max(1, min(int(shard_id), shard_count))
    
    start_idx, end_idx = get_weighted_shard_bounds(total_items, shard_count, shard_id, runner_pool)
    return all_targets[start_idx:end_idx]

def generate_matrix(scope, shard_count=16, runner_pool="hybrid", custom_targets=None):
    shard_count = max(1, int(shard_count))
    
    if scope == "gmmk3_only":
        raw_targets = [dict(t) for t in GMMK3_TARGETS]
    elif scope == "gmmk2_only":
        raw_targets = [dict(t) for t in GMMK2_TARGETS]
    elif scope in ("tier1_all", "tier1_only"):
        raw_targets = [dict(t) for t in (GMMK3_TARGETS + GMMK2_TARGETS)]
    elif scope == "custom":
        all_targets = get_deterministic_targets(scope, custom_targets)
        if not all_targets:
            raise ValueError("No targets found for custom scope. Please pass --custom-targets.")
            
        if len(all_targets) <= 8:
            raw_targets = []
            for t in all_targets:
                parts = t.split(":")
                kb = parts[0]
                km = parts[1] if len(parts) > 1 else "via"
                kb_name = kb.replace("/", "_")
                raw_targets.append({
                    "mode": "single",
                    "name": f"{kb_name}_{km}",
                    "kb": kb,
                    "km": km
                })
        else:
            effective_shards = min(shard_count, len(all_targets))
            raw_targets = []
            for idx in range(1, effective_shards + 1):
                runner_labels, pool_name = get_runner_for_job(idx, runner_pool)
                start_idx, end_idx = get_weighted_shard_bounds(len(all_targets), effective_shards, idx, runner_pool)
                raw_targets.append({
                    "mode": "shard",
                    "name": f"group-{idx:02d}",
                    "shard_id": idx,
                    "total_shards": effective_shards,
                    "keymap": "custom",
                    "estimated_count": end_idx - start_idx,
                    "runner": runner_labels,
                    "pool": pool_name
                })
    elif scope in ("all_via", "all_keyboards"):
        all_targets = get_deterministic_targets(scope)
        total_kbs = len(all_targets)
        
        raw_targets = []
        for idx in range(1, shard_count + 1):
            runner_labels, pool_name = get_runner_for_job(idx, runner_pool)
            start_idx, end_idx = get_weighted_shard_bounds(total_kbs, shard_count, idx, runner_pool)
            raw_targets.append({
                "mode": "shard",
                "name": f"group-{idx:02d}",
                "shard_id": idx,
                "total_shards": shard_count,
                "keymap": "via/default",
                "estimated_count": end_idx - start_idx,
                "runner": runner_labels,
                "pool": pool_name
            })
    else:
        raw_targets = [dict(t) for t in (GMMK3_TARGETS + GMMK2_TARGETS)]

    # Assign runners for single-mode targets
    if raw_targets and raw_targets[0].get("mode") != "shard":
        for idx, item in enumerate(raw_targets, start=1):
            runner_labels, pool_name = get_runner_for_job(idx, runner_pool)
            item["runner"] = runner_labels
            item["pool"] = pool_name

    return {"include": raw_targets}

def main():
    parser = argparse.ArgumentParser(description="Generate GitHub Actions matrix for LuxQMK Firmware builds")
    parser.add_argument("--scope", default="tier1_all", choices=["tier1_all", "tier1_only", "gmmk3_only", "gmmk2_only", "all_via", "all_keyboards", "custom"], help="Target scope")
    parser.add_argument("--custom-targets", default=None, help="Custom targets comma/space separated (e.g. gmmk/gmmk3/p75/ansi:via)")
    parser.add_argument("--shards", default=16, type=int, help="Number of shards for mass compilation")
    parser.add_argument("--runner-pool", default="hybrid", choices=["hybrid", "oci_only", "github_only"], help="Runner execution pool strategy")
    parser.add_argument("--github-output", default=None, help="Path to GITHUB_OUTPUT file")
    parser.add_argument("--output-json", default=None, help="Optional output JSON file")
    parser.add_argument("--get-shard-targets", action="store_true", help="Retrieve targets for a specific shard ID")
    parser.add_argument("--shard-id", type=int, default=1, help="Shard ID (1-indexed)")
    parser.add_argument("--output-targets-file", default=None, help="File to write shard targets to")
    
    args = parser.parse_args()

    if args.get_shard_targets:
        shard_targets = get_shard_targets(args.scope, args.shard_id, args.shards, args.runner_pool, args.custom_targets)
        targets_str = " ".join(shard_targets)
        if args.output_targets_file:
            with open(args.output_targets_file, "w", encoding="utf-8") as f:
                f.write(targets_str)
            print(f"[+] Written {len(shard_targets)} targets for shard {args.shard_id}/{args.shards} to {args.output_targets_file}")
        else:
            print(targets_str)
        return

    matrix_data = generate_matrix(args.scope, args.shards, args.runner_pool, args.custom_targets)
    compact_json = json.dumps(matrix_data, separators=(",", ":"))
    
    total_entries = len(matrix_data["include"])
    print(f"[+] Prepared build matrix: scope='{args.scope}', shards={args.shards}, pool='{args.runner_pool}', total_jobs={total_entries}")
    if matrix_data["include"] and matrix_data["include"][0].get("mode") == "shard":
        all_targets = get_deterministic_targets(args.scope, args.custom_targets)
        print(f"[+] Total keyboards distributed: {len(all_targets)} across {total_entries} runner shards")

    if args.output_json:
        with open(args.output_json, "w", encoding="utf-8") as f:
            json.dump(matrix_data, f, indent=2)

    if args.github_output:
        delimiter = f"EOF_MATRIX_{os.urandom(6).hex()}"
        with open(args.github_output, "a", encoding="utf-8") as f:
            f.write(f"matrix<<{delimiter}\n{compact_json}\n{delimiter}\n")
    else:
        print(compact_json)

if __name__ == "__main__":
    main()
