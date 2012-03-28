#!/bin/sh

echo $1 $2 $3 $4 $5 $6
latex $1 "$2" "$3"
cd "$4"
dvips -q "$5"
ps2pdf "$6"
