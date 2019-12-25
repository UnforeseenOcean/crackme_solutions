#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#define ECB 1
#include "aes-min.h"

//CRC32 determined using Ghidra
int32_t crc32(int32_t namelen, uint8_t* buffer) {

	if (namelen <= 0) 
	{
		return 0;
	}
	uint32_t result = 0;
	for (int i = 0; i < namelen; i++)
	{
		int bitcount = 8;
		result ^= (buffer[i] ^ 0xffffffff);

		do 
		{
			uint32_t calc = -(result & 1) & 0xedb88320;
			result = result >> 1 ^ calc;
			bitcount--;
		} 
		while (bitcount != 0);

		result ^= 0xffffffff;
	}
	return result;
}

void init()
{

}

struct keydata_format 
{
	uint32_t checksum;
	uint16_t feature_flags;
	uint16_t magic;
	uint32_t serial;
	uint32_t random;
};

void process_serial(char* name, char* serial_out)
{
	uint8_t key_schedule[AES128_KEY_SCHEDULE_SIZE];
	uint32_t crc = crc32(lstrlen(name), name);
	//generated by a C runtime RNG with seed 0x12345678
	uint8_t aes_key[0x10] = 
	{ 
		0xE9, 0x3F, 0x0D, 0xA1, 0x96, 0x95, 0x31, 0x04, 0x49, 0x2D, 0x9E, 0x61, 0x83, 0xCF, 0x09, 0x6F 
	};
	uint8_t buffer[AES_BLOCK_SIZE] = { 0 };
	uint8_t buffer2[AES_BLOCK_SIZE] = { 0 };
	aes128_key_schedule(key_schedule, aes_key);

	struct keydata_format* hdr = (struct keydata_format*)buffer;
	hdr->checksum = crc;
	hdr->feature_flags = 0xFFFF;
	hdr->magic = 0x1979;
	hdr->random = 0;
	hdr->serial = 0xDEADB00B;

	uint32_t hash = 0;
	do
	{
		memcpy(buffer2, buffer, 0x10);
		aes128_encrypt(buffer2, key_schedule);
		hdr->random++;
		if (hdr->random == 0)
		hdr->serial++;
		hash = 0;
		for (uint8_t i = 0; i < 12; i++)
			hash += buffer2[i];
	} 
	while (buffer2[0x0E] != (hash >> 8) || buffer2[0x0F] != (uint8_t)hash);

	uint8_t buffer_str[33];
	for (uint8_t i = 0; i < 16; ++i)
		sprintf(&buffer_str[i * 2], "%02X", (uint8_t)buffer2[i]);

	sprintf(serial_out, "N=%s K=%s", name, buffer_str);
}


