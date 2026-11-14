# fastest-primes

Read an integer N (1 ≤ N ≤ 5,000,000) on stdin, print the number of primes
≤ N on stdout.

## Run

```sh
echo 5000000 | ./primes        # -> 348513
```

`./primes` is the run command (direct execution, lowest latency).
`run.sh` does the same but rebuilds from source first if the prebuilt
binary cannot execute on the machine (wrong architecture).

## Build

```sh
cc -O3 -static -o primes primes.c
```

Static x86-64 binary, built with the baseline arch (no `-march=native`)
so it runs on any x86-64 grader machine. `primes.c` is the full source.

## How it works

Odd-only Sieve of Eratosthenes over a byte array:

- one byte per odd number (slot `i` ↔ number `2i+1`), static BSS array
  (already zeroed — no `memset`/`malloc` at startup);
- marking starts at `p*p`, stepping `2p` in value (`p` in slot space);
- counting uses a SWAR byte-sum trick (8 bytes at a time, baseline
  integer ops only, no POPCNT/SIMD);
- raw `read()`/`write()` syscalls instead of stdio.

## Verification

All public samples pass, plus edge cases, and π(5,000,000) = 348513 was
cross-checked against an independent Python bytearray sieve (and a Node
sieve) — 92 values swept, 0 mismatches.

| N | count |
|---|---:|
| 10 | 4 |
| 100 | 25 |
| 1000 | 168 |
| 100000 | 9592 |
| 1000000 | 78498 |
| 5000000 | 348513 |

## Measured latency (this machine, AMD EPYC, 41 runs each, wall-clock)

| N | min | median |
|---|---:|---:|
| 10 | 0.37 ms | 0.44 ms |
| 1,000,000 | 1.12 ms | 1.32 ms |
| 5,000,000 | 5.12 ms | 5.46 ms |

For comparison on the same machine at N = 5,000,000: Python bytearray
sieve 45 ms min, Node `Uint8Array` sieve 101 ms min (interpreter
startup dominates both at small N: 24 ms / 80 ms at N = 10).
