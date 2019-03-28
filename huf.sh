#!/bin/bash

last="huf" #lz lz+ 3bin 4bin
method="Huffman" #lz77 lz772 三位预编码 四位预编码

filename="huf.txt"
if [ -s $filename ]
then
	rm $filename
fi

awk 'BEGIN{printf "%s\t%s\t%s\t%s\n",("原大小"),("现大小"),("压缩率"),("压缩时间")}' >$filename

for i in `ls ./Files`
#for i in `ls ./Out/4bin`
do   
	index=`expr index "$i" .`
	output="./Out/$last/${i:0:$index}$last"

	input="./Files/$i"
	#input="./Out/4bin/$i"
	(/usr/bin/time -f "%e" ./$method/encode $input $output) 2> temp
	time=`cat temp`
	rm temp
	inputsize=`stat -c %s $input`
	outputsize=`stat -c %s $output`

	awk 'BEGIN{printf "%d\t%d\t%.4f\t%.2f\n",('$inputsize'), ('$outputsize'),\
		('$inputsize'-'$outputsize')/'$inputsize', ('$time')}' >>$filename
	#echo $?
done