mkdir 1.b.files.out;for file in `ls 1.b.files`;do sort -n 1.b.files/$file -o "1.b.files.out/$file";done;cat 1.b.files.out/*.txt|sort -n|uniq -c|awk '{print $2"\t"$1}'>1.b.out.txt
