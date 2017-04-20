#! /bin/bash

var=http://www.mmm.cn/123.html
echo $var | cut -c 1-3

var=http://www.mmm.cn/123.html
echo $var | cut -c 1,3

var=http://www.mmm.cn/123.html
echo $var | cut -c 4


#echo ${var#*:}
#echo ${var##*//}
#echo ${var%cn*}
#echo ${var%%123*}
#echo ${var:0:5}
#echo ${var:8}
#echo ${var:0-7:4}
#echo ${var:0-7}
