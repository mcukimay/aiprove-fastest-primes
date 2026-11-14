/* fastest-primes: count primes <= N (1 <= N <= 5,000,000).
 *
 * Odd-only Sieve of Eratosthenes over a byte array, with:
 *  - static BSS array (already zeroed, no memset/malloc at startup)
 *  - raw read()/write() syscalls instead of stdio (no scanf/printf overhead)
 *  - marking starts at p*p, stepping 2p in value == p in index space
 *  - single counting pass over the odd slots
 *
 * Build:  cc -O3 -static -o primes primes.c   (x86-64 baseline, no -march=native
 *         so the binary runs on any x86-64 grader machine)
 * Run:    echo 5000000 | ./primes            -> 348513
 */
#include <unistd.h>

#define MAXN 5000000

static unsigned char is_comp[(MAXN / 2) + 1];

/* Count zero bytes in a[0..n). The array only holds 0/1, so count the 1s
 * (composites) 8 bytes at a time with the byte-sum multiply trick --
 * ((w * 0x0101..) >> 56) is the exact sum of w's 8 bytes when each byte
 * is 0 or 1 -- and subtract from n. Pure baseline integer ops, no
 * POPCNT / SIMD, so it stays portable to any x86-64.
 *
 * (The "has-zero-byte" SWAR test ((w - 0x0101..) & ~w & 0x8080..) is NOT
 * used here: borrow from a zero byte can set the next byte's top bit when
 * that byte is 0x01, so it overcounts. It only detects existence.) */
static long count_zero_bytes(const unsigned char *a, long n)
{
    long comp = 0;
    long i = 0;

    for (; i + 8 <= n; i += 8) {
        unsigned long long w;
        __builtin_memcpy(&w, a + i, 8);
        comp += (long)((w * 0x0101010101010101ULL) >> 56);
    }
    for (; i < n; i++)
        comp += a[i];
    return n - comp;
}

static void print_long(long x)
{
    char out[24];
    char rev[24];
    int len = 0, t = 0;

    if (x == 0) {
        out[len++] = '0';
    } else {
        while (x > 0) {
            rev[t++] = (char)('0' + (x % 10));
            x /= 10;
        }
        while (t > 0)
            out[len++] = rev[--t];
    }
    out[len++] = '\n';
    /* Ignore the return: stdout is the whole point of the program, and
     * there is nothing useful to do if it fails. */
    (void)!write(1, out, (unsigned)len);
}

int main(void)
{
    char buf[32];
    long n = 0;
    int seen = 0;
    ssize_t r = read(0, buf, sizeof(buf) - 1);

    if (r > 0) {
        ssize_t i;
        for (i = 0; i < r; i++) {
            char c = buf[i];
            if (c >= '0' && c <= '9') {
                n = n * 10 + (c - '0');
                seen = 1;
            } else if (seen) {
                break;
            }
        }
    }
    (void)seen;

    if (n < 2) {
        print_long(0);
        return 0;
    }

    /* Odd numbers <= n are 2*i+1 for i in [0, n_odds). Slot i holds the
     * compositeness of 2*i+1. Slot 0 is the number 1 (never prime, never
     * counted). */
    long n_odds = (n + 1) / 2;
    long i, j;

    for (i = 1;; i++) {
        long p = 2 * i + 1;
        if (p * p > n)
            break;
        if (!is_comp[i]) {
            /* p*p is odd; its slot is (p*p - 1)/2 == p*p/2 (integer div).
             * Multiples 2p apart in value are p apart in slot space. */
            long start = (p * p) / 2;
            for (j = start; j < n_odds; j += p)
                is_comp[j] = 1;
        }
    }

    /* Count the odd primes (slots 1..n_odds-1); +1 for the prime 2. */
    print_long(count_zero_bytes(is_comp + 1, n_odds - 1) + 1);
    return 0;
}
