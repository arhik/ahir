/* AES-Rijndael Block Cipher

Written by Prakalp Somawanshi 11th Feb 2008
prakalp@ee.iitb.ac.in
   
Copyright (c) 2008 prakalp

Simply compile and run.

See rijndael documentation. The code follows the documentation as closely
as possible, and where possible uses the same function and variable names.
"AES Proposal:Rijndael" by Joan Daemen and Vincent Rijmen. 

Inspiration from Adam Berent implementation is acknowledged.

Written for clarity, rather than speed.
Assumes long is 32 bit quantity.
Full implementation. 
   
Version Speciality: DATA Type: Word,Byte
Pre-Computation of Tables
with Key Expansion 
Shift Row and Substitution table combination 
Only Encryption.
*/


#define BYTE unsigned char	/* 8 bits  */
#define WORD unsigned long	/* 32 bits */

#ifdef RUN
#include <stdio.h>
#endif

/* Circular Shift by one Bit of Byte   ..Eg   1100 1100 => 1001 1001 */
#define ROTL(x) ( (((x)>>7)|((x)<<1)) & 0xFF)

/* Circular Shift by one Byte of Word  ..Eg   2 3 1 4 => 3 1 4 2  */
#define ROTL8(x) (((x)<<8)|((x)>>24))

/* Circular Shift by two Byte of Word  ..Eg   2 3 1 4 => 1 4 2 3  */
#define ROTL16(x) (((x)<<16)|((x)>>16))

/* Circular Shift by three Byte of Word..Eg   2 3 1 4 => 4 2 3 1  */
#define ROTL24(x) (((x)<<24)|((x)>>8))


#define nb 128			/* nb = block size in Bits */
#define nk 128			/* nk = Key Lenngth in Bits */
#define Nb (nb>>5)		/* Nb= Number of colomn in the STATE = block lengh/32 */
#define Nk (nk>>5)		/* Nk= Number of colomn in Cipher key= Keylength/32   */
#define Nr 10			/* Nr= Number of rounds as a function of block and key length */

void encrypt();
WORD SubByte(WORD);
/*
  WORD pack(BYTE *);
*/
void unpack(WORD, BYTE *);
WORD MixCol(WORD x);
void gkey();
BYTE product(WORD, WORD);
BYTE bmul(BYTE, BYTE);
BYTE mod(WORD, WORD);


/* Polynomial for encryption */
/* BYTE MxCo[4]={0x02, 0x03, 0x01, 0x01}; */
#define MxCo 0x1010302

/* Constant matrix to help Multiplication over GF(2^8)*/
BYTE E[256] = {
    0x01, 0x03, 0x05, 0x0f, 0x11, 0x33, 0x55, 0xff, 0x1a, 0x2e, 0x72, 0x96,
    0xa1, 0xf8, 0x13, 0x35,
    0x5f, 0xe1, 0x38, 0x48, 0xd8, 0x73, 0x95, 0xa4, 0xf7, 0x02, 0x06, 0x0a,
    0x1e, 0x22, 0x66, 0xaa,
    0xe5, 0x34, 0x5c, 0xe4, 0x37, 0x59, 0xeb, 0x26, 0x6a, 0xbe, 0xd9, 0x70,
    0x90, 0xab, 0xe6, 0x31,
    0x53, 0xf5, 0x04, 0x0c, 0x14, 0x3c, 0x44, 0xcc, 0x4f, 0xd1, 0x68, 0xb8,
    0xd3, 0x6e, 0xb2, 0xcd,
    0x4c, 0xd4, 0x67, 0xa9, 0xe0, 0x3b, 0x4d, 0xd7, 0x62, 0xa6, 0xf1, 0x08,
    0x18, 0x28, 0x78, 0x88,
    0x83, 0x9e, 0xb9, 0xd0, 0x6b, 0xbd, 0xdc, 0x7f, 0x81, 0x98, 0xb3, 0xce,
    0x49, 0xdb, 0x76, 0x9a,
    0xb5, 0xc4, 0x57, 0xf9, 0x10, 0x30, 0x50, 0xf0, 0x0b, 0x1d, 0x27, 0x69,
    0xbb, 0xd6, 0x61, 0xa3,
    0xfe, 0x19, 0x2b, 0x7d, 0x87, 0x92, 0xad, 0xec, 0x2f, 0x71, 0x93, 0xae,
    0xe9, 0x20, 0x60, 0xa0,
    0xfb, 0x16, 0x3a, 0x4e, 0xd2, 0x6d, 0xb7, 0xc2, 0x5d, 0xe7, 0x32, 0x56,
    0xfa, 0x15, 0x3f, 0x41,
    0xc3, 0x5e, 0xe2, 0x3d, 0x47, 0xc9, 0x40, 0xc0, 0x5b, 0xed, 0x2c, 0x74,
    0x9c, 0xbf, 0xda, 0x75,
    0x9f, 0xba, 0xd5, 0x64, 0xac, 0xef, 0x2a, 0x7e, 0x82, 0x9d, 0xbc, 0xdf,
    0x7a, 0x8e, 0x89, 0x80,
    0x9b, 0xb6, 0xc1, 0x58, 0xe8, 0x23, 0x65, 0xaf, 0xea, 0x25, 0x6f, 0xb1,
    0xc8, 0x43, 0xc5, 0x54,
    0xfc, 0x1f, 0x21, 0x63, 0xa5, 0xf4, 0x07, 0x09, 0x1b, 0x2d, 0x77, 0x99,
    0xb0, 0xcb, 0x46, 0xca,
    0x45, 0xcf, 0x4a, 0xde, 0x79, 0x8b, 0x86, 0x91, 0xa8, 0xe3, 0x3e, 0x42,
    0xc6, 0x51, 0xf3, 0x0e,
    0x12, 0x36, 0x5a, 0xee, 0x29, 0x7b, 0x8d, 0x8c, 0x8f, 0x8a, 0x85, 0x94,
    0xa7, 0xf2, 0x0d, 0x17,
    0x39, 0x4b, 0xdd, 0x7c, 0x84, 0x97, 0xa2, 0xfd, 0x1c, 0x24, 0x6c, 0xb4,
    0xc7, 0x52, 0xf6, 0x01
};


/* Constant matrix to help Multiplication over GF(2^8)*/
BYTE L[256] = {
    0x00, 0x00, 0x19, 0x01, 0x32, 0x02, 0x1a, 0xc6, 0x4b, 0xc7, 0x1b, 0x68,
    0x33, 0xee, 0xdf, 0x03,
    0x64, 0x04, 0xe0, 0x0e, 0x34, 0x8d, 0x81, 0xef, 0x4c, 0x71, 0x08, 0xc8,
    0xf8, 0x69, 0x1c, 0xc1,
    0x7d, 0xc2, 0x1d, 0xb5, 0xf9, 0xb9, 0x27, 0x6a, 0x4d, 0xe4, 0xa6, 0x72,
    0x9a, 0xc9, 0x09, 0x78,
    0x65, 0x2f, 0x8a, 0x05, 0x21, 0x0f, 0xe1, 0x24, 0x12, 0xf0, 0x82, 0x45,
    0x35, 0x93, 0xda, 0x8e,
    0x96, 0x8f, 0xdb, 0xbd, 0x36, 0xd0, 0xce, 0x94, 0x13, 0x5c, 0xd2, 0xf1,
    0x40, 0x46, 0x83, 0x38,
    0x66, 0xdd, 0xfd, 0x30, 0xbf, 0x06, 0x8b, 0x62, 0xb3, 0x25, 0xe2, 0x98,
    0x22, 0x88, 0x91, 0x10,
    0x7e, 0x6e, 0x48, 0xc3, 0xa3, 0xb6, 0x1e, 0x42, 0x3a, 0x6b, 0x28, 0x54,
    0xfa, 0x85, 0x3d, 0xba,
    0x2b, 0x79, 0x0a, 0x15, 0x9b, 0x9f, 0x5e, 0xca, 0x4e, 0xd4, 0xac, 0xe5,
    0xf3, 0x73, 0xa7, 0x57,
    0xaf, 0x58, 0xa8, 0x50, 0xf4, 0xea, 0xd6, 0x74, 0x4f, 0xae, 0xe9, 0xd5,
    0xe7, 0xe6, 0xad, 0xe8,
    0x2c, 0xd7, 0x75, 0x7a, 0xeb, 0x16, 0x0b, 0xf5, 0x59, 0xcb, 0x5f, 0xb0,
    0x9c, 0xa9, 0x51, 0xa0,
    0x7f, 0x0c, 0xf6, 0x6f, 0x17, 0xc4, 0x49, 0xec, 0xd8, 0x43, 0x1f, 0x2d,
    0xa4, 0x76, 0x7b, 0xb7,
    0xcc, 0xbb, 0x3e, 0x5a, 0xfb, 0x60, 0xb1, 0x86, 0x3b, 0x52, 0xa1, 0x6c,
    0xaa, 0x55, 0x29, 0x9d,
    0x97, 0xb2, 0x87, 0x90, 0x61, 0xbe, 0xdc, 0xfc, 0xbc, 0x95, 0xcf, 0xcd,
    0x37, 0x3f, 0x5b, 0xd1,
    0x53, 0x39, 0x84, 0x3c, 0x41, 0xa2, 0x6d, 0x47, 0x14, 0x2a, 0x9e, 0x5d,
    0x56, 0xf2, 0xd3, 0xab,
    0x44, 0x11, 0x92, 0xd9, 0x23, 0x20, 0x2e, 0x89, 0xb4, 0x7c, 0xb8, 0x26,
    0x77, 0x99, 0xe3, 0xa5,
    0x67, 0x4a, 0xed, 0xde, 0xc5, 0x31, 0xfe, 0x18, 0x0d, 0x63, 0x8c, 0x80,
    0xc0, 0xf7, 0x70, 0x07
};

BYTE fbsub[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b,
    0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf,
    0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1,
    0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2,
    0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3,
    0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39,
    0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f,
    0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21,
    0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d,
    0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14,
    0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62,
    0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea,
    0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f,
    0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9,
    0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9,
    0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f,
    0xb0, 0x54, 0xbb, 0x16
};


WORD Rcon[15] =
    { 0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000,
    0x20000000, 0x40000000, 0x80000000, 0x1b000000, 0x36000000,
    0x6c000000, 0xd8000000, 0xab000000, 0x4d000000, 0x9a000000
};


/* x.y= AntiLog(Log(x) + Log(y)) */
/*
BYTE bmul(BYTE x, BYTE y)
{
    if (x && y)
	return E[mod((L[x] + L[y]), 255)];
    else
	return 0;
}
*/

#define bmul(x, y) (((x) && (y)) ? E[mod((L[(x)] + L[(y)]), 255)] : 0)

BYTE mod(WORD a, WORD b)
{
    if (b == 0)
	return (BYTE) 0;
    if (a < b)
	return (BYTE) a;
    while (a >= b) {
	a = a - b;
    }
    return (BYTE) a;
}

BYTE product(WORD x, WORD y)
{				/* dot product of two 4-byte arrays */
    BYTE xb[4], yb[4];
    unpack(x, xb);
    unpack(y, yb);
    return bmul(xb[0], yb[0])
	^ bmul(xb[1], yb[1])
	^ bmul(xb[2], yb[2])
	^ bmul(xb[3], yb[3]);
}

/* pack bytes into a 32-bit Word */
/*
WORD pack(BYTE * b)
{
    return (((WORD) b[3] << 24)
	    | ((WORD) b[2] << 16)
	    | ((WORD) b[1] << 8)
	    | b[0]);
}
*/

#define pack(b) (((WORD) (b)[3] << 24)		     \
		 | ((WORD) (b)[2] << 16)	     \
		 | ((WORD) (b)[1] << 8)		     \
		 | (b)[0])


/* unpack bytes from a word */
void unpack(WORD a, BYTE * b)
{
    b[0] = (BYTE) a;
    b[1] = (BYTE) (a >> 8);
    b[2] = (BYTE) (a >> 16);
    b[3] = (BYTE) (a >> 24);
}

BYTE key[16] = {
    1, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
};

BYTE block[16] = {
    0, 1, 2, 3,
    4, 5, 6, 7,
    8, 9, 10, 11,
    12, 13, 14, 15
};

/*Expanded Key Buffer size= One round words*(Number of rounds +1 )*/
WORD W[Nb * (Nr + 1)];

void start(void)
{
    BYTE i, j;
    WORD a[Nb];			/* State length in words : 128 bit=> 4 Words */


    /*                  Expansion of Key                              */

    W[0] = 10;
    gkey();

    /*    Encryption Round : 10 rounds for 128,128 Key and block size resp.  */

    encrypt();
}

/*============================================================================ */
/*                          Key-Expansion                                      */
/* =========================================================================== */

void gkey()
{
    BYTE i, j;
    WORD temp;

    if (Nk < 6) {
	for (i = 0, j = 0; i < Nk; i++, j += 4)
	    W[i] = pack(key + j);
	for (i = Nk; i < (Nb * (Nr + 1)); i++) {
	    temp = W[i - 1];
	    if (i % Nk == 0)
		temp = SubByte(ROTL8(temp)) ^ Rcon[(i >> 2) - 1];
	    W[i] = W[i - Nk] ^ temp;
	}
    } else {
	for (i = 0, j = 0; i < Nk; i++, j += 4)
	    W[i] = pack(key + j);
	for (i = Nk; i < (Nb * (Nr + 1)); i++) {
	    temp = W[i - 1];
	    if (i % Nk == 0)
		temp = SubByte(ROTL8(temp)) ^ Rcon[(i >> 2) - 1];
	    else if (i % Nk == 4)
		temp = SubByte(temp);
	    W[i] = W[i - Nk] ^ temp;
	}
    }

}

/*===================================================================================*/
/*                     Encryption of 128 bit Block plain Text                         */
/*====================================================================================*/

void encrypt()
{
    BYTE i, j, k, c1, c2, c3, c6;
    WORD a;

    for (k = 0; k < 10; k++) {

	/*Round Key Addition  */
	for (i = j = 0; i < Nb; i++, j += 4) {
	    a = pack(block + j);
	    a ^= W[i + (k * 4)];
	    unpack(a, (block + j));
	}

	/*Byte Sub Transf rmation: nonlinear byte substitution */
	/*+ ShiftRow Transformation: Cyclically shifted over different ofsets */

	/* The following substitutions are short-circuited in the
	   actual code below 
	   c[0] = fbsub[block[0]];
	   c[1] = fbsub[block[5]];
	   c[2] = fbsub[block[10]];
	   c[3] = fbsub[block[15]];
	   
	   c[4] = fbsub[block[4]];
	   c[5] = fbsub[block[9]];
	   c[6] = fbsub[block[14]];
	   c[7] = fbsub[block[3]];
	   
	   c[8] = fbsub[block[8]];
	   c[9] = fbsub[block[13]];
	   c[10] = fbsub[block[2]];
	   c[11] = fbsub[block[7]];
	   
	   c[12] = fbsub[block[12]];
	   c[13] = fbsub[block[1]];
	   c[14] = fbsub[block[6]];
	   c[15] = fbsub[block[11]];
	   
	   for (i = 0; i < 16; i++)
	     block[i] = c[i];
	*/

	block[0] = fbsub[block[0]];
	c1 = fbsub[block[5]];
	
	c2 = fbsub[block[10]];
	c3 = fbsub[block[15]];
	
	block[4] = fbsub[block[4]];
	block[5] = fbsub[block[9]];
	block[9] = fbsub[block[13]];
	block[13] = fbsub[block[1]];

	c6 = fbsub[block[14]];
	
	block[8] = fbsub[block[8]];
	block[10] = fbsub[block[2]];
	block[12] = fbsub[block[12]];
	block[14] = fbsub[block[6]];
	block[15] = fbsub[block[11]];
	block[11] = fbsub[block[7]];
	block[7] = fbsub[block[3]];
	

	block[1] = c1;
	block[2] = c2;
	block[3] = c3;
	block[6] = c6;
	   
	/* Mix Column Transformation : State is considered as a polynomial
	   over GF(2^8) and multiplied modulo x^4+1 with a fixed polynomial c(x)
	   given by '03'X^3 + '01'X^2 + '01'X + '02' which is coprime to 'x^4+1' */

	if (k != 9) {
	    for (i = j = 0; i < Nb; i++, j += 4) {
		a = pack(block + j);
		a = MixCol(a);
		unpack(a, (block + j));
	    }
	}

    }

    for (i = j = 0; i < Nb; i++, j += 4) {
	a = pack(block + j);
	a ^= W[i + (k * 4)];
	unpack(a, (block + j));
    }
}

/*====================================================================================*/
/*    Pre Calculated value of S-Box : during encryption each value
      of the state is replaced with the corresponding S-Box value 
      There is an obvious time/space trade-off possible here.      
      You could have calculated it onfly                                           */
/* ================================================================================*/

WORD SubByte(WORD a)
{
    BYTE b[4];
    unpack(a, b);
    b[0] = fbsub[b[0]];
    b[1] = fbsub[b[1]];
    b[2] = fbsub[b[2]];
    b[3] = fbsub[b[3]];
    return pack(b);
}

/*===============================================================================*/
/*                    matrix Multiplication over Galois Field                    */
/*===============================================================================*/

WORD MixCol(WORD x)
{
    WORD y, m;
    BYTE b[4];

    m = MxCo;
    b[0] = product(m, x);
    m = ROTL8(m);
    b[1] = product(m, x);
    m = ROTL8(m);
    b[2] = product(m, x);
    m = ROTL8(m);
    b[3] = product(m, x);
    y = pack(b);
    return y;
}

#ifdef RUN

int main(void)
{
    BYTE init[16] = {
	0, 1, 2, 3,
	4, 5, 6, 7,
	8, 9, 10, 11,
	12, 13, 14, 15
    };

    int i, run;
    long long t1[10], t2[10];

    for (run = 0; run < 10; run++) {
	for (i = 0; i < 16; i++) {
	    block[i] = init[i];
	}

	t1[run] = cpucycles();
	start();
	t2[run] = cpucycles();
    }

    for (run = 0; run < 10; run++) {
	printf("\n%lld", t2[run] - t1[run]);
    }
    printf("\n");
    
    return 0;
}

#endif
