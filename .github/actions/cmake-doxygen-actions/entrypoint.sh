#!/bin/sh
cd $2
cd $1
mkdir build
cd build 
cmake .. -DBUILD_DOC=ON
make doc
