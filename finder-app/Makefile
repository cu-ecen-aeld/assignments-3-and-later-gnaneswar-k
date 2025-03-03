CC = $(CROSS_COMPILE)gcc
TARGET = writer
SRC := $(TARGET).c
OBJS := $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f *.o
	rm -f $(TARGET)

