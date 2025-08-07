CC := gcc
CFLAGS := -Wall -g
INCLUDE := -I../inc/ -I./
CFLAGS += $(INCLUDE)
SRCS := img_builder.c ../common/crypto.c
OBJS := $(SRCS:.c=.o)
TARGET := img_gen

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
