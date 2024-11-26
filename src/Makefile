all: $(wildcard *.cpp) $(wildcard *.h)
	g++ $(wildcard *.cpp) -o ../bin/debug -g -I ../deps/ -L ../deps/linenoise -l linenoise ../deps/libelfin/dwarf/libdwarf++.a ../deps/libelfin/elf/libelf++.a