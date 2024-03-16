# VERSION 1
gcc gradingServerQ.c -o server
gcc submitQ.c -o client
./server <portNo>
./client <serverIP> <portNo> <filename> <loopNum> <sleepNum> <timeout>

# VERSION 2
gcc gradingServerQ.c -o server -pthread
gcc submitQ.c -o client
./server <portNo>
./client <serverIP> <portNo> <filename> <loopNum> <sleepNum> <timeout>

# VERSION 3
gcc filecalls.c queuecalls.c gradingServerQ.c -o server -pthread
gcc filecalls.c submitQ.c -o client
./server <portNo> <poolsize>
./client <serverIP> <portNo> <filename> <loopNum> <sleepNum> <timeout>

# VERSION 4
gcc filecalls.c queuecalls.c gradingServerQ.c -o server -pthread
gcc filecalls.c submitQ.c -o client
./server <portNo>
./client <serverIP> <portNo> <new or status> <filename or reqID>
