#!/bin/bash

n=$#

#first=1
declare -a working_dirs
declare -a total
declare -a completed
declare -a old_completed
declare -a empty
declare -a old_empty

for i in `seq 1 $n`
do
    working_dirs[$i]=$1; shift
    completed[$i]=0
    empty[$i]=0
done

#echo 

while true
do
    #echo -ne "\r"
    
    for i in `seq 1 $n`
    do
        file="${working_dirs[$i]}/.progress"

        total[$i]=`head -n 1 $file`
        completed[$i]=`tail -n +2 "$file" | tr -cd x | wc -m`
        empty[$i]=`tail -n +2 "$file" | tr -cd o | wc -m`

        sum=$((${completed[$i]}+${empty[$i]}))
        echo -e "$(basename ${working_dirs[$i]})  \tcompleted $sum/${total[$i]} ($(($sum/${total[$i]}*100))%) including ${empty[$i]} empty"
    done

    sleep 5s
    echo -ne "\033[`echo -n $n`A"
done    

echo
