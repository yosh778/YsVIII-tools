#!/bin/bash

DIR=$1

#cp -f patch101_fix.xai $DIR/app/PCSG00881/patch101.xai
cp -f patch102_fix.xai $DIR/app/PCSG00881/patch102.xai
# cp -f patch102_fix_ch.xai $DIR/app/PCSH00297/patch102.xai

# ./xaiPatch patch101_fix.xai "shift-jis_dist/patch101/script/mp1201.bin" script/mp1201.bin
# ./xaiPatch $DIR/app/PCSG00881/patch101.xai "shift-jis_dist/patch101/script/mp1201.bin" script/mp1201.bin

# ./xaiPatch $DIR/app/PCSG00881/rootast.xai "jap/base/movie/logo.mp4" movie/logo.mp4

# for script in script/*; do
#     ./xaiPatch $DIR/app/PCSG00881/rootast.xai $script $script > /dev/null
# done

#./xaiPatch $DIR/app/PCSG00881/rootast.xai custom/mp4102t2.bin script/mp4102t2.bin

./patchEboot.py $DIR/app/PCSG00881/eboot.bin
./patchEboot.py $DIR/app/PCSG00881/mai_moe/eboot_origin.bin

# ./patchEboot.py $DIR/app/PCSH00297/eboot.bin > /dev/null
# ./patchEboot.py $DIR/app/PCSH00297/mai_moe/eboot_origin.bin > /dev/null


echo "Unmounting '${DIR}', please wait ..."
umount $DIR -v
