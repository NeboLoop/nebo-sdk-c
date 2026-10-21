/**
 * Health proof check.
 *   proof                                   checks the shared test vector
 *   proof <nonce> <key> <sidecar> <port>    prints the proof for these inputs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"

int main(int argc, char **argv) {
    char out[65];
    if (argc == 5) {
        if (nebo_health_proof(argv[1], argv[2], argv[3], atoi(argv[4]), out) != 0) return 1;
        printf("%s\n", out);
        return 0;
    }

    static const char want[] = "f67ca363caac14eb7f345df8c7548d11a40334c55d255e30ec02339265ed8db8";
    if (nebo_health_proof("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f",
                          "fedcba9876543210fedcba9876543210fedcba9876543210fedcba9876543210",
                          "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
                          54321, out) != 0 || strcmp(out, want) != 0) {
        printf("FAIL: vector proof %s, want %s\n", out, want);
        return 1;
    }
    if (nebo_health_proof("not-hex", "k", "s", 1, out) != -1 ||
        nebo_health_proof("zz0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f", "k", "s", 1, out) != -1) {
        printf("FAIL: malformed nonce accepted\n");
        return 1;
    }
    printf("PASS\n");
    return 0;
}
