#!/bin/bash

# Requires: getopt csvtool parallel

# Preamble: option processing #################################################

# Set up option parsing with short and long options via getopt.
TEMP=$(getopt -o w:p: --long working-dir:processes: -- "$@")

# Check if parsing worked.
[ $? != 0 ] && echo "Option parsing failed." >&2 && exit 1

# Set the parsed options as arguments.
eval set -- "$TEMP"

# Constants (possibly overriden by arguments).
WORKING_DIR='/tmp/vignettes'
N_PROCESSES=1

# Set up option variables using options passed via arguments.
while true
do
    case "$1" in
        -w|--working-dir) WORKING_DIR=$2; shift 2;;
	-p|--processes)   N_PROCESSES=$2; shift 2;;
	--) shift; break;;
        *)  break;;
    esac
done

# Body ########################################################################

# Data from the info file split into specific columns.
types=$(csvtool -t ',' namedcol type "$WORKING_DIR/info.csv" | tail -n +2)
packages=$(csvtool -t ',' namedcol package "$WORKING_DIR/info.csv" | tail -n +2)
items=$(csvtool -t ',' namedcol item "$WORKING_DIR/info.csv" | tail -n +2)
wds=$(csvtool -t ',' namedcol working_directory "$WORKING_DIR/info.csv" | tail -n +2)
runnables=$(csvtool -t ',' namedcol runnable_source_file "$WORKING_DIR/info.csv" | tail -n +2)

# Function for running vignettes etc. It takes four arguments:
# $1 - type of item (vignette, test, or example)
# $2 - package
# $3 - name of item
# $4 - path to the executable
# $5 - working directory
function run_item {
    cd "$5" 
    if [ "$4" = NA ]
    then    
	echo Nothing to run for \"$3\",a $1 from package $2 #2 >& 1 | tee 1> "$5/$1_$2_$3.log"
    else 
        echo Executing \"$3\", a $1 from package $2
        Rscript "$4" 2 >& 1 1> "$5/$1_$2_$3.log"
        echo Done executing $3, a $1 from package $2
    fi
}

# We export the function so that it is visible in subprocesses.
export -f run_item

# R environmental variables
export R_LIBS=/home/kondziu/R/installed/tinytracer/
export R_COMPILE_PKG=1
export R_DISABLE_BYTECODE=0
export R_ENABLE_JIT=3
export R_KEEP_PKG_SOURCE=no

# Run function in parallel
parallel -j${N_PROCESSES} --link \
         run_item ::: $types ::: $packages ::: $items ::: $runnables ::: $wds
