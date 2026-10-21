#!/bin/sh
# Runs a sidecar the way Nebo does and checks the contract:
# announce line on stdout, token required on every call, exit on stdin EOF
# and on SIGTERM.
# Usage: sidecar_test.sh <sidecar binary> <probe binary>
set -eu

APP=$1
PROBE=$2
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
mkfifo "$WORK/stdin"

TOKEN=0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef

# Refuses to start without a token.
if NEBO_APP_TOKEN= "$APP" </dev/null >/dev/null 2>&1; then
    echo "FAIL: started without NEBO_APP_TOKEN"; exit 1
fi

NEBO_APP_TOKEN=$TOKEN NEBO_DATA_DIR=$WORK "$APP" <"$WORK/stdin" >"$WORK/stdout" &
PID=$!
exec 3>"$WORK/stdin"   # hold stdin open, like Nebo

i=0
until grep -q '^nebo-app-listening ' "$WORK/stdout"; do
    i=$((i + 1)); [ $i -lt 100 ] || { echo "FAIL: no announce line"; exit 1; }
    sleep 0.1
done
ADDR=$(sed -n 's/^nebo-app-listening \(127\.0\.0\.1:[0-9][0-9]*\)$/\1/p' "$WORK/stdout")
[ -n "$ADDR" ] || { echo "FAIL: bad announce line: $(cat "$WORK/stdout")"; exit 1; }
echo "announced $ADDR"

# 16 = UNAUTHENTICATED, 0 = OK
[ "$("$PROBE" "$ADDR")" = 16 ] || { echo "FAIL: call without token not refused"; exit 1; }
[ "$("$PROBE" "$ADDR" wrong)" = 16 ] || { echo "FAIL: call with wrong token not refused"; exit 1; }
[ "$("$PROBE" "$ADDR" "$TOKEN")" = 0 ] || { echo "FAIL: call with token refused"; exit 1; }

exec 3>&-   # close stdin: the sidecar must exit
i=0
while kill -0 $PID 2>/dev/null; do
    i=$((i + 1)); [ $i -lt 50 ] || { kill $PID; echo "FAIL: still running after stdin closed"; exit 1; }
    sleep 0.1
done
wait $PID || { echo "FAIL: non-zero exit after stdin closed"; exit 1; }

# SIGTERM stops it cleanly while stdin stays open.
: >"$WORK/stdout"
NEBO_APP_TOKEN=$TOKEN "$APP" <"$WORK/stdin" >"$WORK/stdout" &
PID=$!
exec 3>"$WORK/stdin"
i=0
until grep -q '^nebo-app-listening ' "$WORK/stdout"; do
    i=$((i + 1)); [ $i -lt 100 ] || { echo "FAIL: no announce line"; exit 1; }
    sleep 0.1
done
kill -TERM $PID
wait $PID || { echo "FAIL: non-zero exit after SIGTERM"; exit 1; }
exec 3>&-
echo "PASS"
