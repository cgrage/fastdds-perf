#!/bin/bash
PUBLISHER_COUNT="${1:-10}"
export LD_LIBRARY_PATH=~/Fast-DDS/install/lib

kill_everything() {
    echo "Killing everything"
    killall -rwq "publisher|subscriber"
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
        # slow-start
        sleep 0.5
    done
}

kill_everything

echo "Starting processes"

start_process build/subscriber 1
start_process build/publisher $PUBLISHER_COUNT

ps -C publisher,subscriber

echo "Run test for 120 sek"
#sleep 120
mpstat -P ALL 2 60
echo "End of test"


kill_everything

echo "Stopped"