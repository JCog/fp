#!/usr/bin/env python3
import argparse
import glob
import multiprocessing
import re
import subprocess
import sys

from concurrent.futures import ProcessPoolExecutor

CLANG_VER = 16

FORMAT_OPTS = "-i -style=file"
TIDY_OPTS = "-p . --fix --fix-errors"

COMPILER_OPTS = "-std=gnu11"


def get_clang(program: str) -> str:
    try:
        subprocess.run(
            f"clang-{program}-{CLANG_VER} --version".split(),
            check=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        return f"clang-{program}-{CLANG_VER}"
    except FileNotFoundError:
        out = subprocess.run(
            f"clang-{program} --version".split(), check=True, capture_output=True
        )
        ver_re = re.compile(r"version (\d+)(\.\d+\.\d+)")
        match = re.search(ver_re, out.stdout.decode())
        if match:
            ver = int(match.group(1))
            if ver >= CLANG_VER:
                return f"clang-{program}"
    return ""


def format_file(f: list[str]):
    subprocess.run(
        f"{clang_format} {FORMAT_OPTS} {' '.join(f)}".split(),
        check=True,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )


def tidy_file(f: list[str]):
    subprocess.run(
        f"{clang_tidy} {TIDY_OPTS} {' '.join(f)} -- {COMPILER_OPTS}".split(),
        check=True,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Format files using clang-format and clang-tidy"
    )
    parser.add_argument(
        "-j", "--jobs", type=int, help="Number of concurrent jobs to run"
    )
    args = parser.parse_args()

    try:
        clang_format = get_clang("format")
    except FileNotFoundError:
        sys.stderr.write("error: clang-format 16 not found")
        sys.exit(1)

    try:
        clang_tidy = get_clang("tidy")
    except FileNotFoundError:
        sys.stderr.write("error: clang-tidy 16 not found")
        sys.exit(1)

    files = glob.glob("src/**/*.[c,h]", recursive=True)
    num_jobs = args.jobs if args.jobs and args.jobs > 0 else multiprocessing.cpu_count()

    print(f"Formatting {len(files)} files with {num_jobs} jobs...")
    if num_jobs == 1:
        format_file(files)
    else:
        with ProcessPoolExecutor(max_workers=num_jobs) as executor:
            chunks = [
                files[i : i + len(files) // num_jobs]
                for i in range(0, len(files), len(files) // num_jobs)
            ]
            executor.map(format_file, chunks)
    print("Done formatting all files.")
