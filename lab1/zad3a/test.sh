	#!/bin/bash

	PAIRS=($1 $3 $5)
	SIZES=($2 $4 $6)
 
 	small=data/small.txt
 	med=data/medium.txt
 	big=data/big.txt

	for pair in "${PAIRS[@]}"
	do
		echo "no of pairs: ${pair}, no of lines: 2*$2"
		echo
		./main.out create_table $pair merge_files $pair $small:$small save_to_file add_remove $((pair / 2)) $small remove_table
		echo
		echo "no of pairs: ${pair}, no of lines: 2*$4"
		echo
		./main.out create_table ${pair} merge_files ${pair}  $med:$med save_to_file add_remove $((pair / 2)) $med remove_table
		echo
		echo "no of pairs: $pair, no of lines: 2*$6"
		echo
		./main.out create_table $pair merge_files $pair  $big:$big save_to_file add_remove $((pair / 2)) $big remove_table
		echo
	done