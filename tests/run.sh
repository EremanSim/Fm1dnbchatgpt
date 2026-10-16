#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
${CC:-cc} -std=c99 -O2 -Wall -Wextra -Werror -Icore tests/test_engine.c core/break174.c -o build/test-engine
./build/test-engine

${CC:-cc} -std=c99 -O2 -Wall -Wextra -Werror -Wno-unused-function -Icore tests/test_input.c -o build/test-input
./build/test-input
