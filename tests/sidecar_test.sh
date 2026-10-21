#!/bin/sh
# Runs sidecars the way Nebo does and checks the contract: announce line on
# stdout, NEBO_SIDECAR_TOKEN required on every call (NEBO_APP_TOKEN is
# outbound only and refused inbound), Nebo's UIService.HealthCheck probe,
# exit on stdin EOF and on SIGTERM.
# Usage: sidecar_test.sh <tool app> <ui app> <probe>
set -eu

TOOL_APP=$1
UI_APP=$2
PROBE=$3
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
mkfifo "$WORK/stdin"

SIDECAR_TOKEN=0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
APP_TOKEN=fedcba9876543210fedcba9876543210fedcba9876543210fedcba9876543210

fail() { echo "FAIL: $*"; exit 1; }

# Starts $1 with stdin held open on fd 3 and sets PID and ADDR.
start() {
    : >"$WORK/stdout"
    NEBO_SIDECAR_TOKEN=$SIDECAR_TOKEN NEBO_APP_TOKEN=$APP_TOKEN NEBO_DATA_DIR=$WORK \
        "$1" <"$WORK/stdin" >"$WORK/stdout" &
    PID=$!
    exec 3>"$WORK/stdin"
    i=0
    until grep -q '^nebo-app-listening ' "$WORK/stdout"; do
        i=$((i + 1)); [ $i -lt 100 ] || fail "no announce line from $1"
        sleep 0.1
    done
    ADDR=$(sed -n 's/^nebo-app-listening \(127\.0\.0\.1:[0-9][0-9]*\)$/\1/p' "$WORK/stdout")
    [ -n "$ADDR" ] || fail "bad announce line: $(cat "$WORK/stdout")"
    echo "$1 announced $ADDR"
}

# expect <code> <method> [token]   (0 = OK, 12 = UNIMPLEMENTED, 16 = UNAUTHENTICATED)
expect() {
    code=$1; shift
    got=$("$PROBE" "$ADDR" "$@")
    [ "$got" = "$code" ] || fail "$* -> $got, want $code"
}

# Refuses to start without the sidecar token, even with the app token set.
if NEBO_APP_TOKEN=$APP_TOKEN "$TOOL_APP" </dev/null >/dev/null 2>&1; then
    fail "started without NEBO_SIDECAR_TOKEN"
fi

start "$TOOL_APP"
expect 16 tool-name
expect 16 tool-name wrong
expect 16 tool-name "$APP_TOKEN"
expect 0 tool-name "$SIDECAR_TOKEN"
expect 16 ui-health
expect 12 ui-health "$SIDECAR_TOKEN"   # no UI: Nebo counts UNIMPLEMENTED as healthy

exec 3>&-   # close stdin: the sidecar must exit
i=0
while kill -0 $PID 2>/dev/null; do
    i=$((i + 1)); [ $i -lt 50 ] || { kill $PID; fail "still running after stdin closed"; }
    sleep 0.1
done
wait $PID || fail "non-zero exit after stdin closed"

start "$UI_APP"
expect 16 ui-health
expect 16 ui-health "$APP_TOKEN"
expect 0 ui-health "$SIDECAR_TOKEN"

kill -TERM $PID   # SIGTERM stops it cleanly while stdin stays open
wait $PID || fail "non-zero exit after SIGTERM"
exec 3>&-
echo "PASS"
