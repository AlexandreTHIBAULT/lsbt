all: lsbt

lsbt: lsbt.c
	gcc lsbt.c -o lsbt -lm

clean:
	rm lsbt