
EXE = 

LUA ?= bin/minilua

OPT ?= -O2

PROG_SRCS := main/wasm2wat.c main/wat2wasm.c main/wasm2wasm.c main/miniwasm.c main/raywasm.c main/jit49.c main/runtime/rlruntime.c
PROG_OBJS := $(PROG_SRCS:%.c=%.o)

WEB49_SRCS := src/read_bin.c src/read_wat.c src/write_wat.c src/write_bin.c src/io.c src/tables.c src/interp/interp.c src/opt/tree.c src/opt/tee.c src/ast.c
WEB49_OBJS := $(WEB49_SRCS:%.c=%.o)

DASC_SRCS := src/jit/jit.dasc
DASC_OBJS := $(DASC_TMPS:%.dasc=%.o)

OBJS := $(WEB49_OBJS) $(DASC_OBJS)

default: all

all: bins

bins: bin/wasm2wat$(EXE) bin/wat2wasm$(EXE) bin/wasm2wasm$(EXE) bin/miniwasm$(EXE) bin/jit49$(EXE)

# bin

bin/minilua: dynasm/minilua.c
	@mkdir -p bin
	$(CC) dynasm/minilua.c -o $(@) -lm $(LDFLAGS)

bin/jit49$(EXE): main/jit49.o $(OBJS)
	@mkdir -p bin
	$(CC) $(OPT) main/miniwasm.o $(OBJS) -o $(@) -lm $(LDFLAGS)

bin/miniwasm$(EXE): main/miniwasm.o $(OBJS)
	@mkdir -p bin
	$(CC) $(OPT) main/miniwasm.o $(OBJS) -o $(@) -lm $(LDFLAGS)

bin/raywasm$(EXE): main/raywasm.o main/runtime/rlruntime.c $(OBJS)
	@mkdir -p bin
	$(CC) $(OPT) main/raywasm.o main/runtime/rlruntime.c $(OBJS) -o $(@) -lm -lraylib -pthread -lGL -ldl $(LDFLAGS)

bin/wat2wasm$(EXE): main/wat2wasm.o $(OBJS)
	@mkdir -p bin
	$(CC) $(OPT) main/wat2wasm.o $(OBJS) -o $(@) -lm $(LDFLAGS)

bin/wasm2wat$(EXE): main/wasm2wat.o $(OBJS)
	@mkdir -p bin
	$(CC) $(OPT) main/wasm2wat.o $(OBJS) -o $(@) -lm $(LDFLAGS)

bin/wasm2wasm$(EXE): main/wasm2wasm.o $(OBJS)
	@mkdir -p bin
	$(CC) $(OPT) main/wasm2wasm.o $(OBJS) -o $(@) -lm $(LDFLAGS)

# util

format: .dummy
	find . -name '*.c' | xargs -I FILENAME clang-format -style=file -i FILENAME
	find . -name '*.h' | xargs -I FILENAME clang-format -style=file -i FILENAME
	find . -name '*.inc' | xargs -I FILENAME clang-format -style=file -i FILENAME

clean: .dummy
	find . -name '*.o' | xargs rm
	find . -name '*.tmp.c' | xargs rm
	find bin -type f | xargs rm

# intermediate files

$(DASC_OBJS): dynasm/dynasm.lua $(@:%.o=%.dasc)
	$(LUA) dynasm/dynasm.lua -o $(@) $(@:%.tmp.c=%.dasc)
	$(CC) -c $(OPT) $(@:%.o=%.dasc) -o $(@) $(CFLAGS)

$(PROG_OBJS) $(WEB49_OBJS): $(@:%.o=%.c)
	$(CC) -c $(OPT) $(@:%.o=%.c) -o $(@) $(CFLAGS)

# dummy

.dummy:
