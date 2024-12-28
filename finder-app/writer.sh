#!/bin/bash

writefile=$1
writestr=$2

if [ $# -ne 2 ]
then
	echo "Number of arguments is not two!"
	exit 1
fi

if [ ! -d $writefile ]
then
	mkdir -p $(dirname $writefile)
fi

touch $writefile
echo $writestr > $writefile

if [ $? -ne 0 ]
then
	echo "Could not write to file."
	exit 1
fi

