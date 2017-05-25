#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "packet.h"
#include "ccitt16.h"
#include "AddCongestion.h"
#include <errno.h>

const int timeout = 3;
int readFile(char *filename, char *buffer, int len);
int TCPConnectionOpen(const char *remote_ip, const int remote_port);
int TCPConnectionClose(const int sock);
int TCPCongestionControl(const int sock, char *data, const int len, const double ber);
int main(int argc, char *argv[]) {
	if(argc != 4) {
		printf("Please pass the IP address, port number and BER value, e.g., ./sender 127.0.0.1 50404 0.001");
		exit(-1);
	}
	const char   *remote_ip  = argv[1]; 
	const int    remote_port = atoi(argv[2]);
	const double ber         = atof(argv[3]);
	int  sender_sock = TCPConnectionOpen(remote_ip, remote_port);
	char data[2000];
	int  len = readFile("input.txt", data, sizeof(data));
	TCPCongestionControl(sender_sock, data, len, ber);
	TCPConnectionClose(sender_sock);
	exit(1);
}

int readFile(char *filename, char *buffer, int len) {
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Could not read file %s.\n", filename);
		return -1;
	}
	fgets(buffer, len, fp);
	fclose(fp);
	return strlen(buffer);
}
int TCPConnectionOpen(const char *remote_ip, const int remote_port) {
	/* Create socket to connect remote hosts */
	int sock;
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket() error:\n");
		exit(-1);
	}
	/* Construct the address structure of the remote host */
	struct sockaddr_in remote_addr;
	remote_addr.sin_family = PF_INET;
	remote_addr.sin_port = htons(remote_port);
	remote_addr.sin_addr.s_addr = inet_addr(remote_ip);
	/* Connect to the remote host */
	if(connect(sock, (struct sockaddr *) &remote_addr, sizeof(remote_addr)) < 0) {
		perror("connect() failed.\n");
		exit(-1);
	} 
	struct timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
	return(sock);
}
int TCPConnectionClose(const int sock) {
	close(sock);
	return 1;
}
packet_t packet(uint16_t sn, uint8_t data1, uint8_t data2) {
	packet_t packet;

	packet.sn = sn;
	packet.data[0] = data1;
	packet.data[1] = data2;
	
	uint16_t crc = calculate_CCITT16((uint8_t *) &packet, 4, GENERATE_CRC);
	packet.crc[0] = (uint8_t)(crc >> 8);
	packet.crc[1] = (uint8_t)(crc & 0xFF);
	packet.n = '\0';
	return packet;
}
int TCPCongestionControl (const int sock, char *data, const int len, const double ber) {
	packet_t pck;
	int      idx         = 0;
	int      mss         = 2;
	float    cwnd        = 1.0;
	float    ssthresh    = 16.0;
	int      out_high, out_low = 257;
	int      timeout     = 0;
	uint16_t sn          = 257;
	uint16_t ack         = 0;
	uint16_t last_ack    = 0;
	int      num_acks    = 0;
	int      num_dup_acks = 0;	
	
	while (idx < len) {
		timeout = 0;
		int i;
		for	(i = 0; i < cwnd; i++) {
			if (idx >= len) break;
			pck = packet(sn, data[idx], data[idx+1]);
			AddCongestion((char *) &pck, ber);	
			if(write(sock, &pck, sizeof(pck)) < 0) {
				printf("write() failed.\n");
			} else {
				printf("Send packet: sn:%d, data:%c%c, crc:0x%02x%02x\n", pck.sn, pck.data[0], pck.data[1], pck.crc[0], pck.crc[1]);
			}
			out_high = sn;
			sn  += mss;
			idx += mss;
		}
		printf("out_low, out_high: [%d, %d]\n", out_low, out_high);
		//for (int i = 0; i < outstanding; ++i) {
		while (out_low <= out_high) {
			int rbytes;
			if((rbytes = read(sock, &ack, sizeof(ack))) < 0) {
			   int errsv = errno;
			   printf("read() failed, error code: %d\n", errsv);
			   timeout = 1;
			   break;
			}
			printf("Receive ACK: %d\n", ack);
			if (ack == last_ack) {
				num_dup_acks++;	
				printf("Duplicate ACK, num_dup_acks %d\n", num_dup_acks);
			} else {
				num_dup_acks = 0;
				if (cwnd < ssthresh) {
					cwnd++;
					printf("Slow start: cwnd: %lf\n", cwnd);						//slow start
				} else {
					cwnd += (double)1/(int)(cwnd); 
					printf("Congestion control: cwnd: %lf\n", cwnd);				//congestion control
				}
				out_low = ack;
			}
			last_ack = ack;
		}
		if (timeout == 1 || num_dup_acks >= 3) { 						//fast retransmission
			ssthresh = (cwnd / 2) > 1 ? (cwnd / 2) : 1;
			cwnd = 1;
			idx = out_low - 257;
			sn = out_low;
			printf("Fast retransmission: ssthresh: %lf, cwnd: %lf\n", ssthresh, cwnd);		
		}
		printf("End of one RTT: cwnd: %lf\n\n", cwnd);
	}
}