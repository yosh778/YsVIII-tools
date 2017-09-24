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

    echo "Copying $filename from $input to $output"
    cp -f "$input"/"$filename" "$output"/"$filename"

done < "$1"

echo "Script $1 finished"

