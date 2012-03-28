#!/bin/sh

latex $1 "$2" "$3"
cd "$4"
dvipdf -q "$5"
