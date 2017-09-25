#!/bin/bash

display_usage()
{
    echo "Usage : $0 <fileList> <inputDir> <outputDir>" 
    echo -e "  Copies the filenames in <fileList> from <inputDir> to <outputDir>" 
}

if [  $# -lt 3 ] 
then 
    display_usage
    exit 1
fi 

input=$2
output=$3

while IFS='' read -r filename || [[ -n "$filename" ]]; do

	[[ "$filename" =~ ^#.*$ ]] && continue
	[[ -z "$filename" ]] && continue

	if [ $1 = tbb2shift.list ]; then
		echo "Converting $filename from $input to $output as SHIFT-JIS"

		node tbbconv/tbbconv.js unpack "$input"/"$filename" tmp
		node tbbconv/tbbconv.js pack tmp "$output"/"$filename" --enc shift-jis
		# mkdir -p tbb
		# cp tmp tbb/"$filename".csv

	else
		echo "Copying $filename from $input to $output"
		cp -f "$input"/"$filename" "$output"/"$filename"
	fi

done < "$1"

echo "Script $1 finished"

