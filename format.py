#!/usr/bin/env python3
import argparse
import glob
import multiprocessing
import subprocess
import sys

from concurrent.futures import ProcessPoolExecutor

CLANG_FORMAT = "clang-format-16"
CLANG_TIDY = "clang-tidy-16"

FORMAT_OPTS = "-i -style=file"
TIDY_OPTS = "-p . --fix --fix-errors"

COMPILER_OPTS = "-std=gnu11"


def is_clang_installed() -> bool:
    try:
        subprocess.run(
            f"{CLANG_FORMAT} --version".split(),
            check=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
    except FileNotFoundError:
        sys.stderr.write(f"error: {CLANG_FORMAT} not found\n")
        return False

    try:
        subprocess.run(
            f"{CLANG_TIDY} --version".split(),
            check=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
    except FileNotFoundError:
        sys.stderr.write(f"error: {CLANG_TIDY} not found\n")
        return False

    return True


def format_file(files: list[str]):
    subprocess.run(
        f"{CLANG_FORMAT} {FORMAT_OPTS} {' '.join(files)}".split(),
        check=True,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

    subprocess.run(
        f"{CLANG_TIDY} {TIDY_OPTS} {' '.join(files)} -- {COMPILER_OPTS}".split(),
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

    if not is_clang_installed():
        sys.exit(1)

    files = glob.glob("src/**/*.[c,h]", recursive=True)
    num_jobs = args.jobs if args.jobs and args.jobs > 0 else multiprocessing.cpu_count()

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
