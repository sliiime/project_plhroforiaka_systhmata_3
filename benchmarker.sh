#!/bin/bash

sum=0
failed=0

for ((i = 0; i < $3 ; i++)) ; 
do 
    ./main < $1 > temp.result
    qresult=$(diff -Z temp.result $2 | tail -n +2)

    duration=$(sed -e 's/.*: \(.*\) μ.*/\1/' <<<$qresult)

    re='^[0-9]+$'
    if ! [[ $duration =~ $re ]] ; then
        echo "error: $duration not a number" >&2; failed=$(($failed + 1))
    fi

    echo $duration
    
    sum=$(($sum+$duration))

done

average=$(($sum/$3))

echo "Successful runs : $(($3 - $failed))/$3" 
echo "Average Query Calculation time : $average μ/s"