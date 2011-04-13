# Chris Marshall
#
# 3/2/2011

CC=g++
CFLAGS=-c -Wall -I/home/chrismar035/ogre/raknet/include/raknet
LFLAGS=-L/home/chrismar035/ogre/raknet/Lib/DLL -lRakNetDynamic 

all: server client

server: server.o
	$(CC) $(LFLAGS) server.o -o server

server.o: server.cpp
	$(CC) $(CFLAGS) server.cpp

client: client.o
	$(CC) $(LFLAGS) client.o -o client

client.o: client.cpp
	$(CC) $(CFLAGS) client.cpp

clean:
	rm -rf *o server client *gch
