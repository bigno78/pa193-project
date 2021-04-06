#!/bin/bash

if [ $# -ne 3 ]
then
    echo "Usage: $0 <parser-executable> <input_dir> <output_dir>" >&2
    exit 1
fi

par=$1
in_path=$2
out_path=$3

if [ -e $out_path ]
then
    mkdir -p $out_path
fi

for f in `ls $in_path/*.txt`
do
    ./$par $f -o $out_path
    if [ $? -ne 0 ]
    then
        echo Error: `basename $f`
        exit 1
    fi
done
