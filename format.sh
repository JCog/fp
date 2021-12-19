#!/usr/bin/env bash

FORMAT_OPTS="-i -style=file"
TIDY_OPTS="-p . --fix --fix-errors"
COMPILER_OPTS="-std=gnu11"

shopt -s globstar

if (( $# > 0 )); then
    echo "Formatting file(s) $*"
    echo "Running clang-tidy..."
    clang-tidy-14 ${TIDY_OPTS} "$@" -- ${COMPILER_OPTS} &> /dev/null
    echo "Running clang-format..."
    clang-format-14 ${FORMAT_OPTS} "$@"
    echo "Adding missing final new lines..."
    sed -i -e '$a\' "$@"
    echo "Done formatting file(s) $*"
    exit
fi

echo "Formatting C files. This will take a bit"
echo "Running clang-tidy..."
clang-tidy-14 ${TIDY_OPTS} src/*.c -- ${COMPILER_OPTS} &> /dev/null
echo "Running clang-format..."
clang-format-14 ${FORMAT_OPTS} src/*{.c,.h}
echo "Adding missing final new lines..."
find src/ -type f -exec sed -i -e '$a\' {} \;
echo "Done formatting all files."

