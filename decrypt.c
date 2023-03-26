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
                     "   Decrypts data using RSA encryption.\n"
                     "   Encrypted data is encrypted by the encrypt program.\n\n"
                     "USAGE\n"
                     "   ./decrypt [-hv] [-i infile] [-o outfile] -n privkey\n\n"
                     "OPTIONS\n"
                     "   -h              Display program help and usage.\n"
                     "   -v              Display verbose program output.\n"
                     "   -i infile       Input file of data to decrypt (default: stdin).\n"
                     "   -o outfile      Output file for decrypted data (default: stdout).\n"
                     "   -n pvfile       Private key file (default: rsa.priv).\n";

//credit: Eugene for getopt() use
//credit: Professor Long, asgn6.pdf
int main(int argc, char **argv) {

    int opt = 0;

    //default input output files
    FILE *infile = stdin;
    FILE *outfile = stdout;
    FILE *pvfile = fopen("rsa.priv", "r");

    bool verbose = false;

    while ((opt = getopt(argc, argv, ITEMS)) != -1) {
        switch (opt) {
        case 'v': verbose = true; break;
        case 'i': infile = fopen(optarg, "r"); break;
        case 'o': outfile = fopen(optarg, "w"); break;
        case 'n': pvfile = fopen(optarg, "r"); break;
        default:
        case 'h':
            fprintf(stderr, "%s", help_message);
            fclose(pvfile);
            return 0;
        }
    }

    //open private keys----------------------------------------------------------------------------

    if (infile == NULL) {
        fprintf(stderr, "ERROR: unable to open infile - %d\n", errno);
        fclose(pvfile);
        return 0;
    }

    if (outfile == NULL) {
        fprintf(stderr, "ERROR: unable to open outfile - %d\n", errno);
        fclose(pvfile);
        return 0;
    }

    if (pvfile == NULL) {
        fprintf(stderr, "ERROR: unable to open pvfile - %d\n", errno);
        fclose(pvfile);
        return 0;
    }

    //begin decryption-----------------------------------------------------------------------------
    //read in private key file
    mpz_t n, d;
    mpz_init(n); //public modulus
    mpz_init(d); //private key

    rsa_read_priv(n, d, pvfile);

    if (verbose == true) {
        gmp_fprintf(stdout, "n (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
        gmp_fprintf(stdout, "d (%d bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
    }

    rsa_decrypt_file(infile, outfile, n, d);

    //delete and clear-----------------------------------------------------------------------------
    mpz_clear(n);
    mpz_clear(d);

    fclose(pvfile);
    return 0;
}
