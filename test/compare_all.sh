#!/bin/bash

if [ $# -ne 3 ]; then
    echo "Usage: $0 <compare_script> <reference_dir> <output_dir>" >&2
    exit 1
fi

comp=$1
ref_path=$2
out_path=$3

for f in $(ls $out_path/*.json); do
    file=$(basename $f)
    if [ -e $ref_path/$file ]; then
        echo $file 
        python3 $comp $ref_path/$file $f
    fi
done
