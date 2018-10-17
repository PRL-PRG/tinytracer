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
CMD="/home/$USER/Workspace/tinytracer/bin/Rscript"
WORKING_DIR='/tmp/vignettes'
START_TIME=`date +%y-%m-%d_%H-%M`
RESULTS_DIR="$WORKING_DIR/_results/$START_TIME"
LOGS_DIR="$WORKING_DIR/_logs/$START_TIME"
N_PROCESSES=1
PROGRESS_LOG="$LOGS_DIR/.progress"

# Set up option variables using options passed via arguments.
while true
do
    case "$1" in
        -w|--working-dir) WORKING_DIR=$2; shift 2        ;;
        -p|--processes)   N_PROCESSES=$2; shift 2        ;;
        -c|--cmd)         CMD=$2;         shift 2        ;;
        --)                               shift;    break;;
        *)                                          break;;
    esac
done

# Function for running vignettes etc. It takes one argument with 5 fields
# delimited by @:
function run_item {  
    echo ::: starting run item
    eval `split @ arg "$1"`

    echo ::: processing arguments 
    # process arguments
    local type=${arg[0]}     #1 type of item (vignette, test, or example)
    local package=${arg[1]}  #2 package
    local item=${arg[2]}     #3 name of item
    local runnable=${arg[3]} #4 path to executable
    local wd=${arg[4]}       #5 working directory

    echo ::: making local variables
    # some shorthand stuff
    local log_dir="$LOGS_DIR/composition"
    local log_file="$log_dir/${type}_${package}_${item}.log"
    local comp_dir="$RESULTS_DIR/composition"
    local comp_file="$comp_dir/${type}_${package}_${item}.csv"

    echo ::: preparing paths
    # prepare paths
    cd "$wd" 
    mkdir -p "$log_dir"
    mkdir -p "$comp_dir"

    echo ::: running script
    # run script
    if [ "$runnable" = NA ]
    then    
    	echo Nothing to run for \"$item\",a $type from package $package
        echo -n "o" >> $PROGRESS_LOG
    else 
        echo Executing \"$item\", a $type from package $package
        echo Runnable at "$runnable"
        echo Saving composition data to "$comp_file"
        SEXP_INSPECTOR_COMPOSITION="$comp_file" \
            "$CMD" "$runnable" 2 >& 1 | tee "$log_file"
        echo Done executing \"$item\", a $type from package $package
        echo -n "x" >> "$PROGRESS_LOG"
    fi

    return 0
}

# Composition of arguments into a string delimited by a custom character, aka
# "join." Takes at least two arguments: 
#   $1:      separator
#   $2-...:  arguments to be joined
function join {
    local IFS="$1" # separator has to be one character
    shift
    echo "$*"
}

# Decompose a string delimited by a custom character into an array, return the
# definition of an array, aka "split." Takes 3 arguments:
#  $1:       separator
#  $2:       name of the returned array
#  $3:       string to be split
function split { #eval result
    local IFS="$1"
    local name="$2"
    shift 2
    local d_arg=($*)
    local d_arg_def=`declare -p d_arg`
    declare -a "$name="${d_arg_def#*=}
    declare -p "$name" 
}

# We export variables and functions so that they are visible in subprocesses.
export CMD
export LOGS_DIR
export RESULTS_DIR
export PROGRESS_LOG
export -f run_item
export -f join
export -f split

# Body ########################################################################

echo retrieving data from "$WORKING_DIR/info.csv"

# Data from the info file split into specific columns.
types=($(csvtool -t ',' namedcol type "$WORKING_DIR/info.csv" | tail -n +2)) # parens make an array
packages=($(csvtool -t ',' namedcol package "$WORKING_DIR/info.csv" | tail -n +2))
items=($(csvtool -t ',' namedcol item "$WORKING_DIR/info.csv" | tail -n +2))
wds=($(csvtool -t ',' namedcol working_directory "$WORKING_DIR/info.csv" | tail -n +2))
runnables=($(csvtool -t ',' namedcol instrumented_source_file "$WORKING_DIR/info.csv" | tail -n +2))

echo composing parameter info

# Compose arguments
composition=$(\
    for i in $(seq 0 $((${#types[@]} - 1)))
    do 
        echo `join @ \
            "${types[i]}" "${packages[i]}" "${items[i]}" \
            "${runnables[i]}" "${wds[i]}"`
    done
)

# R environmental variables
export R_LIBS=/home/kondziu/R/installed/tinytracer/
export R_COMPILE_PKG=1
export R_DISABLE_BYTECODE=0
export R_ENABLE_JIT=3
export R_KEEP_PKG_SOURCE=no

echo starting

echo results at $RESULTS_DIR
echo logs at $LOGS_DIR


mkdir -p `dirname "$PROGRESS_LOG"`
echo $composition | wc -w > "$PROGRESS_LOG"

# Run function in parallel
parallel --verbose --gnu --keep-order -j${N_PROCESSES} --link run_item ::: $composition

echo done with everything
