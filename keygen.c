#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#include "randstate.h"
#include "numtheory.h"
#include "rsa.h"

#define ITEMS "b:i:n:d:s:vh"

char *help_message
    = "SYNOPSIS\n"
      "   Generates an RSA public/private key pair.\n\n"
      "USAGE\n"
      "   ./keygen [-hv] [-b bits] -n pbfile -d pvfile\n\n"
      "OPTIONS\n"
      "   -h              Display program help and usage.\n"
      "   -v              Display verbose program output.\n"
      "   -b bits         Minimum bits needed for public key n (default: 256).\n"
      "   -i confidence   Miller-Rabin iterations for testing primes (default: 50).\n"
      "   -n pbfile       Public key file (default: rsa.pub).\n"
      "   -d pvfile       Private key file (default: rsa.priv).\n"
      "   -s seed         Random seed for testing.\n";

//credit: Eugene for getopt() use
//credit: Professor Long, asgn6.pdf
int main(int argc, char **argv) {

    int opt = 0;

    //default output files for private and public keys
    FILE *pub_file = fopen("rsa.pub", "w");
    FILE *priv_file = fopen("rsa.priv", "w");

    //default options for other flags
    uint64_t modulus_bits = 256;
    bool verbose = false;
    uint64_t iters = 50;
    uint64_t seed = time(NULL);

    while ((opt = getopt(argc, argv, ITEMS)) != -1) {
        switch (opt) {
        case 'b': //sets the modulus_bits
            modulus_bits = (uint64_t) atoi(optarg);
            break;
        case 'i': //sets the iter
            iters = (uint64_t) atoi(optarg);
            break;
        case 'n': pub_file = fopen(optarg, "w"); break;
        case 'd': priv_file = fopen(optarg, "w"); break;
        case 's': //sets the seed
            seed = (uint64_t) atoi(optarg);
            break;
        case 'v': //enables verbose printing
            verbose = true;
            break;
        default:
        case 'h': //default and 'h' case print the help message and close the program
            fprintf(stderr, "%s", help_message);
            fclose(pub_file);
            fclose(priv_file);
            return 0;
        }
    }

    //begin check of output files------------------------------------------------------------------

    if (pub_file == NULL) {
        fprintf(stderr, "ERROR: unable to open public key file - %d\n", errno);
        fclose(pub_file);
        fclose(priv_file);
        return 0;
    }

    if (priv_file == NULL) {
        fprintf(stderr, "ERROR: unable to open private key file - %d\n", errno);
        fclose(pub_file);
        fclose(priv_file);
        return 0;
    }

    //ensure that the priv file is read-write for the owner only
    if (fchmod(fileno(priv_file), 0600) == -1) {
        fprintf(stderr, "ERROR: unable to change private key file permissions\n");
        fclose(pub_file);
        fclose(priv_file);
        return 0;
    }

    //begin creation of keys-----------------------------------------------------------------------
    //initialize randstate with value seed
    randstate_init(seed);

    //initializing of mpz_t for public key
    mpz_t p, q, n, e, s;
    mpz_init(p); //prime1
    mpz_init(q); //prime2
    mpz_init(n); //product of prime1 and prime2
    mpz_init(e); //public exponent
    mpz_init(s); //signature

    //creation of public key
    rsa_make_pub(p, q, n, e, modulus_bits, iters);

    //creation of private key
    mpz_t d; //private key
    mpz_init(d);
    rsa_make_priv(d, e, p, q);

    //creation of signature
    mpz_t m; //holds the raw username
    mpz_init(m);
    char *username = getenv("USER");
    mpz_set_str(m, username, 62);
    rsa_sign(s, m, d, n);

    //writing of keys and verbose output-----------------------------------------------------------
    //writing of keys
    rsa_write_pub(n, e, s, username, pub_file);
    rsa_write_priv(n, d, priv_file);

    if (verbose == true) {
        fprintf(stdout, "user = %s\n", username);
        gmp_fprintf(stdout, "s (%d bits) = %Zd\n", mpz_sizeinbase(s, 2), s);
        gmp_fprintf(stdout, "p (%d bits) = %Zd\n", mpz_sizeinbase(p, 2), p);
        gmp_fprintf(stdout, "q (%d bits) = %Zd\n", mpz_sizeinbase(q, 2), q);
        gmp_fprintf(stdout, "n (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
        gmp_fprintf(stdout, "e (%d bits) = %Zd\n", mpz_sizeinbase(e, 2), e);
        gmp_fprintf(stdout, "d (%d bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
    }

    //closing and clearing-------------------------------------------------------------------------
    fclose(pub_file);
    fclose(priv_file);

    randstate_clear();

    mpz_clear(p);
    mpz_clear(q);
    mpz_clear(n);
    mpz_clear(e);
    mpz_clear(s);
    mpz_clear(d);
    mpz_clear(m);

    return 0;
}
