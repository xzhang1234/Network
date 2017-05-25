all:sender receiver
sender:primary.o
	gcc -o sender sender.o primary.o crc16.c introduceerror.o 
primary.o:primary.c
	gcc -c primary.c 
primary.c:introduceerror.o
	gcc -c introduceerror.c
receiver:secondary.o
	gcc -o receiver receiver.o secondary.o crc16.c
secondary.o:secondary.c
	gcc -c secondary.c
clean:
	rm -f sender receiver primary.o secondary.o introduceerror.o
