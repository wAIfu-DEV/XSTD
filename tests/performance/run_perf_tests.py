#!/usr/bin/env python3

import os
import shutil
import subprocess
import sys
from pathlib import Path


def resolve_root() -> Path:
    return Path(__file__).resolve().parents[2]


def compiler_available(cmd):
    if cmd[0] == "zig":
        return shutil.which("zig") is not None
    return shutil.which(cmd[0]) is not None


def compile_test(compiler_cmd, source, output, include_dirs):
    cmd = compiler_cmd + ["-std=c99", "-O2"]
    for inc in include_dirs:
        cmd.extend(["-I", str(inc)])
    cmd.extend([str(source), "-o", str(output)])
    result = subprocess.run(cmd, capture_output=True, text=True)
    return result


def run_test(executable, args, workdir):
    cmd = [str(executable)] + args
    result = subprocess.run(cmd, capture_output=True, text=True, cwd=workdir)
    return result


def main():
    repo_root = resolve_root()
    perf_dir = Path(__file__).resolve().parent

    tests = [
        {
            "name": "allocator_xstd",
            "source": perf_dir / "allocator" / "xstd_allocator.c",
            "args": [],
        },
        {
            "name": "allocator_stdlib",
            "source": perf_dir / "allocator" / "stdlib_allocator.c",
            "args": [],
        },
        {
            "name": "strbuilder_xstd",
            "source": perf_dir / "strbuilder" / "xstd_strbuilder.c",
            "args": [],
        },
        {
            "name": "strbuilder_stdlib",
            "source": perf_dir / "strbuilder" / "stdlib_strbuilder.c",
            "args": [],
        },
        {
            "name": "file_read_xstd",
            "source": perf_dir / "file_read" / "xstd_file_read.c",
            "args": [str(repo_root / "README.md")],
        },
        {
            "name": "file_read_stdlib",
            "source": perf_dir / "file_read" / "stdlib_file_read.c",
            "args": [str(repo_root / "README.md")],
        },
    ]

    compilers = [
        {"name": "clang.exe", "cmd": ["clang.exe"]},
        {"name": "gcc", "cmd": ["gcc"]},
        {"name": "zig cc", "cmd": ["zig", "cc"]},
    ]

    include_dirs = [repo_root]
    build_root = repo_root / "build" / "perf_tests"
    build_root.mkdir(parents=True, exist_ok=True)

    for compiler in compilers:
        if not compiler_available(compiler["cmd"]):
            print(f"[skip] Compiler not found: {compiler['name']}")
            continue

        compiler_tag = compiler["name"].replace(" ", "_").replace(".", "_")
        compiler_build_dir = build_root / compiler_tag
        compiler_build_dir.mkdir(parents=True, exist_ok=True)

        print(f"\n=== Running tests with {compiler['name']} ===")

        for test in tests:
            exe_suffix = ".exe" if os.name == "nt" else ""
            exe_path = compiler_build_dir / f"{test['name']}{exe_suffix}"

            print(f"-- Compiling {test['name']}...")
            compile_result = compile_test(compiler["cmd"], test["source"], exe_path, include_dirs)
            if compile_result.returncode != 0:
                print(f"   [compile failed] {test['name']}")
                print(compile_result.stderr.strip())
                continue

            print(f"   [run] {exe_path.name}")
            run_result = run_test(exe_path, test["args"], repo_root)
            if run_result.stdout:
                print(run_result.stdout.strip())
            if run_result.stderr:
                print(run_result.stderr.strip(), file=sys.stderr)
            if run_result.returncode != 0:
                print(f"   [failed] exit code {run_result.returncode}")


if __name__ == "__main__":
    main()
