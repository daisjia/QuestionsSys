#!/bin/sh

a=$(pwd)
cd ../server
#make cc
make

cd ../client
#make cc
make

cd ../balance/balance 
#make cc
make

cd $a
#clear

