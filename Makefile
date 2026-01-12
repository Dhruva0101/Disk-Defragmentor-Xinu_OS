defrag: defrag.c blocks.c defrag.h blocks.h
	gcc -std=c11 -O0 -g -o defrag defrag.c blocks.c

clean:
	rm -f defrag disk_defrag
	