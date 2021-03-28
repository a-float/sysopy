#!/bin/bash

PAIRS=($1 $3 $5)
SIZES=($2 $4 $6)

	small=data/small.txt
	med=data/medium.txt
	big=data/big.txt

for pair in "${PAIRS[@]}"
do
	echo "no of pairs: ${pair}, no of lines: 2*$2"
	{ time ./main.out create_table $pair merge_files $pair $small:$small; } 2>&1 >/dev/null
	echo
	echo "no of pairs: ${pair}, no of lines: 2*$4"
	{ time ./main.out create_table ${pair} merge_files ${pair} $med:$med; } 2>&1 >/dev/null
	echo
	echo "no of pairs: $pair, no of lines: 2*$6"
	{ time ./main.out create_table $pair merge_files $pair $big:$big; } 2>&1 >/dev/null
	echo 
done