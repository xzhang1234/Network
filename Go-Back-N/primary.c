#include <stdio.h> 
#include <string.h>    
#include <sys/socket.h>    
#include <stdlib.h>
#include "crc16.h"
#include "packet.h"
#include "introduceerror.h"

/*
 * Helper function that will build a packet (defined in packet.h)
 * with the given type, sequence number, and data.
 */
packet_t build_packet(uint8_t type, uint8_t sn, uint16_t data);

void primary(int sockfd, double ber) {

	int N = 3;                                        // Window size
	char *msg = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";         // Character string to send
	int num_frames = strlen(msg)/2 + strlen(msg)%2;   // Total number of frames to be sent
	packet_t send_buffer[num_frames];                 // A buffer to hold all the the frames to send
	int win_low, win_high, s_recent;                  // Variables to keep track of current window parameters
	int send_counts[num_frames];                      // Array to keep track of how many times a frame has been transmitted
	
	// Populate the send buffer with frames to send
	int i;
	for (i=0; i<strlen(msg); i+=2) {
		if ( (i+1) < strlen(msg))
			send_buffer[i/2] =  build_packet(DATA, i/2, (uint16_t)((msg[i]<<8) | msg[i+1]));
		else 
			send_buffer[i/2] =  build_packet(DATA, i/2, (uint16_t)((msg[i]<<8) | 0x00));
	}

	// Initialize the send counts array to zero
	for (i=0; i<num_frames; ++i)
		send_counts[i] = 0;
	
	// Initialize our window
	win_low = 0;
	win_high = N-1;
	if (win_high >= num_frames) win_high = num_frames - 1;
	s_recent = 0;

	// Begin transmitting frames 
	while (win_low < num_frames)
	{
		// Send all frames in the window
		while (s_recent <= win_high) 
		{
			// Grab the next packet to send
			packet_t packet = send_buffer[s_recent];
			// Apply BER to the packet
			IntroduceError((char*)&packet, ber);
			printf("--> Sending 0x%02x%02x%02x%02x%02x%02x SN: %3d Data: %c%c ..... ", packet.type, packet.sn, packet.data[0], packet.data[1], packet.crc[0], packet.crc[1], packet.sn, packet.data[0], packet.data[1]);

			// Increase the send count for this packet
			++send_counts[s_recent];

			// Send the packet to the receiver
			if( send(sockfd , &packet, sizeof(packet_t), 0) < 0) {
				printf("failed.\n");
			}
			else {
				printf("success.\n");
				++s_recent;
			}
		}
		printf("\n");

		// Get responses from receiver
		packet_t packet;
		if (recv(sockfd , &packet , sizeof(packet_t) , 0) > 0) {
			// Process the receiver's response (we assume ACK/NAK packets are not corrupted)
			if (packet.type == ACK) {
				// Print which packet was positively acknowledged
				printf("Received ACK%u\n", packet.sn);
				// Check that the SN is within range
				if (packet.sn >= (win_low + 1) && packet.sn <= (win_high +1)) {
					// Adjust send window
					win_low = packet.sn;
					win_high = win_low + (N-1);
					if (win_high >= num_frames) win_high = num_frames - 1;
				}
				
			}
			else if (packet.type == NAK) {
				// Print that we received a NAK
				printf("Received NAK%u\n", packet.sn);
				// Check that the SN is within range
				if (packet.sn >= win_low && packet.sn <= win_high) {
					// Adjust send window
					win_low = packet.sn;
					s_recent = win_low;
					win_high = win_low + (N-1);
					if (win_high >= num_frames) win_high = num_frames - 1;
					// Indicate which frames will be retransmitted
					if (win_low == win_high) printf("Retransmitting frame %u\n", win_low);
					else printf("Retransmitting frames %u - %u\n", win_low, win_high);
				}
			}
		}
		
		fflush(stdout);	
	}

	// Print out number of attempts per packet
	printf("\n");
	int sum = 0;
	for (i=0; i<num_frames; ++i) {
		sum += send_counts[i];
		printf("Frame: %3d | Count: %3d\n", i, send_counts[i]);
	}
	printf("Average: %lf\n", (1.0*sum)/num_frames);
}

/*
 * Helper function that will build a packet (defined in packet.h)
 * with the given type, sequence number, and data.
 */
packet_t build_packet(uint8_t type, uint8_t sn, uint16_t data)
{
	packet_t packet;

	packet.type = type;
	packet.sn = sn;
	packet.data[0] = (uint8_t)(data >> 8);
	packet.data[1] = (uint8_t)(data & 0xFF);
	
	uint8_t* packet_ptr = (uint8_t*)(&packet);
	uint16_t crc = calc_crc(packet_ptr, 4, CRC_GENERATE);
	packet.crc[0] = (uint8_t)(crc >> 8);
	packet.crc[1] = (uint8_t)(crc & 0xFF);

	return packet;
}
