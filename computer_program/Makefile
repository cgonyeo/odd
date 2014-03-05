CFLAGS := -Wall -std=gnu99 -ggdb 
LDFLAGS := -lm -lpthread -lwiringPi -lfftw3 -ljansson -lasound

SRCS := $(wildcard *.c) $(wildcard animations/*.c)
HDRS := $(wildcard *.h)
OBJS := $(patsubst %.c,%.o,$(SRCS))

odd: ${OBJS} ${HDRS}
	gcc ${LDFLAGS} ${OBJS} -o $@ 

%.o: %.c ${HDRS}
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm odd *.o animations/*.o
