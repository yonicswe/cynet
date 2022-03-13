#!/bin/bash
if [ $# -ne 1 ] ; then 
    echo "usage: ./client <socket path>"
    exit -1;
fi

./client $1
