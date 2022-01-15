SPECIAL = src/utils.c src/error.c
SRC  = ${filter-out ${SPECIAL},${wildcard src/*.c}}
DEPS = ${wildcard src/*.h}
BIN  = ${subst src/,bin/,${basename ${SRC}}}

VERSION_MAJOR = 2
VERSION_MINOR = 9
VERSION_PATCH = 6

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
	-DVERSION_PATCH=${VERSION_PATCH}

compile: ./bin ${BIN}

./bin:
	mkdir -p bin

${BIN}: bin/% : src/%.c ${DEPS} ${SPECIAL}
	${CC} ${CFLAGS} -o $@ $< ${SPECIAL}

clean:
	rm ${BIN}

all:
	@echo compile
	@echo clean
