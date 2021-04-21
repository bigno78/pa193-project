#!/bin/bash

if [ $# -lt 3 ]
then
    echo "Usage: $0 <parser-executable> <output_dir> <inputs>" >&2
    exit 1
fi

par=$1
out_path=$2

shift
shift

if [ -e $out_path ]
then
    mkdir -p $out_path
fi

for f in "$@"
do
    ./$par $f -d $out_path
    echo Error: `basename $f`
    if [ $? -ne 0 ]
    then
        exit 1
    fi
    echo ----------------------------------------------------------------------
done