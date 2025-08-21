/*
 * crypto function to calculate SHA256 and CRC32
 *
 * Copyright (C) 2025, Liang Cheng
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * direct implement of SHA256 based on
 *      publication number: FIPS PUB 180-4
 *      title: Secure Hash Standard (SHS)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <endian.h>

/* for test crypto function only */
#define DEBUG           0
#define TEST_MAIN       0

#define WIDTH           32        /* for 32-bits */
/*
 * operation macro
 * NOTE: use () to avoid trouble. Lesson learned...
 */
#define SHR(x, n)       ((x) >> (n))
#define ROTR(x, n)      (SHR(x, n) | ((x) << (WIDTH - n)))
#define ROTL(x, n)      (((x) << n) | SHR(x, (WIDTH - n)))
#define SIG0_256(x)     (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define SIG1_256(x)     (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define DLT0_256(x)     (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define DLT1_256(x)     (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))

struct m_block_t {
    uint32_t m[16]; /* M[0] ~ M[15] */
};

static uint32_t Ch(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ ((~x) & z);
}

static uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) {
    return ((x & y) ^ (x & z) ^ (y & z));
}

/* len is in bytes */
void calc_sha256(uint8_t *p_msg, uint32_t len, uint32_t *p_sha) {
    uint32_t n_patch = 0;
    uint32_t byte_n;
    uint8_t *p_new = NULL;
    // first 32 bits of the fractional parts of the cube roots of the first 64 primes
    const static uint32_t K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };
    // first 32 bits of the fractional parts of the square roots of the first 8 primes
    // XXX: do_NOT_declare H as static as this array changes over the time.
    // It can have negative impact on subsequent call.
    uint32_t H[8] ={
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    struct m_block_t *p_m_block;
    uint32_t i, t, N;
    uint32_t W[64] = {0};

    /* Step 1: --- pre-processing --- */
    /* padding: msg + 0b1 + 0b0 + .... + (64 bit) of len */
    /* calculate the number of zeroes padding */
    n_patch = ((len * 8 + 1 + 64) / 512 + 1)* 512 - (len * 8 + 1 + 64);
    while ((len * 8 + 1 + n_patch + 64) % 512 != 0) {
        n_patch++;
    }
    /* (n_patch + 1) is diviable by 8 */
    assert((n_patch + 1) % 8 == 0);
    byte_n = len + (n_patch + 1) / 8 + 64 / 8;
#if DEBUG
    printf(" len = %d n_patch bits = %d byte_n = %d\n", len, n_patch, byte_n);
#endif

    assert((byte_n % 64) == 0);

    p_new = malloc(byte_n);
    memset(p_new, 0, byte_n);
    memcpy(p_new, p_msg, len);
    *((char *)p_new + len) = 0x80;
    {
        /* padding the bits of len */
        uint32_t val;
        val = len * 8;
        i = 1;
        while (val != 0) {
            *(p_new + byte_n - i) = val & 0xFF;
            val = val >> 8;
            i++;
        }
    }

    /* Step 2: --- parsing the messages  --- */
    p_m_block = (struct m_block_t *)(void *)&p_new[0];

    /* step 3: --- calculate hash --- */
    N = byte_n / 64;
    for (i = 0; i < N; i++) {
        uint32_t a, b, c, d, e, f, g, h;

        for (t = 0; t <=15; t++) {
            /* convert to big edian */
            W[t] = htobe32(p_m_block->m[t]);
        }

        /* prepare the message schedule W[t] */
        for (t = 16; t <= 63; t++) {
            W[t] = DLT1_256(W[t-2])+ W[t-7] + DLT0_256(W[t-15]) + W[t-16];
        }
        /* initialize the eight working variables a ~ h */
        a = H[0]; b = H[1]; c = H[2]; d = H[3];
        e = H[4]; f = H[5]; g = H[6]; h = H[7];
#if DEBUG
        for (t = 0; t <= 63; t++) {
            printf("W[%d] = %08x \n", t, W[t]);
        }
        printf("\n");
        printf("%08x %08x %08x %08x %08x %08x %08x %08x\n",
                    a, b, c, d, e, f, g, h);
#endif
        for (t = 0; t <= 63; t++) {
            uint32_t T[2] = {0};

            T[0] = h + SIG1_256(e) + Ch(e, f, g) + K[t] + W[t];
            T[1] = SIG0_256(a) + Maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + T[0];
            d = c;
            c = b;
            b = a;
            a = T[0] + T[1];
#if DEBUG
            printf("t = %d, %08x %08x %08x %08x %08x %08x %08x %08x\n",
                    t, a, b, c, d, e, f, g, h);
#endif
        }
        /* compute the t_th intermediate hash value H'i */
        H[0] = a + H[0];
        H[1] = b + H[1];
        H[2] = c + H[2];
        H[3] = d + H[3];
        H[4] = e + H[4];
        H[5] = f + H[5];
        H[6] = g + H[6];
        H[7] = h + H[7];

        p_m_block++;
    }

    /* the hash is H[0] || H[1]....||H[7] */
    memcpy(p_sha, &H[0], sizeof(H));
    return;
}

/*
 * https://zlib.net/crc_v3.txt
 * for fun:
 * https://stackoverflow.com/questions/2587766/how-is-a-crc32-checksum-calculated
 */
uint32_t calc_crc32(const char *src, uint32_t sz)
{
    char ch;
    uint32_t crc = ~0;
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t b = 0;

    printf("crc = 0x%x\n", crc);
    for(i = 0; i < sz; i++) {
        ch = src[i];
        for(j = 0; j < 8; j++) {
            b = (ch ^ crc) & 1;
            crc >>= 1;
            if (b) {
                crc=crc^0xEDB88320;
            }
            ch >>= 1;
        }
    }

    printf("crc = %x\n", ~crc);
    return ~crc;
}
#if TEST_MAIN
static uint8_t huge_a[1000000];
int main(int argc, char * argv[]) {
    uint32_t sha[8] = {0};
    uint8_t *p_test = NULL;
    int i = 0;

    /* test data are from https://csrc.nist.gov/pubs/fips/180-2/final */
    calc_sha256("abc", strlen("abc"), &sha[0]);
    printf("SHA(\"abc\") = %08x %08x %08x %08x %08x %08x %08x %08x\n",
            sha[0], sha[1], sha[2], sha[3], sha[4], sha[5], sha[6], sha[7]);
    calc_sha256("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
            strlen("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"), &sha[0]);
    printf("SHA(\"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq\") = %08x %08x %08x %08x %08x %08x %08x %08x\n",
            sha[0], sha[1], sha[2], sha[3], sha[4], sha[5], sha[6], sha[7]);

    for (i = 0; i < 1000000; i++) {
        huge_a[i] = 'a';
    }
    calc_sha256(huge_a, 1000000, &sha[0]);
    printf("SHA(\"a....\") = %08x %08x %08x %08x %08x %08x %08x %08x\n",
            sha[0], sha[1], sha[2], sha[3], sha[4], sha[5], sha[6], sha[7]);
    return 0;
}
#endif
