#!/bin/bash

# Requires: getopt csvtool parallel (GNU parallel) 

# Preamble: option processing #################################################

# Set up option parsing with short and long options via getopt.
TEMP=$(getopt -o Dw:p:c:L:R: --long do-not-repeat,working-dir:,processes:,compiler:,cmd:,start-from:,end-at:,logs-dir:,results-dir: -- "$@")

# Check if parsing worked.
[ $? != 0 ] && echo "Option parsing failed." >&2 && exit 1

# Set the parsed options as arguments.
eval set -- "$TEMP"

# Constants (possibly overriden by arguments).
CMD="/home/$USER/tinytracer/bin/Rscript"
WORKING_DIR='/tmp/vignettes'
START_TIME=`date +%y-%m-%d`
RESULTS_DIR="$WORKING_DIR/_results/$START_TIME"
LOGS_DIR="$WORKING_DIR/_logs/$START_TIME"
N_PROCESSES=1
COMPILER=jit
START_FROM=0
END_AT=0
DO_NOT_REPEAT=false

# Set up option variables using options passed via arguments.
while true
do
    case "$1" in
        -w|--working-dir)   WORKING_DIR="$2";   shift 2        ;;
        -R|--results-dir)   RESULTS_DIR="$2";   shift 2        ;;
        -L|--logs-dir)      LOGS_DIR="$2";      shift 2        ;;
        -p|--processes)     N_PROCESSES="$2";   shift 2        ;;
        -c|--cmd)           CMD="$2";           shift 2        ;;
        --compiler)         COMPILER="$2";      shift 2        ;;
        --start-from)       START_FROM="$2";    shift 2        ;;
        --end-at)           END_AT="$2";        shift 2        ;;
	-D|--do-not-repeat) DO_NOT_REPEAT=true; shift          ;;
        --)                                     shift;    break;;
        *)                                                break;;
    esac
done

# Function for running vignettes etc. It takes one argument with 5 fields
# delimited by @:
function run_item {  
    echo ::: starting run item
    eval `split @ arg "$1"`

    echo ::: processing arguments 
    # process arguments
    local type=${arg[1]}     #1 type of item (vignette, test, or example)
    local package=${arg[2]}  #2 package
    local item=${arg[3]}     #3 name of item
    local runnable=${arg[4]} #4 path to executable
    local wd=${arg[5]}       #5 working directory

    echo ::: making local variables
    # some shorthand stuff
    local log_dir="$LOGS_DIR/composition/$COMPILER/"
    local log_file="$log_dir/${type}_${package}_${item}.log"
    local comp_dir="$RESULTS_DIR/composition/$COMPILER/"
    local comp_file="$comp_dir/${type}_${package}_${item}_%d.csv"

    echo ::: preparing paths
    # prepare paths
    cd "$wd" 
    mkdir -p "$log_dir"
    mkdir -p "$comp_dir"

    # run script
    if repeats "$package" "$item"
    then
        echo "Item \"$item\" was run in a previous execution, skipping"
	echo repeat "$1" >> "$PROGRESS_LOG"
    elif [ "$runnable" = NA ]
    then    
    	echo Nothing to run for \"$item\", a $type from package $package
        echo skip "$1"  >> "$PROGRESS_LOG"
    else 
	echo Executing \"$item\", a $type from package $package
        echo Runnable at "$runnable"
        echo Saving composition data to "$comp_file"
        echo start "$1" >> "$PROGRESS_LOG"
        SEXP_INSPECTOR_COMPOSITION="$comp_file" \
            "$CMD" "$runnable" 2 >& 1 | tee "$log_file"
	local result="$?"
        echo Done executing \"$item\", a $type from package $package
        echo stop "$result" "$1" >> "$PROGRESS_LOG"
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

# Creates a list of already run vignettes from existing progress files.
# $1: LOGS_DIR
function generate_repeat_list {
    for file in `find "$1" -name '.progress_*'`
    do
        <"$file" awk '
	function just_vignette_id(string) {
	    split(string, result, "@")
	    return(result[3]"/"result[4])    
        }
	$1 == "stop"   { print just_vignette_id($3) }
	$1 == "skip"   { print just_vignette_id($2) }
	$1 == "repeat" { print just_vignette_id($2) }'
    done | sort | uniq
}

# Checks if a specific vignette is in the list of already executed vignettes.
# $1: package
# $2: vignette
function repeats {
    echo $REPEAT_LIST | tr " " "\n" \
	              | grep -w $1/$2 >/dev/null \
		      && return 0 \
		      || return -1
}

# We expout variables and functions so that they are visible in subprocesses.
export CMD
export LOGS_DIR
export RESULTS_DIR
export PROGRESS_LOG
export COMPILER
export DO_NOT_REPEAT
export -f run_item
export -f join
export -f split
export -f repeats 

# Body ########################################################################

echo retrieving data from "$WORKING_DIR/info.csv"

# Data from the info file split into specific columns.
types=($(csvtool -t ',' namedcol type "$WORKING_DIR/info.csv" | tail -n +2)) # parens make an array
packages=($(csvtool -t ',' namedcol package "$WORKING_DIR/info.csv" | tail -n +2))
items=($(csvtool -t ',' namedcol item "$WORKING_DIR/info.csv" | tail -n +2))
wds=($(csvtool -t ',' namedcol working_directory "$WORKING_DIR/info.csv" | tail -n +2))
runnables=($(csvtool -t ',' namedcol instrumented_source_file "$WORKING_DIR/info.csv" | tail -n +2))

echo retrieved ${#types[@]} rows

# Compose arguments
echo composing parameter info
composition=$(\
    for i in $(seq 0 $((${#types[@]} - 1)))
    do 
        echo `join @ $i \
            "${types[i]}" "${packages[i]}" "${items[i]}" \
            "${runnables[i]}" "${wds[i]}"`
    done
)

if (( $START_FROM != 0 ))
then
    echo trimming composition to start from $START_FROM
    composition=`echo $composition | tr ' ' "\n" | tail -n +$(($START_FROM + 1))`
fi

if (( $END_AT != 0 ))
then
    echo trimming composition to end at $END_AT
    composition=`echo $composition | tr ' ' "\n" | head -n +$(($END_AT - $START_FROM + 1))`
fi

#echo $composition

# R environmental variables
echo setting R environmental variables

export R_KEEP_PKG_SOURCE=no

if [ $COMPILER = 'jit' ];
then
    export R_LIBS=/data/kondziu/R/installed/
    export R_COMPILE_PKG=1
    export R_DISABLE_BYTECODE=0
    export R_ENABLE_JIT=3
elif [ $COMPILER = 'disable_bytecode' ]
then
    export R_LIBS=/data/kondziu/R/installed/
    export R_COMPILE_PKG=0
    export R_DISABLE_BYTECODE=1
    export R_ENABLE_JIT=0
else 
    echo "Unknown compiler setting: $COMPILER"
    exit 2
fi

echo R_KEEP_PKG_SOURCE=$R_KEEP_PKG_SOURCE
echo R_LIBS=$R_LIBS
echo R_COMPILE_PKG=$R_COMPILE_PKG
echo R_DISABLE_BYTECODE=$R_DISABLE_BYTECODE
echo R_ENABLE_JIT=$R_ENABLE_JIT

echo starting

echo results at $RESULTS_DIR
echo logs at $LOGS_DIR

export PROGRESS_LOG="$LOGS_DIR/.progress_$$"
#export PROGRESS_LOCK="$LOGS_DIR/.progress_lock_$$"
mkdir -p `dirname "$PROGRESS_LOG"`
echo total `echo $composition | wc -w` > "$PROGRESS_LOG"



if $DO_NOT_REPEAT 
then
   echo DO NOT REPEAT mode is on
   REPEAT_LIST=`generate_repeat_list "$LOGS_DIR"`
   echo `echo $REPEAT_LIST | wc -w` vignettes have already run
else	
   echo DO NOT REPEAT mode is off
   REPEAT_LIST=
fi
export REPEAT_LIST

echo progress log at $PROGRESS_LOG

# Run function in parallel
#parallel --verbose --gnu --keep-order -j${N_PROCESSES} --link run_item ::: $composition
# I'm removing keep order to see if it'll prevent problems with file handles
parallel --verbose --gnu -j${N_PROCESSES} --link run_item ::: $composition

echo done with everything
