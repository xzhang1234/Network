#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define POLYNOMIAL		(0x1021) 
#define BUFFER_LENGTH	(1024) 
long	M = 2147483647;
char	buffer[BUFFER_LENGTH];

uint16_t Input(char* buffer);
uint16_t GenCRC(char* const buffer, int length);
void     IntroduceError(char *data, double p);

int main (int argc, char** argv) {
	uint16_t const length = Input(buffer);
	uint16_t crc = GenCRC(buffer, length);
	printf("CRC: %x\n", crc);
	//form the codeword
	char* codeword = (char *) malloc(length + 3);
	memcpy(codeword, buffer, length);
	codeword[length + 2] = '\0';
	codeword[length + 1] = (char)crc;
	codeword[length] = (char)(crc >> 8);
	//Ex1
	IntroduceError(codeword,0);//atof(argv[1])
	//Ex2
	//codeword[length + 1] = codeword[length + 1] ^ (char) POLYNOMIAL;
	//codeword[length] = codeword[length] ^ (char) (POLYNOMIAL >> 8);
	//codeword[length - 1] = codeword[length - 1] ^ 0x01;
	printf("Corrupted codeword: ");
	for (uint16_t byte = 0; byte < length + 2; byte++)
		printf("%02x", (uint8_t) codeword[byte]); 
	GenCRC(codeword, length + 2) ? printf("\nError!\n") : printf("\nNo error!\n");
	//free(codeword);
}

uint16_t Input(char *buffer) {
	uint16_t length;
	printf("Enter the data: ");
	fgets(buffer, BUFFER_LENGTH, stdin);
	printf("Data in hex: ", buffer);
	for (length = 0; length < 1024 && buffer[length + 1] != '\0'; length++)
		printf("%02x", (uint8_t) buffer[length]);
	printf("\n");
	return length;
}

uint16_t GenCRC(char* const data, int length) {
	uint16_t remainder = 0x0000;	
	for (uint16_t byte = 0; byte < length; byte++) {
		remainder ^= (data[byte] << 8);
		for (uint8_t bit = 0; bit < 8; bit++) {
			if (remainder & 0x8000) {
				remainder = (remainder << 1) ^ POLYNOMIAL;
			} else {
				remainder = (remainder << 1);
			}			
		}
    }
    return remainder;	
}

void IntroduceError(char *data, double p) {
	char c, *pointer = data;
	int i;
	while (*pointer != '\0') {
		c = 0x01;
		for ( i = 0; i < 8; i++) {
			if ((double)rand()/M <= p)
				*pointer ^= c;
			c <<= 1;
		}
		pointer++;
	}
}