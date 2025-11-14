#!/bin/bash

# ======================================
# CONFIGURATION
# ======================================
STD=c99
OPT=0
COMPILERS=("gcc" "clang")
OUT_DIR="out"
TEST_FILE="test.c"
EXECUTABLE="$OUT_DIR/test"

# ======================================
# FUNCTIONS
# ======================================

terminate() {
    rm -f "$EXECUTABLE"
    exit 0
}

test_all_comps() {
    for COMPILER in "${COMPILERS[@]}"; do
        test_build "$COMPILER"
    done
}

test_all_opt() {
    for OPT in 0 1 2 3 s fast; do
        test_all_comps
    done
}

test_build() {
    COMPILER_CMD=$1
    echo "Building with $COMPILER_CMD -std=$STD -O$OPT..."

    mkdir -p "$OUT_DIR"
    rm -f "$EXECUTABLE"

    $COMPILER_CMD -g "$TEST_FILE" -o "$EXECUTABLE" -std=$STD -O$OPT -Wall -Werror -pedantic
    if [ $? -eq 0 ]; then
        echo -e "\e[32mSuccessfully built with $COMPILER_CMD -std=$STD -O$OPT\e[0m"
    else
        echo -e "\e[31mFailed to build with $COMPILER_CMD -std=$STD -O$OPT\e[0m"
        terminate
    fi

    "$EXECUTABLE"
    if [ $? -eq 0 ]; then
        echo -e "\e[32mSuccessfully passed all tests with $COMPILER_CMD -std=$STD -O$OPT\e[0m"
    else
        echo -e "\e[31mFailed to pass all tests with $COMPILER_CMD -std=$STD -O$OPT\e[0m"
        terminate
    fi

    echo
}

# ======================================
# MAIN
# ======================================

STD=c99
test_all_opt

STD=c17
test_all_opt

terminate
