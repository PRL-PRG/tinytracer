#!/bin/bash

LOG_FILE="$1"

if [ ! -f "$LOG_FILE" ]
then
    echo "Not a log file: $LOG_FILE" >&2
    exit 1
fi

<"$LOG_FILE" awk '
BEGIN          { good=0; bad=0; ugly=0; skipped=0; repeats=0; blacklisted=0 }
$1 == "total"  { total=$2; next }
$1 == "start"  { processes[$2]="?"; next }
$1 == "skip"   { skipped++ }
$1 == "repeat" { repeats++ }
$1 == "blacklisted" { blacklisted++ }
$1 == "stop"   { if ($3 in processes) {
                     if (processes[$3] != "?") { ugly++ }
                     else if($2 == "0")        { good++ }
                     else                      { bad++  }
                     delete processes[$3]
                 } else                        { ugly++ }
                 next                                              }
END            { unfinished=0; for (p in processes) unfinished++
                 done = good + bad + ugly + skipped + repeats + blacklisted

                 print "good         " good
                 print "bad          " bad
                 print "ugly         " ugly
                 print "repeats      " repeats
                 print "unfinished   " unfinished
                 print "skipped      " skipped
                 print "blacklisted  " blacklisted
                 print "-------------------"
                 print "done         " done
                 print "unfinished   " unfinished
                 print "to do        " (total - unfinished - done) 
                 print "-------------------"
                 print "total        " total }'

