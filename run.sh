#!/bin/bash
for i in `seq 1 5`;
do
	nice -n 19 ./package_delivery $1.csv -warehouse "${2}" -pop $3 -g $4 -t $5 -w $6
	gnuplot gnugraph
	mv route.png route_$1_p$3-g$4-t$5-w$6_$i.png
done

