# ``base64`` - ``base32`` - ``base16:`` Encoders & decoders.

This ``C`` implementation tries to be compliant with ``rfc4648 <https://tools.ietf.org/html/rfc4648>``

**base64.c** is the library file with the ``C implementation`` for all decoders and encoders (b16,b32 included), plus some useful functions, everything else is code that use the library. 

One user of my ``base32`` code is here https://metacpan.org/pod/MIME::Base32::XS (specifically at https://metacpan.org/source/LTM/MIME-Base32-XS-0.09/XS.xs) where the base32 encoders/decoders are used as the backend for a ``Perl`` module named ``MIME::Base32::XS``



You can opt to build using the included ``Makefile``, to build all the binaries just do ``make``. To build specific binaries do the following:

 - ``make b16enc b16dec`` to build the base16 encoder/decoder
 - ``make b32enc b32dec`` to build the base32 encoder/decoder
 - ``make b64enc b64dec`` to build the base64 encoder/decoder


``To build manually do the following``:

    build: cc b64enc.c base64.c -o b64enc
           cc b64dec.c base64.c -o b64dec

``Testing encoding/decoding:``

``Encoding to base64``

    ./b64enc  utf-8.sampler.txt utf-8.sampler.txt.b64

``Decode the encode file back to the original``

    ./b64dec utf-8.sampler.txt.b64 utf-8.sampler.txt.decoded64

Now check utf-8.sampler.txt.decoded64 to see if it's an exact copy of the original
file (``utf-8.sampler.txt``).

If using ``make`` to build specific binaries use the same procedure for all other encoders/decoders replacing ``64`` with ``32`` or ``16``.

