# The compiler to use. "g++" works for most *nix systems and Windows with MinGW
CC := g++
# Compilation flags, intended for gcc in this case. Among these, -I.. and -ansi are important. The former adds the
# folder one level up as an include directory and the latter indicates the ANSI C++ standard.
CFLAGS := -I.. -Wall -ansi

# If building on Windows (MinGW)
ifeq ($(OS),Windows_NT)
	# Use this .exe file name for the output executable
	EXEC := hoxml-example.exe
# If not building on Windows (Linux, macOS, *BSD, BeOS/Haiku, etc.)
else
	# Use this .bin file name for the output executable
    # (Extensions aren't typical on other systems but this is convenient for identifying binaries in our .gitignore)
	EXEC := hoxml-example.bin
endif

# Tell make that these targets are not files
.PHONY: release debug remake clean

# Target for building a release executable that excludes debug symbols and uses a high optimization level
release: hoxml-example.c
	$(CC) $(CFLAGS) -O3 -s hoxml-example.c -o $(EXEC)

# Target for building a debug executable that includes debug symbols
debug: hoxml-example.c
	$(CC) $(CFLAGS) -g hoxml-example.c -o $(EXEC)

remake: clean release

# Target for removing files built by this Makefile - just the executable whose name is in the EXEC variable
clean:
	rm -f $(EXEC)