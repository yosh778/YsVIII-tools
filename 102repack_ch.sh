#!/bin/bash

PKG_MENU_ENG_DIR=pkg_menu_eng
PKG_MENU_FIX_DIR=pkg_menu_fix_ch
PKG_MENU_CH_XAI=pkg_menu_ch.xai
PKG_MENU_OUT=pkg_menu_fix_ch.xai
PKG_MENU_LIST=pkg_menu.list

PATCH_102_FIX_DIR=patch102_fix_ch
PATCH_102_REAL=patch102_real_ch.xai
PATCH_102_OUT=patch102_fix_ch.xai

DAT_CH=1stload_ch.dat
DAT_FIX_DIR=1stload_fix_ch
DAT_OUT=1stload_fix_ch.dat

DAT_LIST=1stload.list
DAT_ENG_DIR=1stload_allEng

TBB_LIST=tbb2shift.list


rm $PKG_MENU_FIX_DIR/*
./unxai $PKG_MENU_CH_XAI $PKG_MENU_FIX_DIR > /dev/null
./swap.sh $PKG_MENU_LIST $PKG_MENU_ENG_DIR $PKG_MENU_FIX_DIR > /dev/null

./xai $PKG_MENU_FIX_DIR $PKG_MENU_OUT $PKG_MENU_CH_XAI > /dev/null
cp -f $PKG_MENU_OUT $PATCH_102_FIX_DIR/flash/pkg_menu.xai

rm $DAT_FIX_DIR/*
./undat $DAT_CH $DAT_FIX_DIR > /dev/null
./swap.sh $DAT_LIST $DAT_ENG_DIR $DAT_FIX_DIR > /dev/null

cp -f custom/lit_rtxt_ch.csv $DAT_FIX_DIR/lit_rtxt.csv
cp -f custom/lit_rtxt_ch.csv $PATCH_102_FIX_DIR/system/lit_rtxt.csv

cp -f custom/pl_const_ch.plt $DAT_FIX_DIR/pl_const.plt
cp -f custom/pl_const_ch.plt $PATCH_102_FIX_DIR/flash/pl_const.plt


# mkdir -p tbb
# ./swap.sh $TBB_LIST $DAT_ENG_DIR tbb > /dev/null

cp -f tbb_utf8/* $DAT_FIX_DIR


./dat $DAT_FIX_DIR $DAT_OUT > /dev/null
cp -f $DAT_OUT $PATCH_102_FIX_DIR/system/1stload.dat

./xai $PATCH_102_FIX_DIR $PATCH_102_OUT $PATCH_102_REAL > /dev/null
cp -f $PATCH_102_OUT patch102.xai

# Some checks
diff $PATCH_102_FIX_DIR/flash/pkg_menu.xai $PKG_MENU_OUT
diff $PATCH_102_FIX_DIR/system/1stload.dat $DAT_OUT
