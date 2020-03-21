#!/bin/sh

../balance/balance/main >> ../log/balance.log &

echo "balance start..."

for (( i=1; i<=10; i=i+1 ))
do
	../server/main >> ../log/server.log &
	echo "server $i start..."
done


