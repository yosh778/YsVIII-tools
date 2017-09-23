# XAST & DAT Unpacking / Packing tools

Unpacking & Packing tools for Ys VIII assets

## XAST
### Unpacker

Usage : `unxai <input> <output>`

Extracts all files from an input XAST archive into the given output directory.


### Packer

Usage : `xai <folder> <output> (<original>)`

Packs a whole folder as a XAST output archive.

If the original .xai archive is given as third argument,
it will use the same header structure to avoid game compatibility issues (recommanded).

WARNING : When specifying the original .xai, DO NOT add new files (not supported yet).

### Checksums

Credits to [weaknespase](https://github.com/weaknespase) for the checksum algorithm.


## DAT
### Unpacker

Usage : `undat <input> <output>`

Extracts all files from an input DAT archive into the given output directory.


### Packer

Usage : `dat <folder> <output>`

Packs a whole folder as a DAT output archive.

WARNING : filenames must not be more than 15 characters, also no folder structure should be used in theory.
