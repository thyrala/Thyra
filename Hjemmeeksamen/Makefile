CFLAGS = -g -std=gnu11 -Wall -Wextra
BIN = client server

all: $(BIN)

client: client-NewFSP.o rdp.o send_packet.o
	gcc $(CFLAGS) -o client client-NewFSP.o rdp.o send_packet.o

server: server-NewFSP.o rdp.o send_packet.o
	gcc $(CFLAGS) -o server server-NewFSP.o rdp.o send_packet.o

client.o: client-NewFSP.c rdp.h send_packet.h
	gcc -c $(CFLAGS) -o client client-NewFSP.c

server.o: server-NewFSP.c rdp.h send_packet.h
	gcc -c $(CFLAGS) -o server server-NewFSP.c

rdp.o: rdp.c rdp.h send_packet.h
	gcc -c $(CFLAGS) rdp.c

send_packet.o: send_packet.c send_packet.h
	gcc -c $(CFLAGS) send_packet.c

valgrindC:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./client "127.0.0.1" 2222 0.1

valgrindS:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./server 2222 2 0.1

runC:
	./client "127.0.0.1" 2222 0.1

runS:
	./server 2222 2 0.1

clean:
	rm -f $(BIN) kernel-file-* *.o