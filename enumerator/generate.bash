#!/bin/bash

function usage_error {
    echo "ERROR: USAGE - $0 prefix"
    exit 1
}

function file_error {
    echo "ERROR: $0 - missing necessary file $1"
    exit 2
}

function cmd_error {
    echo "ERROR: $0 - command failed: $*"
    exit 3
}

if [[ $# -ne 1 ]]; then
    usage_error
fi

pref=$1
upref=$(echo $pref | tr '[a-z]' '[A-Z]')
lpref=$(echo $pref | tr '[A-Z]' '[a-z]')

# get source directory
sdir=$(dirname $0)

# find source files
doth=$sdir/enumerator.h
dotc=$sdir/enumerator.c
[[ -f $doth ]] || file_error $doth
[[ -f $dotc ]] || file_error $dotc

# create new files with given prefix, and substituting prefix
sed_cmd="sed -e s/PREFIX/$upref/g -e s/prefix/$lpref/g"
echo "DEBUG: sed_cmd - $sed_cmd"
cmd="$sed_cmd $doth > ./${lpref}_enumerator.h"
echo "DEBUG: cmd - $cmd"
$sed_cmd $doth > ./${lpref}_enumerator.h || cmd_error $cmd

cmd="$sed_cmd $dotc > ./${lpref}_enumerator.c"
$sed_cmd $dotc > ./${lpref}_enumerator.c || cmd_error $cmd

exit 0

