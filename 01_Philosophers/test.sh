#!/bin/bash
# Test script for dining_philosophers

TARGET=./dining_philosophers

# ANSI color codes for output
GREEN='\033[32m'
RED='\033[31m'
YELLOW='\033[33m'
NC='\033[0m'  # No Color

# Define test cases: (Test ID, Philosopher Count, Duration)
TESTS=(
    "01 3 5"
    "02 5 5"
    "03 10 5"
    "04 -5 5"
    "05 0 5"
    "06 1 5"
)

declare -A TEST_RESULTS

# Check if the executable exists
if [ ! -x "$TARGET" ]; then
    printf "${YELLOW}Executable not found. Building project...${NC}\n"
    make
fi

# Run tests
for TEST in "${TESTS[@]}"; do
    read -r ID PHILOSOPHERS DURATION <<< "$TEST"
    LOGFILE="output_${ID}.log"

    printf "\n${YELLOW}[Test $ID] Running with ${PHILOSOPHERS} philosophers for ${DURATION}s...${NC}\n"

    # Use 'script' to capture output while displaying it in real-time
    script -q -t 0 "$LOGFILE" "$TARGET" "$PHILOSOPHERS" &
    pid=$!
    sleep "$DURATION"
    kill "$pid" >/dev/null 2>&1

    # Check for normal states
    if [[ "$PHILOSOPHERS" -gt 1 ]]; then
        if grep -q "thinking" "$LOGFILE" && grep -q "hungry" "$LOGFILE" && grep -q "eating" "$LOGFILE"; then
            # Verify if each philosopher has eaten at least once
            all_ate=true
            for ((i=0; i<PHILOSOPHERS; i++)); do
                if ! grep -q "Philosopher $i is eating" "$LOGFILE"; then
                    all_ate=false
                    printf "${RED}[Test $ID] Philosopher $i did not eat.${NC}\n"
                fi
            done

            if $all_ate; then
                printf "${GREEN}[Test $ID] Passed: All philosophers ate at least once.${NC}\n"
                TEST_RESULTS["$ID"]="✅ Passed"
            else
                printf "${RED}[Test $ID] Failed: Some philosophers did not eat.${NC}\n"
                TEST_RESULTS["$ID"]="❌ Failed"
            fi
        else
            printf "${RED}[Test $ID] Failed: Missing expected states.${NC}\n"
            TEST_RESULTS["$ID"]="❌ Failed"
        fi
    else
        # Edge case handling for invalid philosopher numbers
        if grep -q "At least two philosophers are required" "$LOGFILE"; then
            printf "${GREEN}[Test $ID] Passed: Proper error handling detected.${NC}\n"
            TEST_RESULTS["$ID"]="✅ Passed"
        else
            printf "${RED}[Test $ID] Failed: No error message for invalid input.${NC}\n"
            TEST_RESULTS["$ID"]="❌ Failed"
        fi
    fi
    
    # Cleanup log file
    rm -f "$LOGFILE"
done

# Summary
printf "\n${YELLOW}Test Summary:${NC}\n"
for ID in "${!TEST_RESULTS[@]}"; do
    printf "Test $ID: ${TEST_RESULTS[$ID]}\n"
done
printf "\n${GREEN}All tests completed.${NC}\n"
