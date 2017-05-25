all:sender receiver
sender:sender.c  packet.h 
	gcc -o sender sender.c AddCongestion.c ccitt16.o
receiver:receiver.c packet.h
	gcc -o receiver receiver.c ccitt16.o 
clean:
	rm -f sender receiver