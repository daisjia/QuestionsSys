#/bin/sh

echo "" > a

ps -ef | grep ../balance/balance/main >> a
ps -ef | grep ../server/main >> a

while read line
do
	b=$(cat $line | awk '{print $2}')
	kill -9 $b &> /dev/null
done < a

rm -rf a
ps -ef | grep main
