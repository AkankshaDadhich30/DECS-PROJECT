#!/bin/bash

# Function to submit a new request
submit_new_request() {
    
    local source_code_file=$1

    # Send new request to the server
    ./client  localhost 5000  new $source_code_file &

    # Return the request ID
}

# Function to check the status of a request
check_request_status() {
    local request_id=$1

    # Send status request to the server
    ./client  localhost 5000 status $request_id &
}

# Function to measure performance for a single client
measure_performance() {
    
    local source_code_file=$1
    local polling_interval=$2

    # Submit a new request and get the request ID
    local request_id=$(submit_new_request $source_code_file)
    echo "Server Response: IN QUEUE REQUEST ID GIVEN: $request_id"

    local start_time=$(date +%s.%N)

    # Poll the server until the status is 'grading done'
    while true; do
        local status_response=$(check_request_status $request_id)
        echo "Server Response for request id: $request_id is $status_response"
        if [[ $status_response == *"results"* ]]; then
            local end_time=$(date +%s.%N)
            local response_time=$(echo "$end_time - $start_time" | bc)
            echo "Response time for request id: $request_id is $response_time seconds"
            break
        fi

        sleep $polling_interval
    done
}

# Main script
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <server_address> <source_code_file> <polling_interval> <num_clients>"
    exit 1
fi


source_code_file=$1
polling_interval=$2
num_clients=$3

# Run performance measurement for each client
for ((i=1; i<=$num_clients; i++)); do
    echo "===== Client $i ====="
    measure_performance $source_code_file $polling_interval &
done

# Wait for all background processes to finish
wait

#rm compile_*
#rm runtime_*
#rm file_*
#rm diff_*
