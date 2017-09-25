# Ys VIII Unpacking / Packing tools

Unpacking & Packing tools for Ys VIII assets


## How to translate Ys VIII (JP) with it ?

- First you need the japanese game with the 1.02 update (PCSG00881) & the dumped english assets including the 1.01 update (tested on the EUR version, PCSB01128)
- Replace rootast.xai & patch101.xai with english ones first
- Now, we need to fix the japanese patch102.xai with english.
To do that, use the XAST Unpacker to extract english rootast.xai & patch101.xai & japanese patch102.xai
- Now replace all files in patch102.xai with their english version in patch101.xai, or rootast.xai if not in 1.01, except flash/pkg_menu.xai & system/1stload.dat which must not change
- Then, we must patch the japanese system/1stload.dat
- First use the DAT Unpacker to extract both japanese / english system/1stload.dat files
- Now use the `swap.sh` bash script on `1stload.list` and english then japanese directories created in last step.
Something like `./swap.sh 1stload.list 1stload_eng/ 1stload_jap/`, otherwise you can just replace each non-commented file in `1stload.list` with its english version (`swap.sh` does that)
- Now we'll patch the japanese flash/pkg_menu.xai
- First use the XAST Unpacker to extract both japanese / english flash/pkg_menu.xai files
- Now use the `swap.sh` bash script on `pkg_menu.list` and english then japanese directories created in last step.
Something like `./swap.sh pkg_menu.list pkg_menu_eng/ pkg_menu_jap/`, otherwise you can just replace each non-commented file in `pkg_menu.list` with its english version (`swap.sh` does that)
- Some files from 1stload.dat like pl_const.plt & lit_rtxt.csv require manual hacking to work correctly, mostly encoding fixes.
I will not post them here but you can always look for them elsewhere
- Now unpack & then repack .tbb files in 1stload.dat using TBB tools with `--enc shift-jis`, & place these in your patched 1stload folder
- Finally, repack the fixed 1stload.dat & pkg_menu.xai files using the `dat` & `xai` tools, use it as system/1stload.dat & flash/pkg_menu.xai in 102 & then repack 102 using the XAST Packer.
Something like `./xai patch102_fix patch102_fixed.xai patch102_untouched.xai`
- Finished, just use the generated patch102_fixed.xai file as patch102.xai in your japanese game folder & have fun.

## TODO

- Improve some .tbb files
- Improve pl_const.plt hacking
- Improve lit_rtxt.csv if needed
- Fix pkg_menu/help.rsb if any use
- Translate trophies ?


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

Add the `--enc shift-jis` argument to repack a into shift-jis (CSV is unicode)

## Credits

Credits to Tony Blue for helping get the game on 3.60, [weaknespase](https://github.com/weaknespase) for the XAST checksum + TBB tools, & [vampirexxxx](https://github.com/vampirexxxx) who is a big help in hacking the assets !
