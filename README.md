# rsa-suite
This program will implement an RSA suite, capable of producing RSA public and private keys, a file encryptor, a file decryptor, and file signatures.

## Building

Ensure that pkg-config is installed on the machine.

Build this program with:
```
make
```
or
```
make all
```
to make a specific binary run one of the following:
```
make keygen
```
```
make encrypt
```
```
make decrypt
```

## Execution

Execute the programs with:

```
$ ./keygen [-hv] [-b bits] -n pbfile -d pvfile
```
```
$ ./encrypt [-hv] [-i infile] [-o outfile] -n pubkey
```
```
$ ./decrypt [-hv] [-i infile] [-o outfile] -n privkey
```

## Usage
### keygen
   -h              Display program help and usage.   
   -v              Display verbose program output.   
   -b bits         Minimum bits needed for public key n (default: 256).   
   -i confidence   Miller-Rabin iterations for testing primes (default: 50).   
   -n pbfile       Public key file (default: rsa.pub).   
   -d pvfile       Private key file (default: rsa.priv).   
   -s seed         Random seed for testing.   

### encrypt
   -h              Display program help and usage.   
   -v              Display verbose program output.   
   -i infile       Input file of data to encrypt (default: stdin).   
   -o outfile      Output file for encrypted data (default: stdout).   
   -n pbfile       Public key file (default: rsa.pub).   

### decrypt
   -h              Display program help and usage.   
   -v              Display verbose program output.   
   -i infile       Input file of data to decrypt (default: stdin).   
   -o outfile      Output file for decrypted data (default: stdout).   
   -n pvfile       Private key file (default: rsa.priv).  

*credit: Professor Long, keygen/encrypt/decrypt resource binaries*
