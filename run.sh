#!/bin/bash
for i in `seq 1 30`;
do
	nice -n 19 ./package_delivery
	gnuplot gnugraph
	mv route.png route-Archer-$i.png
done

