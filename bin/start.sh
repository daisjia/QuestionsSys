#!/bin/sh

#num=$(ps -ef | grep redis-server | wc -l)
#if [ ${num} -eq 1 ];
#then
#	../myredis/redis-server ../myredis/redis.conf
#fi


../balance/balance/main >> ../log/balance.log &

echo "balance start..."

for (( i=1; i<=10; i=i+1 ))
do
	../server/main >> ../log/server.log &
	echo "server $i start..."
done


