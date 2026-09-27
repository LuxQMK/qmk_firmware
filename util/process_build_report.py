#!/usr/bin/env python3
"""
LuxQMK Build Report & Failure Diagnostics Engine
Parses QMK compilation outputs, isolates failed targets, extracts compiler error diagnostics,
and generates structured reports to help maintainers systematically fix broken keyboard targets.
"""

import os
import sys
import glob
import json
import argparse

def categorize_error(log_content: str) -> str:
    lower = log_content.lower()
    if "overflowed by" in lower or "exceeds" in lower or "section `.text' will not fit" in lower or "flash overflow" in lower:
        return "Flash/ROM Size Overflow"
    if "undefined reference to" in lower or "undefined symbol" in lower or "ld returned 1 exit status" in lower:
        return "Linker: Undefined Reference"
    if "no such file or directory" in lower or "fatal error:" in lower:
        return "Missing Header / File"
    if "error: " in lower:
        return "C Compiler Syntax / Type Error"
    if "matrix" in lower and ("pin" in lower or "error" in lower):
        return "Hardware Matrix Pin Config Error"
    return "Other Compilation Error"

def extract_error_snippet(log_content: str, max_lines: int = 8) -> str:
    lines = log_content.strip().splitlines()
    error_lines = []
    for line in lines:
        if any(keyword in line for keyword in ["error:", "fatal error:", "overflowed by", "undefined reference", "Error 1", "Error 2"]):
            error_lines.append(line.strip())
    
    if error_lines:
        return "\n".join(error_lines[-max_lines:])
    
    # Fallback to tail lines
    return "\n".join(lines[-max_lines:]) if lines else "Unknown failure"

def process_shard(build_dir: str, artifacts_dir: str, output_dir: str, shard_name: str):
    os.makedirs(output_dir, exist_ok=True)
    logs_dir = os.path.join(output_dir, "logs")
    os.makedirs(logs_dir, exist_ok=True)

    # 1. Detect compiled binaries in artifacts
    compiled_binaries = []
    if os.path.exists(artifacts_dir):
        for f in os.listdir(artifacts_dir):
            if f.endswith((".bin", ".hex", ".uf2")):
                compiled_binaries.append(f)

    # 2. Parse failed logs
    failed_logs = glob.glob(os.path.join(build_dir, "failed.log.*"))
    failures = []
    failed_targets_list = []

    for fpath in failed_logs:
        fname = os.path.basename(fpath)
        # Format is failed.log.<pid>.<keyboard_safe>.<keymap>[.<extra>]
        parts = fname.split(".")
        if len(parts) >= 4:
            kb_safe = parts[2]
            km = parts[3]
        else:
            kb_safe = fname
            km = "default"

        kb_name = kb_safe.replace("_", "/")
        target_name = f"{kb_name}:{km}"
        failed_targets_list.append(target_name)

        try:
            with open(fpath, "r", encoding="utf-8", errors="replace") as f:
                content = f.read()
        except Exception:
            content = "Could not read log file"

        cat = categorize_error(content)
        snippet = extract_error_snippet(content)

        # Copy individual log to diagnostics/logs
        log_out_name = f"{kb_safe}_{km}.log"
        with open(os.path.join(logs_dir, log_out_name), "w", encoding="utf-8") as f:
            f.write(content)

        failures.append({
            "target": target_name,
            "keyboard": kb_name,
            "keymap": km,
            "category": cat,
            "snippet": snippet,
            "log_file": log_out_name
        })

    # If no binaries were produced and no failed.log was caught, register target as failed
    if len(compiled_binaries) == 0 and len(failures) == 0 and shard_name != "unknown":
        target_name = shard_name
        failed_targets_list.append(target_name)
        log_out_name = f"{shard_name}.log"
        with open(os.path.join(logs_dir, log_out_name), "w", encoding="utf-8") as f:
            f.write(f"Target {shard_name} failed to produce any binary output.\n")

        failures.append({
            "target": target_name,
            "keyboard": shard_name,
            "keymap": "via",
            "category": "Compilation Failed (No Binary)",
            "snippet": f"Target {shard_name} failed to produce any binary artifact.",
            "log_file": log_out_name
        })

    # Write failed_keyboards.txt
    failed_targets_list.sort()
    with open(os.path.join(output_dir, "failed_keyboards.txt"), "w", encoding="utf-8") as f:
        for t in failed_targets_list:
            f.write(f"{t}\n")

    # Write shard summary JSON
    shard_summary = {
        "shard": shard_name,
        "success_count": len(compiled_binaries),
        "failure_count": len(failures),
        "successful_binaries": sorted(compiled_binaries),
        "failures": failures
    }

    with open(os.path.join(output_dir, "shard_summary.json"), "w", encoding="utf-8") as f:
        json.dump(shard_summary, f, indent=2)

    print(f"[+] Shard {shard_name}: {len(compiled_binaries)} succeeded, {len(failures)} failed.")

def aggregate_reports(diagnostics_root: str, output_dir: str, github_summary_file: str = None):
    os.makedirs(output_dir, exist_ok=True)
    all_logs_dir = os.path.join(output_dir, "logs")
    os.makedirs(all_logs_dir, exist_ok=True)

    all_failures = []
    total_success = 0
    total_failed = 0
    categories = {}

    summary_files = glob.glob(os.path.join(diagnostics_root, "**", "shard_summary.json"), recursive=True)
    for sfile in summary_files:
        try:
            with open(sfile, "r", encoding="utf-8") as f:
                data = json.load(f)
            total_success += data.get("success_count", 0)
            total_failed += data.get("failure_count", 0)
            for fail in data.get("failures", []):
                all_failures.append(fail)
                cat = fail.get("category", "Other")
                categories[cat] = categories.get(cat, 0) + 1
        except Exception as e:
            print(f"[!] Error reading {sfile}: {e}", file=sys.stderr)

    # Copy all individual logs
    log_files = glob.glob(os.path.join(diagnostics_root, "**", "logs", "*.log"), recursive=True)
    for lf in log_files:
        dest = os.path.join(all_logs_dir, os.path.basename(lf))
        try:
            with open(lf, "r", encoding="utf-8", errors="replace") as src, open(dest, "w", encoding="utf-8") as dst:
                dst.write(src.read())
        except Exception:
            pass

    # Sort failures alphabetically by target
    all_failures.sort(key=lambda x: x["target"])

    # Write all_failed_keyboards.txt
    failed_txt_path = os.path.join(output_dir, "all_failed_keyboards.txt")
    with open(failed_txt_path, "w", encoding="utf-8") as f:
        for f in all_failures:
            f.write(f"{f['target']} [{f['category']}]\n")

    # Write full breakdown JSON
    breakdown = {
        "total_attempted": total_success + total_failed,
        "total_success": total_success,
        "total_failed": total_failed,
        "success_rate_percent": round((total_success / (total_success + total_failed) * 100), 2) if (total_success + total_failed) > 0 else 0,
        "categories": categories,
        "failed_targets": all_failures
    }

    with open(os.path.join(output_dir, "failure_breakdown.json"), "w", encoding="utf-8") as f:
        json.dump(breakdown, f, indent=2)

    # Generate Markdown Report
    total_all = total_success + total_failed
    rate = breakdown["success_rate_percent"]
    
    md_lines = [
        "## 🛠️ LuxQMK Compilation & Failure Diagnostics Report\n",
        f"| Metric | Count | Percentage |",
        f"| :--- | :--- | :--- |",
        f"| **Total Attempted** | **{total_all}** | 100% |",
        f"| **✅ Successfully Built** | **{total_success}** | **{rate}%** |",
        f"| **❌ Failed Keyboards** | **{total_failed}** | **{round(100 - rate, 2) if total_all > 0 else 0}%** |\n",
        "### 🔍 Breakdown by Failure Cause\n",
        "| Cause / Error Category | Number of Boards |",
        "| :--- | :--- |"
    ]

    for cat, count in sorted(categories.items(), key=lambda x: x[1], reverse=True):
        md_lines.append(f"| `{cat}` | **{count}** |")

    if all_failures:
        md_lines.append("\n### 📋 Failed Keyboard Targets (Sample List)\n")
        md_lines.append("| Keyboard Target | Category | Error Snippet |")
        md_lines.append("| :--- | :--- | :--- |")
        for fail in all_failures[:35]: # Show first 35 failures
            snippet_clean = fail['snippet'].replace("\n", "<br>").replace("|", "\\|")
            md_lines.append(f"| `{fail['target']}` | {fail['category']} | `{snippet_clean[:120]}` |")
        if len(all_failures) > 35:
            md_lines.append(f"| *...and {len(all_failures) - 35} more targets* | *See `all_failed_keyboards.txt` artifact* | |")

    md_content = "\n".join(md_lines) + "\n"

    report_md_path = os.path.join(output_dir, "build_report.md")
    with open(report_md_path, "w", encoding="utf-8") as f:
        f.write(md_content)

    if github_summary_file:
        try:
            with open(github_summary_file, "a", encoding="utf-8") as f:
                f.write(md_content)
        except Exception as e:
            print(f"[!] Could not write to GITHUB_STEP_SUMMARY: {e}", file=sys.stderr)

    print(f"[+] Consolidated report: {total_success} succeeded, {total_failed} failed ({rate}% success rate).")

def main():
    parser = argparse.ArgumentParser(description="LuxQMK Build Failure & Diagnostics Processor")
    parser.add_argument("--mode", choices=["shard", "aggregate"], required=True, help="Processing mode")
    parser.add_argument("--build-dir", default=".build", help="Path to .build directory")
    parser.add_argument("--artifacts-dir", default="artifacts", help="Path to artifacts directory")
    parser.add_argument("--output-dir", default="diagnostics", help="Output directory for reports")
    parser.add_argument("--shard-name", default="unknown", help="Shard identifier")
    parser.add_argument("--diagnostics-root", default="all_diagnostics", help="Root directory with all shard diagnostics")
    parser.add_argument("--github-summary", default=None, help="Path to GITHUB_STEP_SUMMARY")

    args = parser.parse_args()

    if args.mode == "shard":
        process_shard(args.build_dir, args.artifacts_dir, args.output_dir, args.shard_name)
    elif args.mode == "aggregate":
        aggregate_reports(args.diagnostics_root, args.output_dir, args.github_summary)

if __name__ == "__main__":
    main()
