# Base64, base32, base16 encoders/decoders based on rfc4648

Base64.c is the main file with the C implementation, everything
else is headers or useful utilities built to encode to the different
encodings but that use the main implementation code.


Testing encoding/decoding

Base64:

    build: cc b64enc base64.c -o b64enc
           cc b64dec base64.c -o b64dec


Encode to base64

    ./b64enc  utf-8.sampler.txt utf-8.sampler.txt.b64

Decode the encode file back to the original

    ./b64dec utf-8.sampler.txt.b64 utf-8.sampler.txt.decoded64


Now check utf-8.sampler.txt.decoded64 to see if it's an exact copy of the original
file (utf-8.sampler.txt)


Use the same  procedure for all other encodings replacing '64' by '32' or '16'


Thanks,
Orestes





    
