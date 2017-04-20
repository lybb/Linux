#!/bin/bash

i=0
str=""
rate=('-' '\' '|' '/')
while [ $i -le 100 ]
do
	printf "[%-100s][%d %%][%c]\r" "$str" "$i" "${rate[i%4]}"
	sleep 0.1
	let i++
	str+="="
done
printf "\n"
