#!/usr/bin/env python3
"""
Parallel test runner for XSTD on Windows.

Replicates the compiler/standard/optimization matrix from TEST_ALL_WIN.bat,
built around per-combination isolation so builds and executions can happen
concurrently while reporting results only after every job completes.
"""

from __future__ import annotations

import argparse
import itertools
import os
import shutil
import subprocess
import sys
from concurrent.futures import ProcessPoolExecutor, as_completed
from dataclasses import dataclass
from pathlib import Path
from typing import List, Sequence


@dataclass(frozen=True)
class Compiler:
    label: str
    command: Sequence[str]


@dataclass
class JobResult:
    compiler: str
    std: str
    opt: str
    success: bool
    stage: str
    exit_code: int
    log_path: Path
    error: str | None = None

    @property
    def label(self) -> str:
        return f"{self.compiler} {self.std} -O{self.opt}"


STANDARDS = ("c99", "c17")
OPTIMIZATIONS = ("0", "1", "2", "3", "s", "fast")
COMPILERS: Sequence[Compiler] = (
    Compiler("gcc.exe", ("gcc.exe",)),
    Compiler("clang.exe", ("clang.exe",)),
    Compiler("zig.exe cc", ("zig.exe", "cc")),
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run all compiler/standard/optimization combinations in parallel."
    )
    parser.add_argument(
        "--jobs",
        "-j",
        type=int,
        default=max(1, min(8, (os.cpu_count() or 1))),
        help="Maximum number of concurrent builds/tests (default: min(8, CPU cores)).",
    )
    parser.add_argument(
        "--root",
        type=Path,
        default=Path(__file__).resolve().parent,
        help="Root directory containing test.c and the out folder.",
    )
    parser.add_argument(
        "--keep",
        action="store_true",
        help="Keep existing per-combination output directories instead of recreating them.",
    )
    return parser.parse_args()


def safe_name(label: str) -> str:
    return "".join(ch if ch.isalnum() else "_" for ch in label).strip("_") or "compiler"


def ensure_directory(path: Path, *, recreate: bool) -> None:
    if recreate and path.exists():
        shutil.rmtree(path)
    path.mkdir(parents=True, exist_ok=True)


def run_job(
    *,
    compiler: Compiler,
    std: str,
    opt: str,
    source: Path,
    out_root: Path,
    recreate: bool,
) -> JobResult:
    combo_dir = out_root / f"{safe_name(compiler.label)}_{std}_O{opt}"
    ensure_directory(combo_dir, recreate=recreate)

    log_path = combo_dir / "build.log"
    exe_path = combo_dir / "test.exe"

    compile_args: List[str] = list(compiler.command)
    compile_args.extend(
        [
            "-g",
            str(source),
            "-o",
            str(exe_path),
            f"-std={std}",
            f"-O{opt}",
            "-Wall",
            "-Wextra",
            "-Werror",
            "-pedantic",
        ]
    )

    def append_log(lines: Sequence[str]) -> None:
        combo_dir.mkdir(parents=True, exist_ok=True)
        with log_path.open("a", encoding="utf-8") as handle:
            for line in lines:
                handle.write(line)
                if not line.endswith("\n"):
                    handle.write("\n")

    append_log([f"$ {' '.join(compile_args)}"])

    compile_proc = subprocess.run(
        compile_args,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    append_log(compile_proc.stdout.splitlines())

    if compile_proc.returncode != 0:
        return JobResult(
            compiler=compiler.label,
            std=std,
            opt=opt,
            success=False,
            stage="compile",
            exit_code=compile_proc.returncode,
            log_path=log_path,
        )

    test_proc = subprocess.run(
        [str(exe_path)],
        cwd=combo_dir,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    append_log(["", "=== Program Output ==="])
    append_log(test_proc.stdout.splitlines())

    return JobResult(
        compiler=compiler.label,
        std=std,
        opt=opt,
        success=test_proc.returncode == 0,
        stage="complete" if test_proc.returncode == 0 else "test",
        exit_code=test_proc.returncode,
        log_path=log_path,
    )


def main() -> int:
    args = parse_args()
    root: Path = args.root

    source = root / "test.c"
    if not source.exists():
        print(f"error: cannot find test.c at {source}", file=sys.stderr)
        return 1

    out_root = root / "out"
    out_root.mkdir(exist_ok=True)

    jobs = list(itertools.product(COMPILERS, STANDARDS, OPTIMIZATIONS))
    total = len(jobs)
    print(f"Queued {total} job(s) with up to {args.jobs} concurrent workers.")

    results: List[JobResult] = []
    with ProcessPoolExecutor(max_workers=args.jobs) as executor:
        future_to_combo = {
            executor.submit(
                run_job,
                compiler=compiler,
                std=std,
                opt=opt,
                source=source,
                out_root=out_root,
                recreate=not args.keep,
            ): (compiler.label, std, opt)
            for compiler, std, opt in jobs
        }

        for future in as_completed(future_to_combo):
            compiler_label, std, opt = future_to_combo[future]
            combo_text = f"{compiler_label} {std} -O{opt}"
            try:
                result = future.result()
            except Exception as exc:
                log_path = (
                    out_root / f"{safe_name(compiler_label)}_{std}_O{opt}" / "build.log"
                )
                if not log_path.exists():
                    log_path.parent.mkdir(parents=True, exist_ok=True)
                    with log_path.open("w", encoding="utf-8") as handle:
                        handle.write(f"Unhandled exception: {exc}\n")
                results.append(
                    JobResult(
                        compiler=compiler_label,
                        std=std,
                        opt=opt,
                        success=False,
                        stage="exception",
                        exit_code=1,
                        log_path=log_path,
                        error=str(exc),
                    )
                )
                print(f"[FAIL] {combo_text} (exception)", flush=True)
            else:
                results.append(result)
                status = "PASS" if result.success else f"FAIL ({result.stage})"
                print(f"[{status}] {combo_text}", flush=True)

    failures = [res for res in results if not res.success]
    print()
    print(f"Completed {len(results)} job(s).")

    if failures:
        print(f"Failures: {len(failures)}")
        for res in failures:
            msg = f"  {res.label} during {res.stage} (exit {res.exit_code}) -> {res.log_path}"
            if res.error:
                msg += f" | error: {res.error}"
            print(msg)
        return 1

    print("All builds and tests succeeded.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
