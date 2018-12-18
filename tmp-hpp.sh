#!/bin/bash
# Helper script to allow clang-tidy to run on a single header file, ensuring the header actually includes everything it needs
echo '#include "./'$(basename $2)'"' > $2.cc
$1 $2.cc "${@:3}"
RET=$?
rm $2.cc
exit $RET
