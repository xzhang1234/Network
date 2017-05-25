#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

/*
 * Packet structure used for communication between
 * the sender and receiver programs.
 */
typedef struct {
	uint16_t sn;
	uint8_t data[2];
	uint8_t crc[2];
	char n;
} packet_t;

packet_t packet(uint16_t sn, uint8_t data1, uint8_t data2);

#endif
