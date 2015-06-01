TARGET=pngnormalize
CC=gcc
LDFLAGS= -lplist -lz -lzip
SOURCES += $(wildcard ./*.c)
OBJS = $(SOURCES:%.c=%.o)

all:$(TARGET)
$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

.PHONY:clean
clean:
	-$(RM) $(TARGET) $(OBJS)

