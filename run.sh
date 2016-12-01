#!/bin/bash
for i in `seq 1 30`;
do
	nice -n 19 ./package_delivery $1.csv -pop $2 -g $3
	gnuplot gnugraph
	mv route.png route-$1-$i.png
done

