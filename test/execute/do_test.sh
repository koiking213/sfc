#!/bin/bash

OK=0
NG=0
for file in *.f90
do
    echo -n "$file: "
    ../../sfc -L ../../runtime $file 
    if (($? == 0 )); then
	    # compilation is succeeded
        if diff <(./a.out) ${file/.f90/.res}; then
	        ((OK++))
	        echo OK
        else
	        # execution result is wrong
	        ((NG++))
	        echo NG
        fi
    else
	    # compilation failed or aborted
	    ((NG++))
	    echo NG
    fi
done


((NG > 0)) && exit 1
exit 0
