#!/bin/sh
######################################################################
## collect_file.sh -- XX Script
##
## - Author: tomyama  Dec 2026
## - Version: $Revision$
## - $Date$
## - Only for personal use !
######################################################################

## script entry point
sh_main()
{
    sh_init "$@"
    cd "$apppath/.."

    parse_input "$@"

#    for arg in "$@"; do
#        printf "arg: \`%s'\n" "$arg"
#    done

    utility_dir="../tomyama_utility"
    collect_source_file "test/tmym_common.c" "$utility_dir/tmym_common.c"
    collect_source_file "test/tmym_common.h" "$utility_dir/tmym_common.h"
}

## script setup
sh_init()
{
    di_work="`pwd`"
    appname="`basename \"$0\"`"
    di_tmp="`dirname  \"$0\"`"
    cd "$di_tmp/"; apppath="`pwd`"; cd "$di_work/"
    unset di_tmp
    version=`grep '$[R]evision' "$apppath/$appname" | \
        sed 's/^.*$R/R/' | sed 's/ *\$$//'`
}

## argument analysis
parse_input()
{
    while [ 1 ]; do
        arg="$1"
        shift 2>/dev/null
        if [ $? -ne 0 ]; then
            break
        fi

        #echo $arg
        case "$arg" in
        '-v' | '--version')
            #printf '%s' "$appname"
            #grep Revision "$apppath/$appname" | sed -n 's/^##//p'
            echo "$appname - version($version)"
            ;;
        '-h' | '--help')
            usage
            ;;
        '-'*)
            errp "$appname: \`$arg': unknown option"
            errp "`usage`"
            exit 1
            ;;
        *)
            ;;
        esac
    done
}

## error output
errp()
{
    echo "$@" 1>&2
}

## format indication
usage()
{
    echo "usage: $appname [-h|--help] [-v|--version]"
}

## Revision: 1.5
get_epoch_sec_of_last_update_time()
{
    targetfile="$1"
    perl -e 'print( ( stat( $ARGV[ 0 ] ) )[ 9 ], "\n" )' "$targetfile"
}
is_update_necessary()
{
    _basefile="$1"
    _genfile="$2"

    if [ ! -f "$_basefile" ]; then
        echo "$0: error: $_basefile: file not found" 1>&2
        exit 1
    fi

    _epoch_base="`get_epoch_sec_of_last_update_time \"$_basefile\"`"

    if [ -f "$_genfile" ]; then
        _epoch_genfile="`get_epoch_sec_of_last_update_time \"$_genfile\"`"

        if [ "$_epoch_genfile" -ge "$_epoch_base" ]; then
            return 0
        fi
        return 1
    else
        return 2
    fi
}

collect_source_file()
{
    _dstfile="$1"
    _basefile="$2"
    ret_val=0

    echo " * * * * *"

    is_update_necessary "$_basefile" "$_dstfile"
    ret=$?
    if [ $ret -eq 0 ]; then
        echo "$_dstfile: Already updated."
        ret_val=0
    elif [ $ret -eq 1 ]; then
        echo "$_dstfile: Update required."
        ret_val=1
    else
        echo "$_dstfile: Needs to be created."
        ret_val=1
    fi

    if [ $ret_val -ne 0 ]; then
        ls -l "$_basefile"
        diff "$_basefile" "$_dstfile"
        cp -fp "$_basefile" "$_dstfile"
        echo "COPIED!"
    fi

    return $ret_val
}

sh_main "$@"
exit $?
