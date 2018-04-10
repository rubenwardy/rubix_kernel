#!/bin/sh

(( speaker-test -t sine -f 1000 )& pid=$! ; sleep 0.1s ; kill -9 $pid) > /dev/null
sleep 3
(( speaker-test -t sine -f 1000 )& pid=$! ; sleep 0.1s ; kill -9 $pid) > /dev/null
echo -e "p3\r"
