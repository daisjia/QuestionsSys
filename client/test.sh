#!/bin/sh

ulimit -c unlimited
g++ -std=c++11 -o test test.cpp IM.User.Msg.pb.cc -lpthread -lprotobuf && rm -rf core.*

./test
