#!/bin/sh

a=$(ps -ef | grep "./main[ ]8000" | awk '{print $2}')

kill -9 $a &> /dev/null
