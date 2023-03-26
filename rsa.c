#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#include "numtheory.h"
#include "randstate.h"

//creates a new RSA public key
void rsa_make_pub(mpz_t p, mpz_t q, mpz_t n, mpz_t e, uint64_t nbits, uint64_t iters) {
    //figure out how many bits go in p and how many go in q
    uint64_t p_bits, q_bits;
    //perhaps use the gmp random
    p_bits = (random() % ((3 * nbits) / 4) - (nbits / 4) + 1) + (nbits / 4);
    q_bits = nbits - p_bits;

    p_bits++;

    make_prime(p, p_bits, iters);
    make_prime(q, q_bits, iters);

    //n = p * q
    mpz_mul(n, p, q);

    //totient = (p - 1)(q - 1)
    mpz_t totient, p_minus_1, q_minus_1;
    mpz_init(p_minus_1);
    mpz_init(p_minus_1);
    mpz_init(totient);

    mpz_sub_ui(p_minus_1, p, 1);
    mpz_sub_ui(q_minus_1, q, 1);

    mpz_mul(totient, p_minus_1, q_minus_1);

    mpz_clear(p_minus_1);
    mpz_clear(q_minus_1);

    //finding plublic exponent e
    //stop loop when random num coprime with phi/totient, aka gcd = 1
    mpz_t d;
    mpz_init(d);

    do {
        mpz_urandomb(e, state, nbits);
        gcd(d, e, totient);
    } while (mpz_cmp_ui(d, 1) != 0);

    mpz_clear(d);
    mpz_clear(totient);
    return;
}

//write a public key in hex-string format
void rsa_write_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile) {
    gmp_fprintf(pbfile, "%Zx\n", n);
    gmp_fprintf(pbfile, "%Zx\n", e);
    gmp_fprintf(pbfile, "%Zx\n", s);
    gmp_fprintf(pbfile, "%s\n", username);
    return;
}

//reads a public key file
void rsa_read_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile) {
    gmp_fscanf(pbfile, "%Zx %Zx %Zx %s", n, e, s, username);
    return;
}

//create a new RSA private key d
void rsa_make_priv(mpz_t d, mpz_t e, mpz_t p, mpz_t q) {
    mpz_t n, p_minus_1, q_minus_1;
    mpz_init(n);

    //creation of (p - 1) and (q - 1)
    mpz_init(p_minus_1);
    mpz_init(q_minus_1);
    mpz_sub_ui(p_minus_1, p, 1);
    mpz_sub_ui(q_minus_1, q, 1);

    //creation of n = (p - 1)(n - 1)
    mpz_mul(n, p_minus_1, q_minus_1);

    //calculation of d
    mod_inverse(d, e, n);

    mpz_clear(n);
    mpz_clear(p_minus_1);
    mpz_clear(q_minus_1);
    return;
}

//writes out a RSA private key to the file pvfile
void rsa_write_priv(mpz_t n, mpz_t d, FILE *pvfile) {
    gmp_fprintf(pvfile, "%Zx\n", n);
    gmp_fprintf(pvfile, "%Zx\n", d);
    return;
}

//reads in an RSA private key from file pvfile
void rsa_read_priv(mpz_t n, mpz_t d, FILE *pvfile) {
    gmp_fscanf(pvfile, "%Zx\n%Zx\n", n, d);
    return;
}

//encrypts message m, stores the cypher text in c
void rsa_encrypt(mpz_t c, mpz_t m, mpz_t e, mpz_t n) {
    pow_mod(c, m, e, n);
    return;
}

//encrypt the file INFILE, outputs to OUTFILE
void rsa_encrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t e) {
    //encryption must be done in blocks n - 1
    uint64_t block_size = (mpz_sizeinbase(n, 2) - 1) / 8; //block in bytes

    //array allocation
    uint8_t *block = (uint8_t *) malloc(block_size);
    block[0] = 0xFF; //padding

    //begin reading file while there are still items to read
    uint64_t bytes_read = 0;
    while ((bytes_read = fread((block + 1), 1, block_size - 1, infile)) > 0) {
        mpz_t m;
        mpz_init(m);
        mpz_import(m, bytes_read + 1, 1, 1, 1, 0, block);

        //encryption
        mpz_t c;
        mpz_init(c);
        rsa_encrypt(c, m, e, n);

        //writing encrypted values
        gmp_fprintf(outfile, "%Zx\n", c);

        //clearing/reset bytes_read
        mpz_clear(m);
        mpz_clear(c);
        bytes_read = 0;
    }

    free(block);
    return;
}

//decrypts cyphertext c into message m
void rsa_decrypt(mpz_t m, mpz_t c, mpz_t d, mpz_t n) {
    pow_mod(m, c, d, n);
    return;
}

//encrypt an entire
void rsa_decrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t d) {
    //decryption must be done in blocks n - 1
    uint64_t block_size = (mpz_sizeinbase(n, 2) - 1) / 8;

    //array allocation
    uint8_t *block = (uint8_t *) malloc(block_size);

    mpz_t c;
    mpz_init(c);
    while (gmp_fscanf(infile, "%Zx", c) != EOF) {

        uint64_t j = 0; //bytes read by export

        //decryption
        mpz_t m;
        mpz_init(m);
        rsa_decrypt(m, c, d, n);

        mpz_export(block, &j, 1, 1, 1, 0, m);

        //write out decrypted version
        fwrite((block + 1), 1, j - 1, outfile);

        mpz_clear(m);
        j = 0;
    }
    mpz_clear(c);
    free(block);
    return;
}

//sign a message m
void rsa_sign(mpz_t s, mpz_t m, mpz_t d, mpz_t n) {
    //m ^ d % n
    pow_mod(s, m, d, n);
    return;
}

//verify a username
bool rsa_verify(mpz_t m, mpz_t s, mpz_t e, mpz_t n) {
    mpz_t t;
    mpz_init(t);
    pow_mod(t, s, e, n);

    //if t and m are not equal, return 0
    if (mpz_cmp(t, m) != 0) {
        return false;
    }

    //otherwise, return true
    return true;
}
