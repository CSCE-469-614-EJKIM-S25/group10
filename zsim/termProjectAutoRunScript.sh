#!/bin/bash

# Log file
LOG_FILE="termProjectRunScript.log"
RUN_SCRIPT="./termProjectRunScript"  # Path to the script

# Define suites and benchmarks
# EDIT: Taken out PARSEC: dedup since it's buggy
declare -A SUITES
SUITES["SPEC"]="bzip2 gcc mcf hmmer sjeng libquantum xalan milc cactusADM leslie3d namd soplex calculix lbm"
SUITES["PARSEC"]="blackscholes bodytrack canneal fluidanimate freqmine streamcluster swaptions x264"

# Define replacement policies
REPLACEMENT_POLICIES=("Mockingjay")

# Forbidden patterns
FORBIDDEN_PATTERNS=(
    "./build/opt/zsim configs/termProject/\$repl/\${bench}.cfg > outputs/termProject/\$repl/\${bench}/\${bench}.log 2>&1 &"
    "./build/opt/zsim configs/termProject/\$repl/\${bench}_8c_simlarge.cfg > outputs/termProject/\$repl/\${bench}_8c_simlarge/\${bench}.log 2>&1 &"
)

# Check for forbidden patterns
for pattern in "${FORBIDDEN_PATTERNS[@]}"; do
    if grep -Fq "$pattern" "$RUN_SCRIPT"; then
        echo "Error: Forbidden pattern detected in $RUN_SCRIPT!" | tee -a "$LOG_FILE"
        echo "Ensure simulations do not run in the background. Exiting." | tee -a "$LOG_FILE"
		echo "Fix: Ensure to eliminate the & in both the echo string and command"
        exit 1
    fi
done

# Signal handler for SIGTERM and SIGINT
handle_signal() {
    echo "$(date) - Received termination signal. Exiting script." | tee -a "$LOG_FILE"
    exit 1
}

# Trap signals
trap handle_signal SIGTERM SIGINT

# Start logging
echo "$(date) - Script started" >> "$LOG_FILE"
echo "$(date) - Script started" 

# Iterate over each combination and execute the command
for suite in "${!SUITES[@]}"; do
    for benchmark in ${SUITES[$suite]}; do
        for policy in "${REPLACEMENT_POLICIES[@]}"; do
            
            # Skip SPEC benchmarks with LRU policy (since they have already been run)
#            if [[ "$suite" == "SPEC" && "$policy" == "LRU" ]]; then
#                echo "$(date) - Skipping: ./hw4runscript $suite $benchmark $policy (Already executed)" | tee -a "$LOG_FILE"
#                continue
#            fi
            
            COMMAND="./termProjectRunScript $suite $benchmark $policy"
            echo "$(date) - Executing: $COMMAND" | tee -a "$LOG_FILE"
            $COMMAND >> "$LOG_FILE" 2>&1
            echo "$(date) - Finished: $COMMAND" | tee -a "$LOG_FILE"
        done
    done
done

echo "$(date) - Script finished successfully" | tee -a "$LOG_FILE"

