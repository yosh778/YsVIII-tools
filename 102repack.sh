#!/bin/bash

PKG_MENU_FIX_DIR=pkg_menu_fix
PKG_MENU_JP_XAI=pkg_menu_jp.xai
PKG_MENU_OUT=pkg_menu_fix.xai

PATCH_102_FIX_DIR=patch102_fix
PATCH_102_REAL=patch102_real.xai
PATCH_102_OUT=patch102_fix.xai

DAT_JP=1stload_jp.dat
DAT_FIX_DIR=1stload_fix
DAT_OUT=1stload_fix.dat

DAT_LIST=1stload.list
DAT_ENG_DIR=1stload_allEng


./xai $PKG_MENU_FIX_DIR $PKG_MENU_OUT $PKG_MENU_JP_XAI
cp -f $PKG_MENU_OUT $PATCH_102_FIX_DIR/flash/pkg_menu.xai

rm $DAT_FIX_DIR/*
./undat $DAT_JP $DAT_FIX_DIR
./swap.sh $DAT_LIST $DAT_ENG_DIR $DAT_FIX_DIR

cp -f custom/lit_rtxt.csv $DAT_FIX_DIR
cp -f custom/lit_rtxt.csv $PATCH_102_FIX_DIR/system/lit_rtxt.csv

./dat $DAT_FIX_DIR $DAT_OUT
cp -f $DAT_OUT $PATCH_102_FIX_DIR/system/1stload.dat

./xai $PATCH_102_FIX_DIR $PATCH_102_OUT $PATCH_102_REAL
cp -f $PATCH_102_OUT patch102.xai

# Some checks
diff $PATCH_102_FIX_DIR/flash/pkg_menu.xai $PKG_MENU_OUT
diff $PATCH_102_FIX_DIR/system/1stload.dat $DAT_OUT
