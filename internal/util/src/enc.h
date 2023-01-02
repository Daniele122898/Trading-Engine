//
// Created by danie on 11/22/2022.
//

#ifndef TRADINGENGINE_ENC_H
#define TRADINGENGINE_ENC_H

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <string>

static void handleErrors() {
    ERR_print_errors_fp(stderr);
    abort();
}

// hashed array should be 32 bytes for sha256
static unsigned int sha256(unsigned char *text, size_t text_len, unsigned char *hashed) {
    EVP_MD_CTX *context;
    /* Create and initialise the context */
    if (!(context = EVP_MD_CTX_new()))
        handleErrors();

    if (!EVP_DigestInit_ex(context, EVP_sha256(), NULL))
        handleErrors();

    if (!EVP_DigestUpdate(context, text, text_len))
        handleErrors();

    unsigned int len;
    if (!EVP_DigestFinal_ex(context, hashed, &len))
        handleErrors();

    EVP_MD_CTX_free(context);

    return len;
}

inline void get_rand(unsigned char *salt, int num) {
    RAND_bytes(salt, 32);
}

static unsigned int sha256_salted(std::string &input, unsigned char *hashed, unsigned char *salt) {
    get_rand(salt, 32);
    unsigned char *inp = reinterpret_cast<unsigned char *>(input.data());
    unsigned char combined[input.size() + 32];
    std::copy(inp, inp + input.size(), combined);
    std::copy(salt, salt + 32, combined + input.size());

    return sha256(combined, input.size() + 32, hashed);
}

static bool sha256_match(std::basic_string<std::byte> &pwhash, std::basic_string<std::byte> &salt, std::string &password) {
    unsigned char *pwdb = reinterpret_cast<unsigned char *>(pwhash.data());
    unsigned char *pw = reinterpret_cast<unsigned char *>(password.data());
    unsigned char *s = reinterpret_cast<unsigned char *>(salt.data());
    unsigned char combined[password.size() + 32];
    std::copy(pw, pw + password.size(), combined);
    std::copy(s, s + 32, combined + password.size());

    unsigned char hash[32];
    sha256(combined, password.size() + 32, hash);
    for (int i = 0; i < 32; ++i) {
        if (pwdb[i] != hash[i])
            return false;
    }
    return true;
}

#endif //TRADINGENGINE_ENC_H
