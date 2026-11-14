#!/bin/sh
# Run the entry. Preferred invocation is ./primes directly (lowest
# latency); this script is the fallback path: if the prebuilt static
# binary cannot execute on this machine (wrong architecture), rebuild
# it from primes.c with the local C compiler and run that instead.
DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
"$DIR/primes" "$@"
rc=$?
if [ "$rc" -eq 126 ] || [ "$rc" -eq 127 ]; then
    cc -O3 -o "$DIR/primes" "$DIR/primes.c" && exec "$DIR/primes" "$@"
fi
exit "$rc"
