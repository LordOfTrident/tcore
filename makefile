SPECIAL = src/utils.c src/error.c
SRC  = ${filter-out ${SPECIAL},${wildcard src/*.c}}
DEPS = ${wildcard src/*.h}
BIN  = ${subst src/,bin/,${basename ${SRC}}}

VERSION_MAJOR = 2
VERSION_MINOR = 6
VERSION_PATCH = 9

CC = cc
CVER = c99
CFLAGS = \
	-O3\
	-std=${CVER}\
	-Wall\
	-pedantic\
	-Wno-deprecated-declarations\
	-DVERSION_MAJOR=${VERSION_MAJOR}\
	-DVERSION_MINOR=${VERSION_MINOR}\
	-DVERSION_PATCH=${VERSION_PATCH}\
	-g

compile: ${BIN}

${BIN}: bin/% : src/%.c ${DEPS} ${SPECIAL}
	mkdir -p bin
	${CC} ${CFLAGS} -o $@ $< ${SPECIAL}

clean:
	rm ${BIN}

all:
	@echo compile
	@echo clean
