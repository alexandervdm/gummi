#!/bin/sh

latex $1 "$2" "$3"
cd "$4"
dvips -q "$5"
ps2pdf "$6"
