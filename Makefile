C_SRC = $(wildcard *.c)
C_OBJ = $(patsubst %c, %o, $(C_SRC))

CFLAGS = -Wall -O2 -g -lpthread

SERVER = fserver
CLIENT = fclient
DAEMON = daemon
SHAREOBJS = socketfd.o

.PHONY:all clean cleanobjs testfile
# 这句不写规则的语句可以自动把相应的a.c b.c编译成a b，神奇~
all:clean cleanobjs ${SERVER} ${CLIENT} ${DAEMON} cleanobjs testfile

${SERVER}:${SERVER}.o ${SHAREOBJS}
	gcc -o $@ $^ $(CFLAGS)

${CLIENT}:${CLIENT}.o ${SHAREOBJS}
	gcc -o $@ $^ $(CFLAGS)

${DAEMON}:${DAEMON}.o ${SHAREOBJS}
	gcc -o $@ $^ $(CFLAGS)

%.o:%.c
	$(CC) -c $^ $(CFLAGS)

clean:  
	rm -f ${SERVER} ${CLIENT} ${DAEMON}

cleanobjs:  
	rm -f *.o

testfile:
	@echo "test message from 1.txt xxxxxxxxxxxxxxxxxxx" > 1.txt