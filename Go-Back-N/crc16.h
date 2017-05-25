#ifndef CRC16_H
#define CRC16_H

#include <stdlib.h>
#include <stdint.h>

#define CRC_GENERATE 3
#define CRC_CHECK 2

#define CRC_CHECK_FAILURE 1
#define CRC_CHECK_SUCCESS 0

// Function Prototypes
/*
 * This function will either calculate a 16 bit CRC code or
 * check a 16 bit CRC code. It is an improved version of what
 * we submitted for the previous lab (Lab 3.)
 */
uint16_t calc_crc(uint8_t *data, uint16_t len, uint8_t cmd);

#endif
