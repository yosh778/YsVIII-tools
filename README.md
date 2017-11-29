# Ys VIII Unpacking / Packing tools

Unpacking & Packing tools for Ys VIII assets


## How to translate Ys VIII (JP / CH) with it ?

Follow the guide [here](https://github.com/yosh778/YsVIII-tools/blob/master/Translation.md).

WARNING : This guide is mostly informative may not contain some complex or more recent steps

## Eboot patcher

Usage : `python3 patchEboot.py <eboot>`

This python script enables you to apply eboot specific patches. For mai dumps, run it on both the base eboot.bin & mai_moe/eboot_origin.bin.
This patch allows you to get the savedata level text to display as "Lv 17" instead of "Lv17" for instance, just like the US/EU game does.


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


### Patcher

Usage : `xaiPatch <inputXai> <inputFile> <filename>`

Replaces a specific file from the input XAST archive with a new one.
You must provide the correct filepath from the XAST archive as filename (example : flash/pl_const.plt)

WARNING : both files must be the same size


## DAT
### Unpacker

Usage : `undat <input> <output>`

Extracts all files from an input DAT archive into the given output directory.


### Packer

Usage : `dat <folder> <output>`

Packs a whole folder as a DAT output archive.

WARNING : filenames must not be more than 15 characters, also no folder structure should be used in theory.

## PLT
### Unpacker

Usage : `unplt <input> <output>`

Extracts all strings from an input PLT archive into the given output text file.

### Packer

Usage : `plt <input> <output>`

Packs all strings from an input text file into the given output PLT archive.

## TBB

Requires Node.js 6+ (8.5 recommended)

### Unpacker

Usage : `node tbbconv/tbbconv.js unpack <inputTBB> <outputCSV>`

Converts a TBB input file into a CSV output file.


### Packer

Usage : `node tbbconv/tbbconv.js pack <inputCSV> <outputTBB> (--enc shift-jis)`

Converts a CSV input file into a TBB output file.

When workingon the Japanese game, add the `--enc shift-jis` argument to repack into shift-jis.


## Scripting

### Disassembler

Usage : `bin2script <byteScript>`

Converts a bytecode script into text.
Credits to [weaknespase](https://github.com/weaknespase) for the [file format reverse](https://gist.github.com/weaknespase/d0a26fbc21a77616199969fe08cd48c2)


### Assembler

Usage : `script2bin <script> <output> (--enc-shift-jis) (--dec-shift-jis) (--preserve-string-sizes)`

Converts a script back into bytecode.

- Add `--enc-shift-jis` to convert game text strings from utf8 back into shift-jis.
- Add `--dec-shift-jis` to convert game text strings from shift-jis into utf8 (experiemental)
- If you specify `--preserve-string-sizes`, `--enc-shift-jis` & `--dec-shift-jis` options will attempt to preserve the original string sizes after conversion (by adding spaces or removing last characters)

### RPN arithmetics

More information about the RPN arithmetic arguments used in conditions & calculations can be found here : https://gist.github.com/weaknespase/d0a26fbc21a77616199969fe08cd48c2#vcalc-operators

You can also find here my first write-up about it : https://gist.github.com/yosh778/141f78102cbcb15673b2375002e52a76


# Credits

- Tony Blue for helping get the game on 3.60
- [weaknespase](https://github.com/weaknespase) for the XAST checksum, TBB tools, script reversing, ... he's our trump card :)
- [vampirexxxx](https://github.com/vampirexxxx) who is a big help in hacking the assets !
