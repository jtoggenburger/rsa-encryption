#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "randstate.h"
#include "numtheory.h"
#include "rsa.h"

#define ITEMS "i:o:n:vh"

char *help_message = "SYNOPSIS\n"
                     "   Encrypts data using RSA encryption.\n"
                     "   Encrypted data is decrypted by the decrypt program.\n\n"
                     "USAGE\n"
                     "   ./encrypt [-hv] [-i infile] [-o outfile] -n pubkey\n\n"
                     "OPTIONS\n"
                     "   -h              Display program help and usage.\n"
                     "   -v              Display verbose program output.\n"
                     "   -i infile       Input file of data to encrypt (default: stdin).\n"
                     "   -o outfile      Output file for encrypted data (default: stdout).\n"
                     "   -n pbfile       Public key file (default: rsa.pub).\n";

//credit: Eugene for getopt() use
//credit: Professor Long, asgn6.pdf
int main(int argc, char **argv) {

    int opt = 0;

    //default input output files
    FILE *infile = stdin;
    FILE *outfile = stdout;
    FILE *pbfile = fopen("rsa.pub", "r");

    bool verbose = false;

    while ((opt = getopt(argc, argv, ITEMS)) != -1) {
        switch (opt) {
        case 'v': verbose = true; break;
        case 'i': infile = fopen(optarg, "r"); break;
        case 'o': outfile = fopen(optarg, "w"); break;
        case 'n': pbfile = fopen(optarg, "r"); break;
        default:
        case 'h':
            fprintf(stderr, "%s", help_message);
            fclose(pbfile);
            return 0;
        }
    }

    //begin check of file--------------------------------------------------------------------------

    if (infile == NULL) {
        fprintf(stderr, "ERROR: unable to open infile - %d\n", errno);
        fclose(pbfile);
        return 0;
    }

    if (outfile == NULL) {
        fprintf(stderr, "ERROR: unable to open outfile - %d\n", errno);
        fclose(pbfile);
        return 0;
    }

    if (pbfile == NULL) {
        fprintf(stderr, "ERROR: unable to open pbfile - %d\n", errno);
        fclose(pbfile);
        return 0;
    }

    //begin encryption-----------------------------------------------------------------------------
    //read in public key file
    mpz_t n, e, s;
    mpz_init(n); //sum of p and q
    mpz_init(e); //public exponent
    mpz_init(s); //signature

    char username[256];

    rsa_read_pub(n, e, s, username, pbfile);

    if (verbose == true) {
        fprintf(stdout, "user = %s\n", username);
        gmp_fprintf(stdout, "s (%d bits) = %Zd\n", mpz_sizeinbase(s, 2), s);
        gmp_fprintf(stdout, "n (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
        gmp_fprintf(stdout, "e (%d bits) = %Zd\n", mpz_sizeinbase(e, 2), e);
    }

    //verify username
    mpz_t m;
    mpz_init(m); //holds raw username
    mpz_set_str(m, username, 62);

    //if username could not be verified
    if (rsa_verify(m, s, e, n) != true) {
        fprintf(stderr, "ERROR: could not verify signature\n");
        fclose(pbfile);
        return 0;
    }

    rsa_encrypt_file(infile, outfile, n, e);

    //clears---------------------------------------------------------------------------------------
    fclose(pbfile);

    mpz_clear(n);
    mpz_clear(e);
    mpz_clear(s);
    mpz_clear(m);

    return 0;
}
