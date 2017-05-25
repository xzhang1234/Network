#include<stdio.h>
#include<sys/socket.h>
#include <string.h> 
#include "packet.h"
#include "crc16.h"

/*
 * Helper function that will build a packet (defined in packet.h)
 * with the given type, sequence number, and data.
 */
packet_t build_packet(uint8_t type, uint8_t sn, uint16_t data);

void secondary(int client_sock) {
		int read_size;
		packet_t packet;
		uint8_t sn = 0;

		// Read a packet from the sender
    while( (read_size = recv(client_sock , &packet , sizeof(packet_t) , 0)) > 0 )
    {
			// Check if there was error
			if (calc_crc((uint8_t*)(&packet), sizeof(packet_t), CRC_CHECK) == CRC_CHECK_SUCCESS) {
				// Check the sequence number
				if (packet.sn == sn) {
					printf("SN: %3u Data: %c%c\n", packet.sn, packet.data[0], packet.data[1]);
					++sn;
					printf("--> Sending ACK%u ..... ", sn);
					packet = build_packet(ACK, sn, 0x0000);
					if (send(client_sock, &packet, sizeof(packet_t), 0) < 0)
						perror("failed.\n\n");					
					else
						printf("success.\n\n");
				}
				// Wrong sequence number
				else {
					// Display SN, send back ACK of SN we expect
					printf("SN: %3u != %3u\n", packet.sn, sn);
					printf("--> Sending ACK%u ..... ", sn);
					packet = build_packet(ACK, sn, 0x0000); 
					if (send(client_sock, &packet, sizeof(packet_t), 0) < 0)
						perror("failed.\n\n");					
					else
						printf("success.\n\n");
				}
			}
			// There was an error
			else {
				printf("ERROR - SN: %3u\n", packet.sn);
				printf("--> Sending NAK%u ..... ", sn);
				packet = build_packet(NAK, sn, 0x0000);
					if (send(client_sock, &packet, sizeof(packet_t), 0) < 0)
						perror("failed.\n\n");					
					else
						printf("success.\n\n");
			}

    }
     
    // Might move back to receiver
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

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
