#!/bin/bash

set -e

for input in test_files/input*.json; do
    num="${input##*/input}"   # remove "tests/input"
    num="${num%.json}"        # remove ".json"

    echo "Running test $num..."

    expected="test_files/output${num}.json"
    output=$(./integration < "$input" | jq -S .)

    if diff -q <(echo "$output") <(jq -S . "$expected") >/dev/null; then
        echo "Test $num: PASS"
    else
        echo "Test $num: FAIL"
        diff -u \
            <(echo "$output") \
            <(jq -S . "$expected")
    fi
done
