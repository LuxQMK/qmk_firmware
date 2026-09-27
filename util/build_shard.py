#!/usr/bin/env python3
"""
LuxQMK Parallel Shard Compilation Engine
Compiles a list of keyboard targets concurrently using isolated subprocesses.
Ensures single board failures (e.g. invalid targets or compile errors) never abort
the rest of the shard, and captures exact compiler output into .build/failed.log.*.
"""

import os
import sys
import glob
import shutil
import argparse
import subprocess
from concurrent.futures import ThreadPoolExecutor, as_completed

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

def collect_binaries(kb_safe: str, keymap: str, artifacts_dir: str, build_dir: str):
    found_binaries = []
    extensions = (".bin", ".hex", ".uf2")
    
    # Check root dir for {kb_safe}_{keymap}.* or {kb_safe}.*
    for ext in extensions:
        for fname in (f"{kb_safe}_{keymap}{ext}", f"{kb_safe}{ext}"):
            target_file = os.path.join(ROOT_DIR, fname)
            if os.path.isfile(target_file):
                dest = os.path.join(artifacts_dir, os.path.basename(target_file))
                shutil.copy2(target_file, dest)
                found_binaries.append(dest)
                
    # Check .build dir
    if not found_binaries and os.path.isdir(build_dir):
        for ext in extensions:
            for f in glob.glob(os.path.join(build_dir, f"*{kb_safe}*{ext}")):
                dest = os.path.join(artifacts_dir, os.path.basename(f))
                shutil.copy2(f, dest)
                found_binaries.append(dest)
                
    return list(set(found_binaries))

def compile_target(target: str, artifacts_dir: str, build_dir: str, via_enable: bool = True, ccache_enable: bool = True):
    parts = target.strip().split(":")
    if len(parts) < 2:
        return target, False, "Invalid target format (expected keyboard:keymap)"
    
    keyboard = parts[0]
    keymap = parts[1]
    kb_safe = keyboard.replace("/", "_")
    
    # Primary build attempt (with VIA_ENABLE if requested)
    cmd = ["qmk", "compile", "-kb", keyboard, "-km", keymap]
    if via_enable:
        cmd.extend(["-e", "VIA_ENABLE=yes"])
    if ccache_enable:
        cmd.extend(["-e", "USE_CCACHE=yes"])
        
    try:
        res = subprocess.run(
            cmd,
            cwd=ROOT_DIR,
            capture_output=True,
            text=True,
            check=False
        )
        output = (res.stdout or "") + "\n" + (res.stderr or "")
    except Exception as e:
        output = f"Failed to execute qmk command: {e}"
        res = subprocess.CompletedProcess(args=cmd, returncode=1)

    if res.returncode == 0:
        found_binaries = collect_binaries(kb_safe, keymap, artifacts_dir, build_dir)
        return target, True, f"Produced {len(found_binaries)} binaries"
    
    # Fallback build attempt: if VIA failed (e.g. flash/eeprom/layer limits), try pure default build
    if via_enable:
        fallback_cmd = ["qmk", "compile", "-kb", keyboard, "-km", keymap]
        if ccache_enable:
            fallback_cmd.extend(["-e", "USE_CCACHE=yes"])
        try:
            res_fb = subprocess.run(
                fallback_cmd,
                cwd=ROOT_DIR,
                capture_output=True,
                text=True,
                check=False
            )
            if res_fb.returncode == 0:
                found_binaries = collect_binaries(kb_safe, keymap, artifacts_dir, build_dir)
                return target, True, f"Produced {len(found_binaries)} binaries (fallback)"
            else:
                # Include fallback error info in output
                output += "\n--- Fallback Attempt (without VIA) Output ---\n" + (res_fb.stdout or "") + "\n" + (res_fb.stderr or "")
        except Exception:
            pass

    # Record failure log in .build/failed.log.<pid>.<kb_safe>.<keymap>
    pid = os.getpid()
    failed_log_path = os.path.join(build_dir, f"failed.log.{pid}.{kb_safe}.{keymap}")
    os.makedirs(build_dir, exist_ok=True)
    try:
        with open(failed_log_path, "w", encoding="utf-8", errors="replace") as f:
            f.write(output)
    except Exception:
        pass
        
    # Extract short error line
    error_lines = [l for l in output.splitlines() if "error" in l.lower() or "overflow" in l.lower() or "assert" in l.lower()]
    snippet = error_lines[-1] if error_lines else "Compilation returned non-zero exit code"
    return target, False, snippet

def main():
    parser = argparse.ArgumentParser(description="LuxQMK Parallel Shard Compilation Engine")
    parser.add_argument("--targets-file", default=None, help="File containing list of space/newline-separated targets")
    parser.add_argument("--targets", nargs="*", default=[], help="List of targets")
    parser.add_argument("--parallel", type=int, default=max(1, os.cpu_count() or 4), help="Number of parallel compiler threads")
    parser.add_argument("--artifacts-dir", default="artifacts", help="Destination folder for compiled binaries")
    parser.add_argument("--build-dir", default=".build", help="Directory for QMK build output & logs")
    parser.add_argument("--via", action="store_true", default=True, help="Enable VIA (VIA_ENABLE=yes)")
    parser.add_argument("--ccache", action="store_true", default=True, help="Enable ccache")
    
    args = parser.parse_args()
    
    artifacts_dir = os.path.abspath(args.artifacts_dir)
    build_dir = os.path.abspath(args.build_dir)
    os.makedirs(artifacts_dir, exist_ok=True)
    os.makedirs(build_dir, exist_ok=True)
    
    target_list = []
    if args.targets_file and os.path.isfile(args.targets_file):
        with open(args.targets_file, "r", encoding="utf-8") as f:
            target_list.extend(f.read().split())
    if args.targets:
        target_list.extend(args.targets)
        
    # Deduplicate while preserving order
    unique_targets = []
    seen = set()
    for t in target_list:
        clean = t.strip()
        if clean and clean not in seen:
            seen.add(clean)
            unique_targets.append(clean)
            
    total = len(unique_targets)
    print(f"[+] Starting parallel shard build: {total} targets with {args.parallel} worker threads.")
    
    if total == 0:
        print("[!] No targets provided.")
        return
        
    succeeded = 0
    failed = 0
    completed = 0
    
    with ThreadPoolExecutor(max_workers=args.parallel) as executor:
        future_to_target = {
            executor.submit(compile_target, t, artifacts_dir, build_dir, args.via, args.ccache): t
            for t in unique_targets
        }
        
        for future in as_completed(future_to_target):
            target = future_to_target[future]
            completed += 1
            try:
                tgt, ok, msg = future.result()
                if ok:
                    succeeded += 1
                    print(f"[{completed}/{total}] \033[92m[OK]\033[0m {tgt}")
                else:
                    failed += 1
                    print(f"[{completed}/{total}] \033[91m[FAILED]\033[0m {tgt} -> {msg[:90]}")
            except Exception as e:
                failed += 1
                print(f"[{completed}/{total}] \033[91m[ERROR]\033[0m {target} -> {e}")

    print(f"\n[+] Shard build complete: {succeeded}/{total} succeeded, {failed}/{total} failed.")

if __name__ == "__main__":
    main()
