#!/bin/bash
PUBLISHER_COUNT="${1:-10}"
export LD_LIBRARY_PATH=~/Fast-DDS/install/lib

kill_everything() {
    echo "Killing everything ..."
    killall -rwq "publisher|subscriber"
    echo "(done)"
}

trap on_sigint SIGINT
on_sigint() 
{
    kill_everything
    exit -1
}

# $1 : exec name
# $2 : count
start_process() {
    echo "Start $1 ($2 times)"
    for i in $(seq $2)
    do
        taskset -c 0 $1 &
    done
}

kill_everything

echo "About to start"
echo "3" ; sleep 1
echo "2" ; sleep 1
echo "1" ; sleep 1

echo "Run test for 120 sek"

start_process build/publisher $PUBLISHER_COUNT
start_process build/subscriber 1

ps -C publisher,subscriber

echo "Run test for 120 sek"
sleep 120
echo "End of test"


kill_everything

echo "Stopped"