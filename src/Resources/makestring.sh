#!/bin/bash

NAME=$1
NAME=${NAME//ico/}
NAME=${NAME//ICO/}
NAME=${NAME/./}
echo $NAME ICON DISCARDABLE \"$1\"