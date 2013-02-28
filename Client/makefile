CC=gcc
CFLAGS=-lssl -lcrypto
LDFLAGS=
EXEC=client.exe

all: $(EXEC)

$(EXEC): md5manager.o client_functions_manager.o client.o crc.o
	$(CC) -o $(EXEC) $^ $(CFLAGS)

md5manager.o: md5manager.c
	$(CC) -o md5manager.o -c md5manager.c $(CFLAGS)

client_functions_manager.o: md5manager.c client_functions_manager.c crc.c
	$(CC) -o client_functions_manager.o -c client_functions_manager.c $(CFLAGS)

client.o: client.c md5manager.h client_functions_manager.h
	$(CC) -o client.o -c client.c $(CFLAGS)

crc.o: crc.c
	$(CC) -o crc.o -c crc.c $(CFLAGS)

.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
