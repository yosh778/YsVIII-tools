# Ys VIII Unpacking / Packing tools

Unpacking & Packing tools for Ys VIII assets


## How to translate Ys VIII (JP / CH) with it ?

- First you need the Japanese (PCSG00881) or Chinese (PCSH00297) game with the 1.02 update & the dumped english assets including the 1.01 update (tested on the EUR version, PCSB01128). Seems the chinese game as base works better though !
- Replace rootast.xai & patch101.xai with english ones first
- Now, we need to fix the japanese patch102.xai with english.
To do that, use the XAST Unpacker to extract english rootast.xai & patch101.xai & japanese patch102.xai
- Now replace all files in patch102.xai with their english version in patch101.xai, or rootast.xai if not in 1.01, except flash/pkg_menu.xai & system/1stload.dat which must not change
- Then, we must patch the japanese system/1stload.dat
- First use the DAT Unpacker to extract both japanese / english system/1stload.dat files
- Now use the `swap.sh` bash script on `1stload.list` and english then japanese directories created in last step.
Something like `./swap.sh 1stload.list 1stload_eng/ 1stload_jap/`, otherwise you can just replace each non-commented file in `1stload.list` with its english version (`swap.sh` does that)
- Some files from 1stload.list will be missing from the extracted english 1stload.dat, so just check which ones are missing according to what failed on last step & get those files from the extracted patch101.xai or else rootast.xai english files.
Then, reproduce the last step again, it should work fine now.
- Now we'll patch the japanese flash/pkg_menu.xai
- First use the XAST Unpacker to extract both japanese / english flash/pkg_menu.xai files
- Now use the `swap.sh` bash script on `pkg_menu.list` and english then japanese directories created in last step.
Something like `./swap.sh pkg_menu.list pkg_menu_eng/ pkg_menu_jap/`, otherwise you can just replace each non-commented file in `pkg_menu.list` with its english version (`swap.sh` does that)
- For pl_const.plt & lit_rtxt.csv from from 1stload.dat, you need to hack more manually
- For pl_const.plt, just use the english one for the chinese game. For the Japanese game, you have to convert the encoding to SHIFT-JIS by first unpacking it, & then once fixed repack it using the PLT tools
- For lit_rtxt.csv, with the Japanese game you have to convert the encoding to SHIFT-JIS again.
Then, you must change the quote system used here. Indeed, english uses quotes like "this", you have to convert them to something like 「this」 (take the quote characters directly from the original game's file to make sure the encoding is right !)
- Now for the japanese version only, unpack & then repack uncommented files from tbb2shift.list in 1stload.dat using TBB tools with `--enc shift-jis`, & place these in your patched 1stload folder (this should fix encoding issues)
- Finally, repack the fixed 1stload.dat & pkg_menu.xai files using the `dat` & `xai` tools, use it as system/1stload.dat & flash/pkg_menu.xai in 102 & then repack 102 using the XAST Packer.
Something like `./xai patch102_fix patch102_fixed.xai patch102_untouched.xai`
- Finished, just use the generated patch102_fixed.xai file as patch102.xai in your japanese game folder & have fun.
- Additionally, you can use the eboot patcher below to fix some details from the eboot itself

## Work left undone

### Secondary

- Fix encoding issues
- Hack into pl_const.plt to improve icon positions if possible
- Translate trophies ? (risky & not doable yet it seems)

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
