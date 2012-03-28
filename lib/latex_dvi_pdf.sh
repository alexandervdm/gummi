#!/bin/sh

echo $1, $2, $3, $4, $5
latex $1 "$2" "$3"
cd "$4"
dvipdf -q "$5"
