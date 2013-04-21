
CC     = gcc
TARGET = lcdctl
OBJS   = main.o gpio.o

all:       $(TARGET)

$(TARGET): $(OBJS)
		$(CC) -o $@ $(OBJS)

clean:
		-rm -f $(TARGET) $(OBJS) .nfs* *~ core

.c.o:
		   $(CC) -c $<

main.o: gpio.h
gpio.o: gpio.h

