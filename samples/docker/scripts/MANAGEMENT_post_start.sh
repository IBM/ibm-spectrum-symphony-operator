#!/bin/bash

function wait_cluster_started
{
    for I in 1 2 3 4 6 
    do
        if egosh user logon -u Admin -x Admin; then
            break;
        fi
        echo "Waiting cluster to start up $I/6"
        sleep 10s
    done
}

###########################################33

#wait_cluster_started
