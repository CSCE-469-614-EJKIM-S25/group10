#!/bin/bash

# Log file
LOG_FILE="hw4runscript.log"

# Define suites and benchmarks
declare -A SUITES
SUITES["SPEC"]="bzip2 gcc mcf hmmer sjeng libquantum xalan cactusADM namd soplex calculix lbm"
SUITES["PARSEC"]="blackscholes bodytrack canneal fluidanimate freqmine streamcluster swaptions x264"

# Define replacement policies
# Put "SRRIP" back in the REPLACEMENT_POLICIES
REPLACEMENT_POLICIES=("SRRIP")

# Signal handler for SIGTERM and SIGINT
handle_signal() {
    echo "$(date) - Received termination signal. Exiting script." | tee -a "$LOG_FILE"
    exit 1
}

# Trap signals
trap handle_signal SIGTERM SIGINT

# Start logging
echo "$(date) - Script started" >> "$LOG_FILE"

# Iterate over each combination and execute the command
for suite in "${!SUITES[@]}"; do
    for benchmark in ${SUITES[$suite]}; do
        for policy in "${REPLACEMENT_POLICIES[@]}"; do
            
            # # Skip SPEC benchmarks with LRU policy (since they have already been run)
            # if [[ "$suite" == "SPEC" && "$policy" == "LRU" ]]; then
            #     echo "$(date) - Skipping: ./hw4runscript $suite $benchmark $policy (Already executed)" | tee -a "$LOG_FILE"
            #     continue
            # fi

            # skip first 3 parsec benchmarks for LRU
            # if [[ "$suite" == "PARSEC" && "$policy" == "LRU" && ("$benchmark" == "blackscholes" || "$benchmark" == "bodytrack" || "$benchmark" == "canneal") ]]; then
            #     echo "$(date) - Skipping: ./hw4runscript $suite $benchmark $policy (Already executed)" | tee -a "$LOG_FILE"
            #     continue
            # fi
            
            COMMAND="./hw4runscript $suite $benchmark $policy"
            echo "$(date) - Executing: $COMMAND" | tee -a "$LOG_FILE"
            $COMMAND >> "$LOG_FILE" 2>&1
            echo "$(date) - Finished: $COMMAND" | tee -a "$LOG_FILE"
        done
    done
done

echo "$(date) - Script finished successfully" | tee -a "$LOG_FILE"