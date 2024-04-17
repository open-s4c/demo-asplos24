#!/bin/bash
# ------------------------------------------------------------------------------
# Run test program until input and output differ. Input is a figure.
#
# Dependencies:
# - viu (https://github.com/atanunq/viu)
# - convert (ImageMagick)
# ------------------------------------------------------------------------------

fn=$1 # figure filename
catprog="./ccat"

if [ -z "$fn" ]; then
    echo "usage: $0 <figure>"
    exit 1
fi

if [ ! -f "$fn" ]; then
    echo "could not find $fn"
    exit 1
fi

# check input is a figure file
ext=$(file --extension "$fn" | cut -d":" -f2 | tr -d " ")
if [ "$ext" != "jpeg/jpg/jpe/jfif" ] \
&& [ "$ext" != "png" ] \
&& [ "$ext" != "gif" ]; then
    echo "$fn is not a supported figure type"
    exit 1
fi

if [ "$ext" == "jpeg/jpg/jpe/jfif" ]; then
    ext="jpg"
fi

# u"se the same extension for the output and temp files
output="output.$ext"
temp="temp.$ext"

if [ ! -f "$catprog" ]; then
    echo "could not find $catprog"
    exit 1
fi

# remove left overs
rm -f $temp $output

# calculate expected MD5 sum
expected=$(md5sum $fn | cut -d" " -f1)

i=0
time while true; do
    i=$(echo "$i + 1" | bc)

    # run test program
    $catprog $fn > $temp

    # compare sum and stop script if they differ
    sum=$(md5sum $temp | cut -d" " -f1)
    if [ true ] && [ "$sum" != "$expected" ]; then
        echo "Incorrect md5sum @ run $i"
        echo "Expected: $expected ($fn)"
        echo "Actual  : $sum ($temp)"
	convert $fn $temp +append $output
	viu $output
	exit 1
    fi
done

