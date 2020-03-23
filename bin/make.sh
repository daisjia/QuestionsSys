#!/bin/sh

b=$2

a=$(pwd)
cd ../server
if [ ${b}=="cc" ];
then
	make cc
fi

make

cd ../client
if [ ${b}=="cc" ]
then
	make cc
fi
make

cd ../balance/balance 
if [ ${b}=="cc" ]
then
	make cc
fi

make

cd $a
#clear

