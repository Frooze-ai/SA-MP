
#pragma once

//----------------------------------------------------------

unsigned int crc32(unsigned int crc, const char *buf, int len);
unsigned int crc32(char *filename);
void sha1(char *filename, unsigned int *digest);
void sha1(char *buf, int len, unsigned int *digest);
void sha1_digest(unsigned int *digest, char *buf);

//----------------------------------------------------------
