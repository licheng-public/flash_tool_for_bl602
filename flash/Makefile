CC := gcc
CFLAGS := -Wall -g
INCLUDE := -I../inc/ -I./
CFLAGS += $(INCLUDE)
SRCS := comm.c uart.c flash.c
OBJS := $(SRCS:.c=.o)
TARGET := flash

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
