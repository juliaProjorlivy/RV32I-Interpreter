#!/bin/bash

#number of arguments
len=$#

if [ $len -lt 2 ]; then
echo "Too little arguments"
fi

diff $1 $2


