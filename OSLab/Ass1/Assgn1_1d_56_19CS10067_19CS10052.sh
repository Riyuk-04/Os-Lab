mkdir files_mod;for file in `ls $1`;do sed "s/\s/,/g"<$1$file|nl -s,>"files_mod/$file";done