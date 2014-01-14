CFLAGS  +=  -Os -Wall
PROG    =   dstat
PREFIX  ?=  /usr
VER     =   1.0
LIBS	  =	-lX11

${PROG}: ${PROG}.c
	@${CC} ${CFLAGS} ${LIBS} -o ${PROG} ${PROG}.c
	@strip ${PROG}

clean:
	@rm -f ${PROG}

install: ${PROG}
	@install -Dm755 ${PROG} ${DESTDIR}${PREFIX}/bin/${PROG}
