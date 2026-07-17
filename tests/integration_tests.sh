#!/bin/bash

# first run integration tests for the system.
set -e
for input in $(ls ./integration_test_files/input*.json | sort -V); do
    num="${input##*/input}"   # remove "tests/input"
    num="${num%.json}"        # remove ".json"

    echo "Running test $num..."

    expected="./integration_test_files/output${num}.json"
    output=$(./quicksilver_system_integration < "$input" | jq -S .)

    if diff -q <(echo "$output") <(jq -S . "$expected") >/dev/null; then
        echo "Test $num: PASS"
    else
        echo "Test $num: FAIL"
        diff -u \
            <(echo "$output") \
            <(jq -S . "$expected")
    fi
done

./quicksilver_as_server &

SERVER_PID=$!

python3 ./server_tests.py
TEST_RESULT=$?

if [ $TEST_RESULT -eq 0 ]; then
    echo "SERVER TEST PASSED"
else
    echo "SERVER TEST FAILED"
fi


kill -TERM $SERVER_PID
wait $SERVER_PID
