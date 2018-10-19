#!/bin/bash

DIR_1="$1"
DIR_2="$2"

file_list_1=`mktemp`
file_list_2=`mktemp`

echo $file_list_1
echo $file_list_2

find "$DIR_1/" -name '*.csv' -type f -printf "%f\n" | cut -f 1,2,3 -d_ | sort | uniq > $file_list_1 
find "$DIR_2/" -name '*.csv' -type f -printf "%f\n" | cut -f 1,2,3 -d_ | sort | uniq > $file_list_2

n_same_files=`comm -1 -2 $file_list_1 $file_list_2 | wc -l`
n_left_files=`comm -2 -3 $file_list_1 $file_list_2 | wc -l`
n_right_files=`comm -1 -3 $file_list_1 $file_list_2 | wc -l`

echo number of same files: $n_same_files
echo number of files exclusive to left set: $n_left_files
echo number of files exclusive to right set: $n_right_files
echo 

echo moving $DIR_1 to $DIR_1/all
tmp_dir_1=`mktemp -d`
mv "$DIR_1"/* $tmp_dir_1 && \
mkdir -p "$DIR_1/all" && \
mv "$tmp_dir_1/"* "$DIR_1/all"

echo copying file from "$DIR_1/all" to "$DIR_1/cropped"
cp -r "$DIR_1/all" "$DIR_1/cropped"

echo removing excess files from "$DIR_1/cropped"
for f in `comm -2 -3 $file_list_1 $file_list_2`
do
    rm -v "$DIR_1/cropped/${f}_"*.csv
done

echo moving $DIR_2 to $DIR_2/all
tmp_dir_2=`mktemp -d`
mv "$DIR_2"/* $tmp_dir_2 && \
mkdir -p "$DIR_2/all" && \
mv "$tmp_dir_2"/* "$DIR_2/all"

echo copying file from "$DIR_2/all" to "$DIR_2/cropped"
cp -r "$DIR_2/all" "$DIR_2/cropped"

echo removing excess files from "$DIR_2/cropped"
for f in `comm -1 -3 $file_list_1 $file_list_2`
do
    rm -v "$DIR_2/cropped/${f}_"*.csv
done

find "$DIR_1/cropped" -name '*.csv' -type f -printf "%f\n" | cut -f 1,2,3 -d_ | sort | uniq > $file_list_1 
find "$DIR_2/cropped" -name '*.csv' -type f -printf "%f\n" | cut -f 1,2,3 -d_ | sort | uniq > $file_list_2

n_same_files=`comm -1 -2 $file_list_1 $file_list_2 | wc -l`
n_left_files=`comm -2 -3 $file_list_1 $file_list_2 | wc -l`
n_right_files=`comm -1 -3 $file_list_1 $file_list_2 | wc -l`

echo number of same files in cropped: $n_same_files
echo number of files exclusive to left cropped set: $n_left_files
echo number of files exclusive to right cropped set: $n_right_files
echo 
