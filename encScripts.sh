

clear
clear

rm -rf "shift-jis_dist/"

DIR="shift-jis_dist"

mkdir -p $DIR


for path in eng/*/script/*; do
    ./bin2script "$path" 2>err > tmp
    exit_status=$?
    cat err | grep -v "Invalid"

    if [ $exit_status -eq 0 ]; then
        PREV="${path%%/*}"
        NEXT="${path#*/}"
        DEST="${PREV}_enc/${NEXT}"
        ./script2bin tmp "$DEST" --enc-shift-jis > /dev/null
        # echo $path
    fi
done

mv "${PREV}_enc" "$DIR"


for path in 1stload_allEng_script/*; do
    ./bin2script "$path" 2>err > tmp
    exit_status=$?
    cat err | grep -v "Invalid"

    if [ $exit_status -eq 0 ]; then
        PREV="${path%%/*}"
        NEXT="${path#*/}"
        DEST="${PREV}_enc/${NEXT}"
        ./script2bin tmp "$DEST" --enc-shift-jis > /dev/null
        # echo $path
    fi
done

mv "${PREV}_enc" "$DIR"


for path in patch102_eng/script/*; do
    ./bin2script "$path" 2>err > tmp
    exit_status=$?
    cat err | grep -v "Invalid"

    if [ $exit_status -eq 0 ]; then
        PREV="${path%%/*}"
        NEXT="${path#*/}"
        DEST="${PREV}_enc/${NEXT}"
        ./script2bin tmp "$DEST" --enc-shift-jis > /dev/null
        # echo $path
    fi
done

mv "${PREV}_enc" "$DIR"


for path in sceneFixes/*; do
    ./bin2script "$path" 2>err > tmp
    exit_status=$?
    cat err | grep -v "Invalid"

    if [ $exit_status -eq 0 ]; then
        PREV="${path%%/*}"
        NEXT="${path#*/}"
        DEST="${PREV}_enc/${NEXT}"
        ./script2bin tmp "$DEST" --enc-shift-jis > /dev/null
        # echo $path
    fi
done

mv "${PREV}_enc" "$DIR"

cp -f "$DIR"/sceneFixes_enc/mp4102t2.bin "$DIR"/eng_enc/base/mp4102t2.bin
cp -f "$DIR"/sceneFixes_enc/mp1103.bin "$DIR"/eng_enc/patch101/mp1103.bin


rm -rf "$DIR"/eng_enc/text_fr/ "$DIR"/eng_enc/patch102_fake

mv "$DIR"/eng_enc/* "$DIR"
rmdir "$DIR"/eng_enc

