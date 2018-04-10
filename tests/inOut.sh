#!/bin/sh

sleep 3

echo "Starting NC connection..."

coproc ./tests/scriptRunner.sh
nc 127.0.0.1 1235 <&"${COPROC[0]}" >&"${COPROC[1]}"
