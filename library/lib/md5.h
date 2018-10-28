#ifndef __MD5_H__
#define __MD5_H__
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;
typedef unsigned char BYTE;
typedef int WORD;
/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;

/* PROTO_LIST is defined depending on how PROTOTYPES is defined above.
If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
returns an empty list.
*/
 

/* MD5 context. */
typedef struct {
  UINT4 state[4];                                   /* state (ABCD) */
  UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;

void MD5Init(MD5_CTX *);
void MD5Update(MD5_CTX *, unsigned char *, unsigned int);
void MD5Final(unsigned char [16], MD5_CTX *);
void md5Calc(uint8_t *md5_digest, uint8_t *text, uint32_t len_text);
void hmacMd5(const unsigned char *text, int text_len,
	      const unsigned char *key, int key_len,
	      unsigned char *digest);

#endif

