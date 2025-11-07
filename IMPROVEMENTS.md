# Base64 Implementation - Improvement Plan

## 1. Performance Optimizations

### 1.1 Add Lookup Tables (Critical)
**Current Issue**: Linear search O(64) per character in `b64_dec` line 316
**Impact**: 10-20x speedup for decoding

**Solution**: Create reverse lookup tables

```c
// Add after alphabet definitions
static unsigned char b64_lookup[256] = {0};
static unsigned char b64_url_lookup[256] = {0};
static unsigned char b32_lookup[256] = {0};
static unsigned char b16_lookup[256] = {0};

// Initialize lookup tables (call once at startup or use static initialization)
void init_lookup_tables(void) {
    for (int i = 0; i < 64; i++) {
        b64_lookup[(unsigned char)b64_alp[i]] = i;
        b64_url_lookup[(unsigned char)b64_url_alp[i]] = i;
    }
    for (int i = 0; i < 32; i++) {
        b32_lookup[(unsigned char)b32_alp[i]] = i;
    }
    for (int i = 0; i < 16; i++) {
        b16_lookup[(unsigned char)b16_alp[i]] = i;
    }
}

// Replace line 316 in b64_dec:
// OLD: for (t = 0; s[i] != b64_alp[t]; t++);
// NEW: t = b64_lookup[(unsigned char)s[i]];
```

### 1.2 Remove Deprecated `register` Keyword
**Location**: Line 247 in `b64_enc`
**Change**: Modern compilers optimize better than `register` hint

## 2. Error Handling Improvements

### 2.1 Return Error Codes Instead of exit()
**Current Issue**: Functions call `exit(EXIT_FAILURE)` making them unusable as library

**Solution**: Define error codes and return them

```c
// Add to base64.h
typedef enum {
    BASE64_OK = 0,
    BASE64_INVALID_CHAR = -1,
    BASE64_BUFFER_TOO_SMALL = -2,
    BASE64_INVALID_PADDING = -3,
    BASE64_NULL_POINTER = -4
} base64_error_t;

// Modify function signatures:
// OLD: unsigned int b64_dec(unsigned char *s, char b[], unsigned int len)
// NEW: base64_error_t b64_dec(unsigned char *s, char b[], unsigned int len, 
//                              unsigned int *out_len);

// Example implementation:
base64_error_t b64_dec(unsigned char *s, char b[], unsigned int len, 
                       unsigned int *out_len) {
    if (!s || !b || !out_len) return BASE64_NULL_POINTER;
    // ... existing code ...
    if (invalid_char) return BASE64_INVALID_CHAR;
    *out_len = rtsize;
    return BASE64_OK;
}
```

### 2.2 Add Input Validation
```c
base64_error_t b64_dec(unsigned char *s, char b[], unsigned int len, 
                       unsigned int *out_len) {
    if (!s || !b || !out_len) return BASE64_NULL_POINTER;
    if (len == 0) return BASE64_INVALID_PADDING;
    
    // Calculate required output buffer size
    unsigned int required = get_data_size((char*)s, len);
    // Check if buffer is large enough (if size parameter provided)
    // ...
}
```

## 3. Buffer Safety

### 3.1 Add Buffer Size Parameters
```c
// Add buffer size parameter to prevent overflow
base64_error_t b64_dec(unsigned char *s, char b[], unsigned int in_len,
                       unsigned int out_buf_size, unsigned int *out_len);
```

### 3.2 Bounds Checking
```c
// In b64_dec, before writing:
if (w >= out_buf_size) {
    return BASE64_BUFFER_TOO_SMALL;
}
b[w++] = x >> 24;
```

## 4. Code Quality Fixes

### 4.1 Remove Unused Variable
**Location**: Line 275 in `b64_dec`
```c
// OLD: unsigned char pad,t,l = 0;
// NEW: unsigned char pad,t;
```

### 4.2 Fix Duplicate Declaration
**Location**: base64.h line 13
Remove duplicate `void base64_enc(char *s, char b[]);`

### 4.3 Add const Correctness
```c
// Make input parameters const where appropriate
unsigned int b64_dec(const unsigned char *s, char b[], unsigned int len);
```

### 4.4 Fix Typo in Comment
**Location**: Line 4 - "standarized" → "standardized"

## 5. Unit Testing Framework

### 5.1 Create Test Framework
Create `test_base64.c` with a simple test framework:

```c
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "base64.h"

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("FAIL: %s\n", msg); \
            return 1; \
        } \
    } while(0)

int test_b64_enc_dec() {
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
    // Test 1 byte padding (2 '=' chars)
    const char *input = "A";
    char encoded[256];
    b64_enc((unsigned char*)input, encoded, 1);
    TEST_ASSERT(encoded[strlen(encoded)-1] == '=', "Missing padding");
    TEST_ASSERT(encoded[strlen(encoded)-2] == '=', "Missing second padding");
    
    // Test 2 byte padding (1 '=' char)
    const char *input2 = "AB";
    b64_enc((unsigned char*)input2, encoded, 2);
    TEST_ASSERT(encoded[strlen(encoded)-1] == '=', "Missing padding");
    
    printf("PASS: Padding tests\n");
    return 0;
}

int test_b64_invalid_char() {
    // Test with invalid character
    const char *invalid = "SGVsbG8=!"; // Invalid '!' at end
    char decoded[256];
    // Should handle gracefully (currently exits - needs error code fix)
    printf("PASS: Invalid char test (needs error code implementation)\n");
    return 0;
}

int test_b64_binary_data() {
    unsigned char binary[] = {0x00, 0xFF, 0x42, 0x13, 0x37};
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
    unsigned int dec_len = b64_dec((unsigned char*)encoded, decoded, strlen(encoded));
    
    TEST_ASSERT(dec_len == 0, "Empty string decode failed");
    printf("PASS: Empty string test\n");
    return 0;
}

int test_b64_rfc4648_vectors() {
    // RFC 4648 test vectors
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

int main(void) {
    int failures = 0;
    
    printf("Running Base64 tests...\n\n");
    
    failures += test_b64_enc_dec();
    failures += test_b64_padding();
    failures += test_b64_binary_data();
    failures += test_b64_empty_string();
    failures += test_b64_rfc4648_vectors();
    // failures += test_b64_invalid_char(); // Enable after error code fix
    
    printf("\nTests completed: %d failures\n", failures);
    return failures;
}
```

### 5.2 Update Makefile for Tests
```makefile
test: test_base64
	./test_base64

test_base64: base64.o test_base64.c
	$(CC) test_base64.c base64.o -o test_base64
```

## 6. Additional Improvements

### 6.1 Add Whitespace Handling (RFC 4648 Section 3.3)
```c
// Skip whitespace during decoding (optional but recommended)
if (isspace(s[i])) {
    --z;
    continue;
}
```

### 6.2 Add Memory Allocation Helpers
```c
// Helper to calculate required buffer size
size_t b64_enc_size(size_t input_len) {
    return ((input_len + 2) / 3) * 4 + 1; // +1 for null terminator
}

size_t b64_dec_size(size_t input_len) {
    return (input_len / 4) * 3 + 1;
}
```

### 6.3 Add Streaming API (for large files)
```c
typedef struct {
    unsigned int state;
    unsigned char buffer[3];
    int buffer_len;
} b64_encoder_t;

void b64_enc_init(b64_encoder_t *ctx);
int b64_enc_update(b64_encoder_t *ctx, const unsigned char *in, 
                   size_t in_len, char *out, size_t *out_len);
int b64_enc_final(b64_encoder_t *ctx, char *out, size_t *out_len);
```

### 6.4 Add Documentation Comments (Doxygen style)
```c
/**
 * @brief Encode binary data to base64
 * @param s Input data to encode
 * @param b Output buffer (must be large enough)
 * @param len Length of input data in bytes
 * @return Number of encoded characters (excluding null terminator)
 * @note Output buffer should be at least ((len + 2) / 3) * 4 + 1 bytes
 */
void b64_enc(unsigned const char *s, char b[], unsigned int len);
```

## 7. Performance Benchmarking

Create `benchmark.c` to measure performance improvements:

```c
#include <time.h>
#include <stdio.h>
#include "base64.h"

#define ITERATIONS 1000000
#define DATA_SIZE 1024

int main(void) {
    unsigned char data[DATA_SIZE];
    char encoded[DATA_SIZE * 2];
    char decoded[DATA_SIZE];
    
    // Initialize test data
    for (int i = 0; i < DATA_SIZE; i++) {
        data[i] = i % 256;
    }
    
    clock_t start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        b64_enc(data, encoded, DATA_SIZE);
    }
    clock_t end = clock();
    
    double encode_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Encode: %f seconds for %d iterations\n", encode_time, ITERATIONS);
    printf("Throughput: %.2f MB/s\n", 
           (DATA_SIZE * ITERATIONS) / (encode_time * 1024 * 1024));
    
    return 0;
}
```

## Priority Order

1. **High Priority**: Lookup tables (biggest performance gain)
2. **High Priority**: Unit testing framework (ensures correctness)
3. **Medium Priority**: Error code returns (library usability)
4. **Medium Priority**: Buffer safety (security)
5. **Low Priority**: Code quality fixes (cleanup)
6. **Low Priority**: Documentation improvements

## Expected Improvements

- **Performance**: 10-20x faster decoding with lookup tables
- **Reliability**: Unit tests catch regressions
- **Usability**: Error codes make it library-friendly
- **Safety**: Buffer checks prevent crashes
- **Maintainability**: Better code quality and documentation

