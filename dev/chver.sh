#!/bin/bash

if [ "$1" == "" ]; then
	echo "This script requires a command line argument <version number>"
fi

if [ -e "README" ]; then
	sed -i "s/AC_INIT(\([^,]*\), \[\([a-zA-Z0-9.~\-]*\)*\], \(.*$\)/AC_INIT(\1, [$1], \3/g" configure.ac
	autoreconf
else
	echo "This script should be run from the trunk root directory."
fi


