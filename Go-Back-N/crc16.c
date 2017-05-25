#include "crc16.h"

// CCITT16 g(x)
uint8_t gx[17] = {1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,1};

/*
 * This function will either calculate a 16 bit CRC code or
 * check a 16 bit CRC code. It is an improved version of what
 * we submitted for the previous lab (Lab 3.)
 */
uint16_t calc_crc(uint8_t *data, uint16_t len, uint8_t cmd)
{
	int i,j,k;
	int idx = 0;

	/*
	 * CRC Generate
	 */
	if (cmd == CRC_GENERATE) {
		// Create a "binary" array from *data
		// Each bit from *data gets a single entry in arr
		uint8_t arr[len*8+16];
		for (i=0; i<len; ++i) {
			for(j=7; j>=0; --j) {
				if(data[i]>>j & 0x1)
					arr[idx++] = 1;
				else
					arr[idx++] = 0;
			}
		}
		// Zero the entries for the CRC
		for (i=0; i<16; ++i)
			arr[idx++] = 0;

		// Apply g(x)
		for (i=0; i<idx-16; ++i) {
			if (arr[i] == 1) {
				for (j=0, k=i; j<17; ++j, ++k)
					arr[k] ^= gx[j];
			}
		}

		// Recover the CRC - it is the last 16 entries in arr
		uint16_t crc;
		for (i=idx-17; i<idx; ++i)
			crc = (crc<<1) | arr[i];

		return crc;

	}
	/*
	 * CRC Check
	 */
	else {
		// Create a "binary" array from *data
		// Each bit from *data gets a single entry in arr
		uint8_t arr[len*8];
		for (i=0; i<len; ++i) {
			for(j=7; j>=0; --j) {
				if(data[i]>>j & 0x1)
					arr[idx++] = 1;
				else
					arr[idx++] = 0;
			}
		}

		// Apply g(x)
		for (i=0; i<idx-16; ++i) {
			if (arr[i] == 1) {
				for (j=0, k=i; j<17; ++j, ++k)
					arr[k] ^= gx[j];
			}
		}
		
		// Recover the CRC - it is the last 16 entries in arr
		uint16_t crc;
		for (i=idx-17; i<idx; ++i)
			crc = (crc<<1) | arr[i];

		// Return either success or failure
		if (crc == 0) 
			return CRC_CHECK_SUCCESS;
		else 
			return CRC_CHECK_FAILURE;
	}

}
