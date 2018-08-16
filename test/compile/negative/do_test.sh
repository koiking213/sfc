#!/bin/bash

OK=0
NG=0
for file in *.f90
do
    echo -n "$file: "
    ../../../sfc $file
    if (($? == 1 )); then
	    # compilation error
	    ((OK++))
	    echo OK
    else
	    # compilation is succeeded or aborted
	    ((NG++))
	    echo NG
    fi
    
done


((NG > 0)) && exit 1
exit 0
