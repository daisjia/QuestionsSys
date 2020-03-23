#!/bin/sh

num=$(ps -ef | grep redis-server | wc -l)
if [ ${num} -eq 1 ];
then
	../myredis/redis-server ../myredis/redis.conf
fi
echo "" > a

ps -ef | grep ../balance/balance/main >> a
ps -ef | grep ../server/main >> a

while read line
do
	b=$(cat $line | awk '{print $2}')
	kill -9 $b &> /dev/null
done < a

rm -rf a

sleep 1
../balance/balance/main >> ../log/balance.log &

echo "balance start..."

for (( i=1; i<=10; i=i+1 ))
do
	../server/main >> ../log/server.log &
	echo "server $i start..."
done


