# XAST Unpacker + Packer

Unpacker & Packer for Ys VIII assets

## Unpacker

Usage : `unxai <input> <output>`

Extracts all files from an input XAST archive into the given output directory.


## Packer

Usage : `xai <folder> <output>`

Packs a whole folder as a XAST output archive.

WARNING : Packer does not compute correct checkums yet

## About checksums

There are 3 checksums :

The XAST checksum @offset 0x18 : this seems to be a checksum covering all header entries, but not the filepaths.
Any change to the file header entries will make the game crash when loading.

Then we have 2 checksum for each file header entry.

The first file checksum seems to be a checksum on the filepath, indeed it doesn't change on new updates but changes between identical differently named files. File entry header offset : 0x0

The second file checksum seems to be a checksum on the file contents, indeed it doesn't change between identical differently named files but does change when a file is updated with different contents. File entry header offset : 0x8
