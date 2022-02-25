#!/bin/make


PREFIX=
CFLAGS=-Wall -g
LDFLAGS=${CFLAGS}

CC=${PREFIX}gcc
OBJCOPY=${PREFIX}objcopy

TARGET=zhiyun_ff
SRC=crc-ccitt.c zhiyun_ff.c
OBJ=${SRC:.c=.o}

.PHONY: all
all: ${TARGET}


${TARGET}: ${OBJ}
	${CC} ${LDFLAGS} ${OBJ} -o $@

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@


.PHONY: clean
clean:
	rm -f ${OBJ} ${TARGET}
