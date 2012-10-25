CC=gcc

CFLAGS=-Wall

TARGET= spamhole

all : $(TARGET)

clean:
	rm -f $(TARGET) core *.o *~ 

spamhole: main.o spamhole.o
	@echo "Compiling spamhole binary..."
	$(CC) $(CFLAGS) -o spamhole main.o spamhole.o 
	@echo

main.o: main.c spamhole.h
	@echo "Compiling main object..."
	$(CC) $(CFLAGS) -c main.c

spamhole.o: spamhole.c spamhole.h
	@echo "Compiling spamhole object..."
	$(CC) $(CFLAGS) -c spamhole.c

