#!/bin/bash

par=$1
in_path=$2
out_path=$3

echo `pwd`

if [ -e $out_path ]
then
    mkdir -p $out_path
fi

for f in `ls $in_path/*.txt`
do
    ./$par $f
    if [ $? -ne 0 ]
    then
        echo Error: `basename $f`
        exit 1
    fi
done
