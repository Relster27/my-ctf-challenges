#!/bin/bash
TIMEOUT_DEVICE=$((67*5))
while true; do
	socat TCP-LISTEN:$PORT_LISTENER,reuseaddr,fork EXEC:"timeout ${TIMEOUT_DEVICE}s bash /app/run.sh",pty,stderr,ctty,setsid
	#socat TCP-LISTEN:$PORT_LISTENER,reuseaddr,fork EXEC:"timeout ${TIMEOUT_DEVICE}s bash /app/run.sh",pty,raw,echo=0,stderr,ctty,setsid
done