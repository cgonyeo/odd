CFLAGS := -Wall -std=gnu99 -ggdb 
LDFLAGS := -lm -lpthread -ljack -lwiringPi -lfftw3 -ljansson

SRCS := $(wildcard *.c)
HDRS := $(wildcard *.h)
OBJS := $(patsubst %.c,%.o,$(SRCS))

odd: ${OBJS} ${HDRS}
	gcc ${LDFLAGS} ${OBJS} -o $@ 

%.o: %.c ${HDRS}
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm odd *.o
