# NEAC: NEko Audio Codec

## WHAT IS THIS?
NEAC: NEko (cat (猫) in japanese) Audio Codec is a open-source lossless audio codec.

Why "cat"? Because I like cats.

16-bit PCM compression provides approximately the same compression ratio as 
FLAC's maximum efficiency setting.

24-bit PCM compression provides compression rates similar to Monkey's Audio's
maximum efficiency setting, and in some cases even higher.

Reference implementation of NEAC is a released under WTFPL Version 2. 
NEAC algorithms are released in the public domain. 

## HOW TO BUILD
run ``build.py``.

How to run ``build.py`` on your computer:
```
python build.py
```

If the compilation completes successfully, object files and executable files will be generated in the following directory.

```
        ../bin        Executable files
        ../obj        Object files
```

gcc is required to run the build script. You can also compile with clang by editing build.py.
Additionally, the build script is intended to be run on Python 3.13.0 or later.

## DISCLAIMER
NEAC created a codec for the purpose of learning lossless music compression technology,
and the compression efficiency of the codec was surprisingly good, so we decided to make
the result public and are distributing it. Since this is a program created for learning
purposes only, tests may not be sufficient.