# #!/bin/bash

numClients=$1
loopNum=$2
sleepTime=$3
timeout=$4

vmstat 2 5 &
for ((i=1; i<=$numClients; i++)); do
    ./client localhost 6000 send.c $loopNum $sleepTime $timeout > client_$i.txt &
    sleep 1
done
wait

for ((i=1; i<=$numClients; i++)); do
        cat client_$i.txt
done
