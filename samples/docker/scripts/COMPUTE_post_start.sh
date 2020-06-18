#!/bin/bash

function wait_cluster_started
{
    for I in 1 2 3 4 6 
    do
        if egosh user logon -u Admin -x Admin 2>/dev/null; then
            break;
        fi
        echo "Waiting cluster to start up $I/6"
        sleep 10s
    done
}

###########################################

#wait_cluster_started
