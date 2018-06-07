#ifndef API_H
#define API_H

// MODE = 3 (highest security level):
#define CRYPTO_PUBLICKEYBYTES 1760U
#define CRYPTO_SECRETKEYBYTES 3856U
#define CRYPTO_BYTES 3366U

#define CRYPTO_ALGNAME "Dilithium"

int crypto_sign_keypair(const unsigned char *ph,
                        unsigned char *pk,
                        unsigned char *sk);

int crypto_sign(unsigned char *sm, unsigned long long *smlen,
                const unsigned char *msg, unsigned long long len, 
                const unsigned char *sk);

int crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                     const unsigned char *sm, unsigned long long smlen,
                     const unsigned char *pk);

#endif
