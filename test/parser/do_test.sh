#!/bin/bash

OK=0
NG=0
for file in test/parser/*.f90
do
    echo -n "$file: "
    ./sfc $file 
    if (($? == 0 )); then
	# compilation is succeeded
	if [[ $file =~ err[0-9]*\.(f|f90)$ ]]; then
	    ((NG++))
	    echo NG1
	else
	    ((OK++))
	    echo OK
	fi
    else
	# compilation is failed
	if [[ $file =~ err[0-9]*\.(f|f90)$ ]]; then
	    ((OK++))
	    echo OK
	else
	    ((NG++))
	    echo NG2
	fi
    fi
done


((NG > 0)) && exit 0
exit 1
