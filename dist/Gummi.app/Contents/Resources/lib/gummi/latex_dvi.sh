#!/bin/sh
# Args: type, flags, outdir, workfile, tempdir, dviname, [psname]

exit_on_error () {
    status=$?
    if [ "$status" -ne "0" ]
    then
        exit $status
    fi
}

latex $2 "$3" "$4"
exit_on_error

olddir=$PWD
cd "$5"
case "$1" in
    "pdf" )
    TEXINPUTS=$TEXINPUTS:$olddir dvipdf -q "$6"
    ;;
    "ps" )
    TEXINPUTS=$TEXINPUTS:$olddir dvips -q "$6"
    exit_on_error
    ps2pdf "$7"
    ;;
    * )
    echo "Can't compile dvi to $1."
    exit 1
    ;;
esac
