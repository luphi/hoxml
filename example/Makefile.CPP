# The compiler to use. "g++" works for most *nix systems and Windows with MinGW
CC:=g++
# Compilation flags, intended for gcc in this case. Among these, -I.. and -std=c++98 are important. The former adds the
# folder one level up as an an include directory and the latter indicates the C++98 standard. hoxml is written in C99.
CFLAGS:=-I.. -O1 -s -Wall -std=c++98

# If building on Windows (MinGW)
ifeq ($(OS),Windows_NT)
	# Use this .exe file name for the output executable
	EXEC:=hoxml-example.exe
# If not building on Windows (Linux, macOS, *BSD, BeOS/Haiku, etc.)
else
	# Use this .bin file name for the output executable
    # (Extensions aren't typical on other systems but this is convenient for identifying binaries in our .gitignore)
	EXEC:=hoxml-example.bin
endif

# Tell make that the "clean" and "all" targets are not files
.PHONY: clean all

# Target for building everything (all) - just hoxml-example.c
all: hoxml-example.c
	$(CC) $(CFLAGS) hoxml-example.c -o $(EXEC)

# Target for removing files built by this Makefile - just the executable whose name is in the EXEC variable
clean:
	rm -f $(EXEC)
