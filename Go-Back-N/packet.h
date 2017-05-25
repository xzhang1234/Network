#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

#define DATA 0x1
#define ACK  0x2
#define NAK  0x3

/*
 * Packet structure used for communication between
 * the sender and receiver programs.
 */
typedef struct {
	uint8_t type;
	uint8_t sn;
	uint8_t data[2];
	uint8_t crc[2];
} packet_t;

#endif
