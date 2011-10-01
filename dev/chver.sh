#!/bin/bash

if [ "$1" == "" ]; then
    echo "This script requires a command line argument <version number>"
fi

absolute_dir=$(cd "$( dirname "$0" )" && pwd)
cd $absolute_dir/..

sed -i "s/AC_INIT(\([^,]*\), \[\([a-zA-Z0-9.~\-]*\)*\], \(.*$\)/AC_INIT(\1, [$1], \3/g" configure.ac
autoreconf


