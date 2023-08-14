CC:=gcc
CFLAGS:=-I.. -O1 -s -Wall -std=c99

ifeq ($(OS),Windows_NT)
	EXEC:=hoxml-test.exe
else
	EXEC:=hoxml-test.bin
endif

.PHONY: clean all

all: hoxml-test.c
	$(CC) $(CFLAGS) hoxml-test.c -o $(EXEC)

clean:
	rm -f $(EXEC)