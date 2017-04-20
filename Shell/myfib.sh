#!/bin/bash

#i=0
#sum=0
#while [ $i -le 100 ]
#do
#	let sum+=$i
#	let i++
#done
#echo "sum=$sum"

#for i in {0..100}
#do
#	let sum+=$i;
#done
#echo "sum=$sum"


##fib1
#one=1
#two=1
#three=1
#i=2
#read top
#while [ $i -lt $top ]
#do
#	let three=$one+$two
#	one=$two
#	two=$three
#	let i++
#done
#echo $three

#while read line
#do
#	sub_one=$(echo $line | cut -c 1-3)
#	sub_two=$(echo $line | cut -c 4-6 | tr '[a-z]' '[A-Z]')
#	sub_three=$(echo $line | cut -c 7-9)
#	line=${sub_three}${sub_two}${sub_one}
#	echo $line < file
#done < file
