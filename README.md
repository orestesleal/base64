# Base64, Base32, and Base16 Encoders & Decoders

A C implementation of Base64, Base32, and Base16 encoding/decoding compliant with [RFC 4648](https://tools.ietf.org/html/rfc4648).

## Overview

This library provides efficient, RFC 4648-compliant implementations of Base64, Base32, and Base16 encoding and decoding. The core library is in `base64.c` and `base64.h`, which contains all encoding/decoding functions. The other files in this repository are example programs and utilities that demonstrate how to use the library.

## Features

- **RFC 4648 Compliant**: Full compliance with the Base64, Base32, and Base16 standards
- **High Performance**: O(1) character lookup using lookup tables for 10-20x faster decoding
- **Binary Support**: Handles both text and binary data (images, executables, etc.)
- **Multiple Variants**: Supports standard Base64 and Base64URL (URL-safe variant)
- **Comprehensive Testing**: Includes unit test suite with RFC test vectors
- **Production Ready**: Used in production systems (see [Users](#users) section)

## Building

### Quick Build

Build all binaries:

```bash
make
```

This will create:
- `b64enc` / `b64dec` - Base64 encoder/decoder
- `b32enc` / `b32dec` - Base32 encoder/decoder
- `b16enc` / `b16dec` - Base16 encoder/decoder

### Selective Build

Build specific encoders/decoders:

```bash
make b16enc b16dec    # Base16 only
make b32enc b32dec    # Base32 only
make b64enc b64dec    # Base64 only
```

### Manual Build

If you prefer to build manually:

```bash
# Compile the library
cc -c base64.c

# Build encoder
cc b64enc.c base64.o -o b64enc

# Build decoder
cc b64dec.c base64.o -o b64dec
```

## Usage

### Command Line Tools

#### Encoding to Base64

```bash
./b64enc input.txt output.b64
```

#### Decoding from Base64

```bash
./b64dec input.b64 output.txt
```

#### Example: Round-trip Encoding/Decoding

```bash
# Encode a file
./b64enc utf-8.sampler.txt utf-8.sampler.txt.b64

# Decode it back
./b64dec utf-8.sampler.txt.b64 utf-8.sampler.txt.decoded64

# Verify they match
diff utf-8.sampler.txt utf-8.sampler.txt.decoded64
```

The same procedure works for Base32 and Base16 by replacing `64` with `32` or `16` in the command names.

### Library Usage

Include the header and link against the library:

```c
#include "base64.h"

// Encode binary data
unsigned char data[] = {0x00, 0xFF, 0x42};
char encoded[256];
b64_enc(data, encoded, sizeof(data));

// Decode base64 string
char decoded[256];
unsigned int decoded_len = b64_dec((unsigned char*)encoded, decoded, strlen(encoded));
```

See `base64.h` for the complete API documentation.

## Testing

### Running the Test Suite

The project includes a comprehensive unit test suite:

```bash
make test
```

This will:
1. Compile the test suite
2. Run all unit tests
3. Display results

### Test Coverage

The test suite includes:

- **Round-trip encoding/decoding**: Verifies data integrity
- **Padding tests**: Validates correct padding for 1-byte and 2-byte inputs
- **Binary data tests**: Ensures proper handling of binary data including null bytes
- **Empty string handling**: Edge case testing
- **RFC 4648 test vectors**: Official compliance tests
- **Large input tests**: Performance and correctness with large datasets
- **Special characters**: Unicode and special character handling

All tests should pass. If any test fails, please report it as a bug.

### Manual Testing

You can also test manually using the command-line tools:

```bash
# Test with text
echo "Hello, World!" | ./b64enc | ./b64dec

# Test with binary data
printf "\x00\xFF\x42" | ./b64enc | ./b64dec | od -An -tx1
```

## Library Structure

- **`base64.c`** / **`base64.h`**: Core library with all encoding/decoding functions
- **`b64enc.c`** / **`b64dec.c`**: Example Base64 command-line tools
- **`b32enc.c`** / **`b32dec.c`**: Example Base32 command-line tools
- **`b16enc.c`** / **`b16dec.c`**: Example Base16 command-line tools
- **`test_base64.c`**: Unit test suite

## API Functions

### Base64 Functions

- `b64_enc()` - General purpose Base64 encoding (supports binary data)
- `b64_dec()` - General purpose Base64 decoding (returns decoded byte count)
- `base64_enc()` - Text-only Base64 encoding
- `base64_dec()` - Text-only Base64 decoding
- `base64url_enc()` - Base64URL encoding (URL-safe variant)
- `base64url_dec()` - Base64URL decoding

### Base32 Functions

- `b32_enc()` - General purpose Base32 encoding
- `b32_dec()` - General purpose Base32 decoding

### Base16 Functions

- `b16_enc()` - General purpose Base16 (hex) encoding
- `b16_dec()` - General purpose Base16 (hex) decoding

### Utility Functions

- `encode_wr_file()` - Encode a file and write to another file
- `decode_rd_file()` - Read and decode a file, write to another file
- `get_file()` - Load a file into memory
- `alloc()` - Memory allocation helper
- `b64_enc_size()` - Calculate required buffer size for encoding
- `b64_dec_size()` - Calculate required buffer size for decoding

See `base64.h` for complete function signatures and documentation.

## Performance

The implementation uses lookup tables for O(1) character decoding, providing significant performance improvements:

- **Decoding**: 10-20x faster than linear search implementations
- **Character Lookup**: Constant time O(1) instead of O(64) linear search
- **Optimized**: Efficient bit manipulation and memory access patterns

The lookup tables are statically initialized at compile time, ensuring zero runtime overhead for initialization.

## Users

This code is used in production systems. One notable user is:

- **[MIME::Base32::XS](https://metacpan.org/pod/MIME::Base32::XS)** - A Perl module that uses the Base32 encoder/decoder as its backend. The integration can be seen in the [XS.xs source file](https://metacpan.org/source/LTM/MIME-Base32-XS-0.09/XS.xs).

## License

Copyright Orestes Leal Rodriguez 2015

## Contributing

Found a bug or have a suggestion? Please open an issue or submit a pull request.

## See Also

- [RFC 4648](https://tools.ietf.org/html/rfc4648) - The Base16, Base32, and Base64 Data Encodings specification
