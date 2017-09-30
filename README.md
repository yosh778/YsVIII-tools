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

## Work left undone

### Primary

- Fix game breaking bugs (freezes & crashes)

### Secondary
- Translate cancel menu string on the first skill tutorial
- Improve some .tbb files if needed
- Improve pl_const.plt hacking if possible
- Translate trophies ? (risky & not doable yet it seems)


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

### Unpacking

Usage : `node tbbconv/tbbconv.js unpack <inputTBB> <outputCSV>`

Converts a TBB input file into a CSV output file.


### Packer

Usage : `node tbbconv/tbbconv.js pack <inputCSV> <outputTBB> (--enc shift-jis)`

Converts a CSV input file into a TBB output file.

When workingon the Japanese game, add the `--enc shift-jis` argument to repack into shift-jis.

## Credits

Credits to Tony Blue for helping get the game on 3.60, [weaknespase](https://github.com/weaknespase) for the XAST checksum + TBB tools, & [vampirexxxx](https://github.com/vampirexxxx) who is a big help in hacking the assets !
