#!/bin/bash

sed -i "s/AC_INIT(\([^,]*\), \[\([a-zA-Z0-9.-~]*\)*\], \(.*$\)/AC_INIT(\1, [$1], \3/g" configure.ac
autoreconf
