# #!/bin/bash

numClients=$1
loopNum=$2
sleepTime=$3
timeout=$4


vmstat 2 10 &
for ((i=1; i<=$numClients; i++)); do
    ./client localhost 5000 send.c $loopNum $sleepTime $timeout > client_$i.txt &
    #sleep 1
done
wait

for ((i=1; i<=$numClients; i++)); do
        cat client_$i.txt
done

rm compile_*
rm runtime_*
rm file_*
rm out_*
