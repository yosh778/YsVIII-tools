# XAST Unpacker + Packer

Unpacker & Packer for Ys VIII assets

## Unpacker

Usage : `unxai <input> <output>`

Extracts all files from an input XAST archive into the given output directory.


## Packer

Usage : `xai <folder> <output> (<original>)`

Packs a whole folder as a XAST output archive.
If the original .xai archive is given as third argument,
it will use the same header structure to avoid game compatibility issues (recommanded).

## Checksums

Credits to [weaknespase](https://github.com/weaknespase) for the checksum algorithm.
