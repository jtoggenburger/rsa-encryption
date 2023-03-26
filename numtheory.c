#include <gmp.h>
#include <stdio.h>
#include <stdbool.h>

#include "numtheory.h"
#include "randstate.h"

//performs modular exponentiation out = (base ^ power) % modulus
void pow_mod(mpz_t out, mpz_t base, mpz_t exponent, mpz_t modulus) {
    mpz_t p, d;
    mpz_init_set(p, base); //sets p = based
    mpz_init_set(d, exponent); //set d = exponent
    mpz_set_ui(out, 1); //v within the pseudocode

    while (mpz_cmp_ui(d, 0) > 0) {
        mpz_t temp; //temp variable to hold d
        mpz_init_set(temp, d);
        mpz_mod_ui(temp, temp, 2);
        if (mpz_cmp_ui(temp, 1) == 0) { //if d % 2 == 1
            //out = (out * p) % modulus
            mpz_mul(out, out, p);
            mpz_mod(out, out, modulus);
        }
        //p = (p * p) % modulus
        mpz_mul(p, p, p);
        mpz_mod(p, p, modulus);

        //d = d / 2
        mpz_fdiv_q_ui(d, d, 2);
        mpz_clear(temp);
    }
    mpz_clear(p);
    mpz_clear(d);
    return;
}

//tests if n is prime through an approximation iters times
bool is_prime(mpz_t n, uint64_t iters) {
    //hardcoded cases for 0 - 4
    if (mpz_cmp_ui(n, 0) == 0) {
        return false;
    }

    if (mpz_cmp_ui(n, 1) == 0) {
        return false;
    }
    if (mpz_cmp_ui(n, 2) == 0) {
        return true;
    }
    if (mpz_cmp_ui(n, 3) == 0) {
        return true;
    }
    if (mpz_cmp_ui(n, 4) == 0) {
        return false;
    }

    //write n = 2^s * r + 1 such that r is odd
    mpz_t r, r_temp, s;
    mpz_init(r);
    mpz_init_set_ui(s, 0);

    //r = (n - 1) / 2^s·
    //do-while r is not odd
    do {
        mpz_add_ui(s, s, 1); //s++
        mpz_t n_temp; //temp variable to hold alterations to n
        mpz_init_set(n_temp, n);
        mpz_sub_ui(n_temp, n_temp, 1); // (n - 1)

        mpz_t s_temp, one; //temporary variable to hold alterations to s
        mpz_init_set(s_temp, s);
        mpz_init_set_ui(one, 1); //mpz representation of 2 THERE HAS TO BE A BETTER WAY
        mp_bitcnt_t s_temp_bit = mpz_get_ui(s_temp);
        mpz_mul_2exp(s_temp, one, s_temp_bit); // 2^s

        mpz_tdiv_q(r, n_temp, s_temp); //(n - 1) / (2^s)
        //comparison check to see if r is odd
        mpz_init_set(r_temp, r);
        mpz_mod_ui(r_temp, r_temp, 2);
        //gmp_printf("r_temp before test is %Zd \n", r_temp);

        //clears
        mpz_clear(n_temp);
        mpz_clear(s_temp);
        mpz_clear(one);
    } while (mpz_cmp_ui(r_temp, 1) != 0); //while r % 2 == 1
    //clear temp variables from do-while
    mpz_clear(r_temp);

    for (uint64_t i = 1; i <= iters; i++) {
        //choose random a within {2,3,...,n-2}
        mpz_t a, n_minus_three;
        mpz_init(a);
        mpz_init_set(n_minus_three, n);
        mpz_sub_ui(n_minus_three, n, 3);
        mpz_urandomm(a, state, n_minus_three);
        mpz_add_ui(a, a, 2); //turns the range into [2, n-2]

        //y = power-mod(a, r, n)
        mpz_t y;
        mpz_init(y);
        pow_mod(y, a, r, n);

        //if y != 1 and y != n - 1
        mpz_t n_minus_1;
        mpz_init(n_minus_1);
        mpz_sub_ui(n_minus_1, n, 1);

        if ((mpz_cmp_ui(y, 1) != 0) && (mpz_cmp(y, n_minus_1) != 0)) {
            // j <-- 1
            mpz_t j;
            mpz_init_set_ui(j, 1);

            //while j <= s - 1 and y != n - 1
            mpz_t s_minus_1;
            mpz_init(s_minus_1);
            mpz_sub_ui(s_minus_1, s, 1);

            while ((mpz_cmp(j, s_minus_1) <= 0) && (mpz_cmp(y, n_minus_1) != 0)) {
                //power-mod(y, 2, n)
                mpz_t two;
                mpz_init_set_ui(two, 2);
                pow_mod(y, y, two, n);
                //mpz_clear(two);

                //if y == 1
                if (mpz_cmp_ui(y, 1) == 0) {
                    return false;
                }

                //j <-- j + 1
                mpz_add_ui(j, j, 1);

                //clears from while loop
                mpz_clear(two);
            }

            //if y!= n - 1
            if (mpz_cmp(y, n_minus_1) != 0) {
                return false;
            }

            //clears from the if loop
            mpz_clear(j);
            mpz_clear(s_minus_1);
        }

        //clears from the for loop
        mpz_clear(a);
        mpz_clear(n_minus_three);
        mpz_clear(y);
        mpz_clear(n_minus_1);
    }

    //clears from the function
    mpz_clear(r);
    mpz_clear(s);

    return true;
}

//generates a prime number bits long, stores it in p
void make_prime(mpz_t p, uint64_t bits, uint64_t iters) {
    //generates a random number of size bits
    do {
        mpz_urandomb(p, state, bits);
    } while ((!is_prime(p, iters)) || (mpz_sizeinbase(p, 2) < bits));
    return;
}

//computes the GCD of a and b, stores the value in d
void gcd(mpz_t d, mpz_t a, mpz_t b) {
    mpz_t b_temp;
    mpz_init_set(d, a); //d = a in this context
    mpz_init_set(b_temp, b);

    //while b != 0
    while (mpz_cmp_ui(b_temp, 0) != 0) {
        //temp = b_temp
        mpz_t temp;
        mpz_init_set(temp, b_temp);

        //b = a % b
        mpz_mod(b_temp, d, b_temp);

        //a = temp
        mpz_set(d, temp);

        mpz_clear(temp);
    }
    mpz_clear(b_temp);
    return;
}

//computes inverse a % n, stores it in i, i set to 0 in case of no inverse
void mod_inverse(mpz_t i, mpz_t a, mpz_t n) {
    mpz_t r, r_prime, t_prime;
    //i is t in the pseudocode
    //(r, r') <- (n, a)
    mpz_init_set(r, n);
    mpz_init_set(r_prime, a);
    //(t, t') <- (0, 1)
    mpz_set_ui(i, 0);
    mpz_init_set_ui(t_prime, 1);

    //while r' != 0
    while (mpz_cmp_ui(r_prime, 0) != 0) {
        mpz_t q;
        mpz_init(q);
        //q <- [r / r']
        mpz_fdiv_q(q, r, r_prime);

        mpz_t temp; //temp variable for parallel assignments
        //r <- r'
        mpz_init_set(temp, r);
        mpz_set(r, r_prime);
        //r' <- r - q * r'
        mpz_mul(r_prime, r_prime, q);
        mpz_sub(r_prime, temp, r_prime);

        //t <- t'
        mpz_init_set(temp, i);
        mpz_set(i, t_prime);
        //t' <- t - q * t'
        mpz_mul(t_prime, t_prime, q);
        mpz_sub(t_prime, temp, t_prime);

        //clear temp var
        mpz_clear(q);
        mpz_clear(temp);
    }

    //if r > 1
    if (mpz_cmp_ui(r, 1) > 0) {
        mpz_set_ui(i, 0);
        return;
    }

    //if t < 0
    if (mpz_cmp_ui(i, 0) < 0) {
        //t <- t + n
        mpz_add(i, i, n);
    }

    //clear variables
    mpz_clear(r);
    mpz_clear(r_prime);
    mpz_clear(t_prime);
    return;
}
