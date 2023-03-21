CC = gcc
CFLAGS = -fPIC -Wall -Wextra -O2 -g
LDFLAGS = -shared
RM = rm -f
TARGET_LIB = ping.so

SRCS = ping.c
OBJS = $(SRCS:.c=.o)

.PHONY: all
all: ${TARGET_LIB}

$(TARGET_LIB): libtype.so $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c=.d): libtype.so %.d:%.c 
	$(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c=.d)

.PHONY: clean
clean:
	-${RM} ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d)

#-----------------------------------------------------------------------------------------------------
#CC:=gcc
#CFLAGS=
#LDFLAGS=
#LIBS+=
#
#release: clean
#	$(CC) $(CFLAGS) -Wall -Werror=format-security -s ping.c -o ping_n00rd1 $(LDFLAGS) $(LIBS)
#
#clean:
#	rm -rf at *~
