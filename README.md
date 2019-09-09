# Base64 - base32 and base16 encoders/decoders implementation based on https://tools.ietf.org/html/rfc4648

**Base64.c** is the library file with the ``C implementation`` for all decoders and encoders (b16,b32 included), everything else is headers and useful utilities built to encode to the different
encodings but that use the main library code.


# Testing encoding/decoding

You can opt to build using the included ``Makefile`` or doing the build manually as detailed in the next section:

Base64:

    build: cc b64enc.c base64.c -o b64enc
           cc b64dec.c base64.c -o b64dec


# Encode to base64

    ./b64enc  utf-8.sampler.txt utf-8.sampler.txt.b64

# Decode the encode file back to the original

    ./b64dec utf-8.sampler.txt.b64 utf-8.sampler.txt.decoded64


Now check utf-8.sampler.txt.decoded64 to see if it's an exact copy of the original
file (**utf-8.sampler.txt**)


Use the same  procedure for all other encodings replacing '64' by '32' or '16'


Thanks,
Orestes

