#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "packet.h"
#include "ccitt16.h"

int main(int argc, char *argv[]) {
	if(argc != 3) {
		printf("Please pass the IP address and port number, e.g., ./receiver 127.0.0.1 50404");
		exit(-1);
	}
	char *IP_ADDRESS = argv[1]; 
	int  PORT = atoi(argv[2]);
	/* Create socket for incoming connections */
	int recv_sock;
	if((recv_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket() failed.\n") ;
		exit(-1);
	}
	/* Construct local address structure */
	struct sockaddr_in recv_addr;
	recv_addr.sin_family = PF_INET;
	recv_addr.sin_port   = htons(PORT);
	recv_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
	/* Bind to the local address */
	if(bind(recv_sock, (struct sockaddr *) &recv_addr, sizeof(recv_addr)) < 0) {
		perror("bind() failed.\n");
		exit(-1);
	}
	/* Listen for connections */
	if(listen(recv_sock, 10) < 0) {
		perror("listen() failed.\n");
		exit(-1);
	}
	/* Wait for the sender to connect */
	struct sockaddr_in sender_addr;
	int saddr_len = sizeof(sender_addr);
	int consock; 
	if((consock = accept(recv_sock, (struct sockaddr *) &sender_addr, (socklen_t *) &saddr_len)) < 0) {
		perror("accept() failed.\n");
		exit(-1);
	}
	uint16_t sn = 257;
	while(1) {
		packet_t packet;
		read(consock, &packet, sizeof(packet_t)); 
		if (!calculate_CCITT16((unsigned char *) &packet, 6, CHECK_CRC)) {
			if(packet.sn == sn) {
				printf("Receive data: %c%c\n", packet.data[0], packet.data[1]);
				fflush(stdout);
				sn += 2;
			}
			sleep(1);
			write(consock, &sn, sizeof(sn));
		} else {
			printf("Discard data\n");
		}
	}
	close(consock);	
	close(recv_sock);
	exit(0);
}