#!/bin/bash

OK=0
NG=0
for file in *.f90
do
    echo -n "$file: "
    ../../../sfc -c $file
    if (($? == 0 )); then
	    # compilation is succeeded
	    ((OK++))
	    echo OK
    else
	    # compilation failed or aborted
	    ((NG++))
	    echo NG
    fi
done


((NG > 0)) && exit 1
exit 0
