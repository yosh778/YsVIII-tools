#!/bin/bash

PKG_MENU_ENG_DIR=pkg_menu_eng
PKG_MENU_FIX_DIR=pkg_menu_fix
PKG_MENU_JP_XAI=pkg_menu_jp.xai
PKG_MENU_OUT=pkg_menu_fix.xai
PKG_MENU_LIST=pkg_menu.list

PATCH_102_FIX_DIR=patch102_fix
PATCH_102_REAL=patch102_real.xai
PATCH_102_OUT=patch102_fix.xai

DAT_JP=1stload_jp.dat
DAT_FIX_DIR=1stload_fix
DAT_OUT=1stload_fix.dat

DAT_LIST=1stload.list
DAT_ENG_DIR=1stload_allEng

TBB_LIST=tbb2shift.list


rm $PKG_MENU_FIX_DIR/*
./unxai $PKG_MENU_JP_XAI $PKG_MENU_FIX_DIR > /dev/null
./swap.sh $PKG_MENU_LIST $PKG_MENU_ENG_DIR $PKG_MENU_FIX_DIR > /dev/null

./xai $PKG_MENU_FIX_DIR $PKG_MENU_OUT $PKG_MENU_JP_XAI > /dev/null
cp -f $PKG_MENU_OUT $PATCH_102_FIX_DIR/flash/pkg_menu.xai

rm $DAT_FIX_DIR/*
./undat $DAT_JP $DAT_FIX_DIR > /dev/null
./swap.sh $DAT_LIST $DAT_ENG_DIR $DAT_FIX_DIR > /dev/null

cp -f custom/lit_rtxt.csv $DAT_FIX_DIR
cp -f custom/lit_rtxt.csv $PATCH_102_FIX_DIR/system/lit_rtxt.csv

cp -f custom/pl_const.plt $DAT_FIX_DIR
cp -f custom/pl_const.plt $PATCH_102_FIX_DIR/flash/pl_const.plt

cp -f custom/mp1103.bin $PATCH_102_FIX_DIR/script/mp1103.bin


# ./script2bin custom/item_test.ys custom/item.bin --enc-shift-jis > /dev/null
# ./script2bin custom/talk.ys custom/talk.bin --enc-shift-jis > /dev/null
./script2bin custom/mp1204_credits.ys custom/mp1204.bin --enc-shift-jis > /dev/null
./script2bin custom/mp1101_mod.ys custom/mp1101.bin --enc-shift-jis > /dev/null
#./script2bin custom/talk_test.ys custom/talk.bin > /dev/null

cp -f custom/mp1204.bin $PATCH_102_FIX_DIR/script/mp1204.bin
#cp -f custom/mp1101.bin $PATCH_102_FIX_DIR/script/mp1101.bin

#cp -f custom/item_fix.bin custom/item.bin


 # cp -f custom/item.bin $DAT_FIX_DIR
#cp -f custom/talk.bin $DAT_FIX_DIR

# mkdir -p tbb
# ./swap.sh $TBB_LIST $DAT_ENG_DIR tbb > /dev/null

cp -f tbb/* $DAT_FIX_DIR


./dat $DAT_FIX_DIR $DAT_OUT > /dev/null
cp -f $DAT_OUT $PATCH_102_FIX_DIR/system/1stload.dat

./xai $PATCH_102_FIX_DIR $PATCH_102_OUT $PATCH_102_REAL > /dev/null
cp -f $PATCH_102_OUT patch102.xai

# Some checks
diff $PATCH_102_FIX_DIR/flash/pkg_menu.xai $PKG_MENU_OUT
diff $PATCH_102_FIX_DIR/system/1stload.dat $DAT_OUT

