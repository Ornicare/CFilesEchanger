CC=gcc
CFLAGS=-lssl -lcrypto -lpthread
LDFLAGS=
EXEC=serveur.exe

all: $(EXEC)

$(EXEC): md5manager.o serveur_functions_manager.o serveur.o crc.o
	$(CC) -o $@ $^ $(CFLAGS)

md5manager.o: md5manager.c
	$(CC) -o $@ -c $< $(CFLAGS)

serveur_functions_manager.o: serveur_functions_manager.c
	$(CC) -o $@ -c $< $(CFLAGS)

serveur.o: serveur.c md5manager.h serveur_functions_manager.h
	$(CC) -o $@ -c $< $(CFLAGS)

crc.o: crc.c
	$(CC) -o crc.o -c crc.c $(CFLAGS)

.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
