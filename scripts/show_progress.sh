#!/bin/bash

WORKING_DIR=$1
PROGRESS_FILE="$WORKING_DIR/.progress"

completed=0
empty=0
first=1
old_completed=0
old_empty=0

echo

while true
do
    total=`head -n 1 "$PROGRESS_FILE"`
    completed=`tail -n +2 "$PROGRESS_FILE" | tr -cd x | wc -m`
    empty=`tail -n +2 "$PROGRESS_FILE" | tr -cd o | wc -m`

    if (( !$first && $completed == $old_completed && $empty == $old_empty ))
    then
        continue
    fi

    first=0
    old_completed=$completed
    old_empty=$empty

    echo -ne "\r"completed $(($completed+$empty)) out of $total \($((($completed+$empty)/$total*100))%\), including $empty non-runnable \($((($empty)/$total*100))%\)
    
    sleep 30s
done    


