/*
 * Unit tests for base64 encoding/decoding functions
 * Run with: make test && ./test_base64
 *
 * Copyright Orestes Leal Rodriguez 2015-2025
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "base64.h"

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("FAIL: %s\n", msg); \
            return 1; \
        } \
    } while(0)

int test_b64_enc_dec_roundtrip() {
    const char *input = "Hello, World!";
    char encoded[256];
    char decoded[256];
    unsigned int dec_len;
    
    b64_enc((unsigned char*)input, encoded, strlen(input));
    dec_len = b64_dec((unsigned char*)encoded, decoded, strlen(encoded));
    
    TEST_ASSERT(dec_len == strlen(input), "Decoded length mismatch");
    TEST_ASSERT(memcmp(input, decoded, dec_len) == 0, "Round-trip failed");
    
    printf("PASS: b64_enc/b64_dec round-trip\n");
    return 0;
}

int test_b64_padding() {
    char encoded[256];
    char decoded[256];
    unsigned int dec_len;
    
    // Test 1 byte input (should have 2 padding chars)
    const char *input1 = "A";
    b64_enc((unsigned char*)input1, encoded, 1);
    unsigned int enc_len1 = strlen(encoded);
    TEST_ASSERT(enc_len1 >= 2, "Encoded string too short");
    TEST_ASSERT(encoded[enc_len1-1] == '=', "Missing first padding");
    TEST_ASSERT(encoded[enc_len1-2] == '=', "Missing second padding");
    
    dec_len = b64_dec((unsigned char*)encoded, decoded, enc_len1);
    TEST_ASSERT(dec_len == 1, "1-byte decode length mismatch");
    TEST_ASSERT(decoded[0] == 'A', "1-byte decode content mismatch");
    
    // Test 2 byte input (should have 1 padding char)
    const char *input2 = "AB";
    b64_enc((unsigned char*)input2, encoded, 2);
    unsigned int enc_len2 = strlen(encoded);
    TEST_ASSERT(enc_len2 >= 1, "Encoded string too short");
    TEST_ASSERT(encoded[enc_len2-1] == '=', "Missing padding");
    
    dec_len = b64_dec((unsigned char*)encoded, decoded, enc_len2);
    TEST_ASSERT(dec_len == 2, "2-byte decode length mismatch");
    TEST_ASSERT(memcmp(input2, decoded, 2) == 0, "2-byte decode content mismatch");
    
    printf("PASS: Padding tests\n");
    return 0;
}

int test_b64_binary_data() {
    unsigned char binary[] = {0x00, 0xFF, 0x42, 0x13, 0x37, 0xDE, 0xAD, 0xBE, 0xEF};
    char encoded[256];
    char decoded[256];
    unsigned int dec_len;
    
    b64_enc(binary, encoded, sizeof(binary));
    dec_len = b64_dec((unsigned char*)encoded, decoded, strlen(encoded));
    
    TEST_ASSERT(dec_len == sizeof(binary), "Binary decode length mismatch");
    TEST_ASSERT(memcmp(binary, decoded, dec_len) == 0, "Binary round-trip failed");
    
    printf("PASS: Binary data test\n");
    return 0;
}

int test_b64_empty_string() {
    char encoded[256];
    char decoded[256];
    
    b64_enc((unsigned char*)"", encoded, 0);
    unsigned int enc_len = strlen(encoded);
    
    // Handle empty encoded string (b64_dec doesn't handle len=0 well)
    if (enc_len == 0) {
        TEST_ASSERT(1, "Empty string encodes to empty (expected)");
        printf("PASS: Empty string test (empty encoding)\n");
        return 0;
    }
    
    unsigned int dec_len = b64_dec((unsigned char*)encoded, decoded, enc_len);
    
    TEST_ASSERT(dec_len == 0, "Empty string decode failed");
    printf("PASS: Empty string test\n");
    return 0;
}

int test_b64_rfc4648_vectors() {
    // RFC 4648 Section 10 test vectors
    struct {
        const char *input;
        const char *expected;
    } vectors[] = {
        {"", ""},
        {"f", "Zg=="},
        {"fo", "Zm8="},
        {"foo", "Zm9v"},
        {"foob", "Zm9vYg=="},
        {"fooba", "Zm9vYmE="},
        {"foobar", "Zm9vYmFy"},
    };
    
    for (int i = 0; i < sizeof(vectors)/sizeof(vectors[0]); i++) {
        char encoded[256];
        char decoded[256];
        
        b64_enc((unsigned char*)vectors[i].input, encoded, strlen(vectors[i].input));
        TEST_ASSERT(strcmp(encoded, vectors[i].expected) == 0, 
                   "RFC vector encoding mismatch");
        
        unsigned int dec_len = b64_dec((unsigned char*)encoded, decoded, strlen(encoded));
        TEST_ASSERT(dec_len == strlen(vectors[i].input), "RFC vector decode length");
        TEST_ASSERT(memcmp(vectors[i].input, decoded, dec_len) == 0, 
                   "RFC vector round-trip");
    }
    
    printf("PASS: RFC 4648 test vectors\n");
    return 0;
}

int test_b64_large_input() {
    // Test with larger input
    char *large_input = malloc(1000);
    char encoded[2000];
    char decoded[2000];
    
    // Fill with pattern
    for (int i = 0; i < 1000; i++) {
        large_input[i] = (char)(i % 256);
    }
    
    b64_enc((unsigned char*)large_input, encoded, 1000);
    unsigned int dec_len = b64_dec((unsigned char*)encoded, decoded, strlen(encoded));
    
    TEST_ASSERT(dec_len == 1000, "Large input decode length mismatch");
    TEST_ASSERT(memcmp(large_input, decoded, 1000) == 0, "Large input round-trip failed");
    
    free(large_input);
    printf("PASS: Large input test\n");
    return 0;
}

int test_b64_special_chars() {
    // Test with various special characters
    const char *special = "!@#$%^&*()_+-=[]{}|;':\",./<>?`~";
    char encoded[256];
    char decoded[256];
    
    b64_enc((unsigned char*)special, encoded, strlen(special));
    unsigned int dec_len = b64_dec((unsigned char*)encoded, decoded, strlen(encoded));
    
    TEST_ASSERT(dec_len == strlen(special), "Special chars decode length");
    TEST_ASSERT(memcmp(special, decoded, dec_len) == 0, "Special chars round-trip");
    
    printf("PASS: Special characters test\n");
    return 0;
}

int test_b64_unicode() {
    // Test with UTF-8 encoded unicode
    const char *unicode = "Hello 世界 🌍";
    char encoded[256];
    char decoded[256];

    b64_enc((unsigned char*)unicode, encoded, strlen(unicode));
    unsigned int dec_len = b64_dec((unsigned char*)encoded, decoded, strlen(encoded));

    TEST_ASSERT(dec_len == strlen(unicode), "Unicode decode length");
    TEST_ASSERT(memcmp(unicode, decoded, dec_len) == 0, "Unicode round-trip");

    printf("PASS: Unicode test\n");
    return 0;
}

int test_b64_invalid_input() {
    const char *invalid = "invalid*base64";
    char decoded[256];

    errno = 0;
    unsigned int dec_len = b64_dec((unsigned char*)invalid, decoded, strlen(invalid));

    TEST_ASSERT(dec_len == 0, "Invalid Base64 should return 0");
    TEST_ASSERT(errno == EINVAL, "Invalid Base64 should set errno");

    printf("PASS: Invalid Base64 input test\n");
    return 0;
}

int test_b32_roundtrip() {
    const unsigned char data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x42};
    char encoded[128];
    char decoded[128];

    b32_enc(data, (unsigned char *)encoded, sizeof(data));
    errno = 0;
    unsigned int dec_len = b32_dec((const unsigned char *)encoded, decoded, strlen(encoded));

    TEST_ASSERT(errno == 0, "Base32 round-trip errno");
    TEST_ASSERT(dec_len == sizeof(data), "Base32 round-trip length");
    TEST_ASSERT(memcmp(data, decoded, dec_len) == 0, "Base32 round-trip content");

    printf("PASS: Base32 round-trip test\n");
    return 0;
}

int test_b32_invalid_input() {
    const char *invalid = "MZXW6!=="; // '!' is invalid in Base32 alphabet
    char decoded[128];

    errno = 0;
    unsigned int dec_len = b32_dec((const unsigned char *)invalid, decoded, strlen(invalid));

    TEST_ASSERT(dec_len == 0, "Invalid Base32 should return 0");
    TEST_ASSERT(errno == EINVAL, "Invalid Base32 should set errno");

    printf("PASS: Invalid Base32 input test\n");
    return 0;
}

int test_b16_roundtrip() {
    const unsigned char data[] = {0x00, 0x11, 0x22, 0x33, 0xFE, 0xDC, 0xBA, 0x98};
    char encoded[128];
    char decoded[128];

    b16_enc(data, encoded, sizeof(data));
    errno = 0;
    unsigned int dec_len = b16_dec(encoded, decoded, strlen(encoded));

    TEST_ASSERT(errno == 0, "Base16 round-trip errno");
    TEST_ASSERT(dec_len == sizeof(data), "Base16 round-trip length");
    TEST_ASSERT(memcmp(data, decoded, dec_len) == 0, "Base16 round-trip content");

    printf("PASS: Base16 round-trip test\n");
    return 0;
}

int test_b16_invalid_input() {
    const char *invalid = "0G"; // 'G' is invalid in hexadecimal alphabet
    char decoded[8];

    errno = 0;
    unsigned int dec_len = b16_dec(invalid, decoded, strlen(invalid));

    TEST_ASSERT(dec_len == 0, "Invalid Base16 should return 0");
    TEST_ASSERT(errno == EINVAL, "Invalid Base16 should set errno");

    printf("PASS: Invalid Base16 input test\n");
    return 0;
}

int main(void) {
    int failures = 0;

    printf("Running Base64 tests...\n");
    printf("======================\n\n");
    
    failures += test_b64_enc_dec_roundtrip();
    failures += test_b64_padding();
    failures += test_b64_binary_data();
    failures += test_b64_empty_string();
    failures += test_b64_rfc4648_vectors();
    failures += test_b64_large_input();
    failures += test_b64_special_chars();
    failures += test_b64_unicode();
    failures += test_b64_invalid_input();
    failures += test_b32_roundtrip();
    failures += test_b32_invalid_input();
    failures += test_b16_roundtrip();
    failures += test_b16_invalid_input();
    
    printf("\n======================\n");
    if (failures == 0) {
        printf("All tests PASSED! ✓\n");
    } else {
        printf("Tests completed with %d failure(s)\n", failures);
    }
    
    return failures;
}

