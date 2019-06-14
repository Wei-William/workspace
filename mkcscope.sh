#!/bin/bash 
echo "find all .c .h .cpp name to cscope.file"
find `pwd` -name "*.[ch]" -o -name "*.cpp" > cscope.files

echo "make cscope datebase"
cscope -Rbkq -i cscope.files

echo "define CSCOPE_DB"
export CSCOPE_DB=$PWD
