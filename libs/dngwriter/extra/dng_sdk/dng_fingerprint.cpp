/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_fingerprint.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_fingerprint.h"

#include "dng_assertions.h"
#include "dng_flags.h"

/*****************************************************************************/

dng_fingerprint::dng_fingerprint ()
	{

	for (uint32 j = 0; j < 16; j++)
		{

		data [j] = 0;

		}

	}

/*****************************************************************************/

bool dng_fingerprint::IsNull () const
	{

	for (uint32 j = 0; j < 16; j++)
		{

		if (data [j] != 0)
			{

			return false;

			}

		}

	return true;

	}

/*****************************************************************************/

bool dng_fingerprint::operator== (const dng_fingerprint &print) const
	{

	for (uint32 j = 0; j < 16; j++)
		{

		if (data [j] != print.data [j])
			{

			return false;

			}

		}

	return true;

	}

/******************************************************************************/

uint32 dng_fingerprint::Collapse32 () const
	{

	uint32 x = 0;

	for (uint32 j = 0; j < 4; j++)
		{

		uint32 y = 0;

		for (uint32 k = 0; k < 4; k++)
			{

			y = (y << 8) + (uint32) data [j * 4 + k];

			}

		x = x ^ y;

		}

	return x;

	}

/******************************************************************************/

// Derived from the RSA Data Security, Inc. MD5 Message-Digest Algorithm

// Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
// rights reserved.
//
// License to copy and use this software is granted provided that it
// is identified as the "RSA Data Security, Inc. MD5 Message-Digest
// Algorithm" in all material mentioning or referencing this software
// or this function.
//
// License is also granted to make and use derivative works provided
// that such works are identified as "derived from the RSA Data
// Security, Inc. MD5 Message-Digest Algorithm" in all material
// mentioning or referencing the derived work.
//
// RSA Data Security, Inc. makes no representations concerning either
// the merchantability of this software or the suitability of this
// software for any particular purpose. It is provided "as is"
// without express or implied warranty of any kind.
//
// These notices must be retained in any copies of any part of this
// documentation and/or software.

/******************************************************************************/

dng_md5_printer::dng_md5_printer ()

	:	final  (false)
	,	result ()

	{

	Reset ();

	}

/******************************************************************************/

void dng_md5_printer::Reset ()
	{

	// No bits processed yet.

	count [0] = 0;
	count [1] = 0;

	// Load magic initialization constants.

	state [0] = 0x67452301;
	state [1] = 0xefcdab89;
	state [2] = 0x98badcfe;
	state [3] = 0x10325476;

	// Not finalized yet.

	final = false;

	}

/******************************************************************************/

void dng_md5_printer::Process (const void *data,
					  		   uint32 inputLen)
	{

	DNG_ASSERT (!final, "Fingerprint already finalized!");

	const uint8 *input = (const uint8 *) data;

	// Compute number of bytes mod 64

	uint32 index = (count [0] >> 3) & 0x3F;

	// Update number of bits

	if ((count [0] += inputLen << 3) < (inputLen << 3))
		{
		count [1]++;
		}

	count [1] += inputLen >> 29;

	// Transform as many times as possible.

	uint32 i = 0;

	uint32 partLen = 64 - index;

	if (inputLen >= partLen)
		{

		memcpy (&buffer [index],
				input,
				partLen);

		MD5Transform (state, buffer);

		for (i = partLen; i + 63 < inputLen; i += 64)
			{

			MD5Transform (state, &input [i]);

			}

		index = 0;

		}

	// Buffer remaining input

	memcpy (&buffer [index],
			&input [i],
			inputLen - i);

	}

/******************************************************************************/

const dng_fingerprint & dng_md5_printer::Result ()
	{

	if (!final)
		{

		static uint8 PADDING [64] =
			{
			0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
			};

		// Save number of bits

		uint8 bits [8];

		Encode (bits, count, 8);

		// Pad out to 56 mod 64.

		uint32 index = (count [0] >> 3) & 0x3f;

		uint32 padLen = (index < 56) ? (56 - index) : (120 - index);

		Process (PADDING, padLen);

		// Append length (before padding)

		Process (bits, 8);

		// Store state in digest

		Encode (result.data, state, 16);

		// We are now finalized.

		final = true;

		}

	return result;

	}

/******************************************************************************/

// Encodes input (uint32) into output (uint8). Assumes len is
// a multiple of 4.

void dng_md5_printer::Encode (uint8 *output,
							  const uint32 *input,
							  uint32 len)
	{

	uint32 i, j;

	for (i = 0, j = 0; j < len; i++, j += 4)
		{
		output [j  ] = (uint8) ((input [i]      ) & 0xff);
		output [j+1] = (uint8) ((input [i] >>  8) & 0xff);
		output [j+2] = (uint8) ((input [i] >> 16) & 0xff);
		output [j+3] = (uint8) ((input [i] >> 24) & 0xff);
		}

	}

/******************************************************************************/

// Decodes input (uint8) into output (uint32). Assumes len is
// a multiple of 4.

void dng_md5_printer::Decode (uint32 *output,
							  const uint8 *input,
							  uint32 len)
	{

	// Check for non-aligned case.

	if (((uintptr) input) & 3)
		{

		uint32 i, j;

		for (i = 0, j = 0; j < len; i++, j += 4)
			{

	 		output [i] = (((uint32) input [j  ])      ) |
	 					 (((uint32) input [j+1]) <<  8) |
	   					 (((uint32) input [j+2]) << 16) |
	   					 (((uint32) input [j+3]) << 24);

	   		}

	   	}

	// Else use optimized code for aligned case.

	else
		{

		len = len >> 2;

		const uint32 *sPtr = (const uint32 *) input;

		uint32 *dPtr = output;

		while (len--)
			{

			#if qDNGBigEndian

			uint32 data = *(sPtr++);

			data = (data >> 24) |
				   ((data >> 8) & 0x0000FF00) |
				   ((data << 8) & 0x00FF0000) |
				   (data << 24);

			*(dPtr++) = data;

			#else

			*(dPtr++) = *(sPtr++);

			#endif

			}

		}

	}

/******************************************************************************/

// MD5 basic transformation. Transforms state based on block.

void dng_md5_printer::MD5Transform (uint32 state [4],
								    const uint8 block [64])
	{

	enum
		{
		S11 = 7,
		S12 = 12,
		S13 = 17,
		S14 = 22,
		S21 = 5,
		S22 = 9,
		S23 = 14,
		S24 = 20,
		S31 = 4,
		S32 = 11,
		S33 = 16,
		S34 = 23,
		S41 = 6,
		S42 = 10,
		S43 = 15,
		S44 = 21
		};

	#if qDNGBigEndian

	uint32 x [16];

	Decode (x, block, 64);

	#else

	uint32 temp [16];

	const uint32 *x;

	if (((uintptr) block) & 3)
		{

		Decode (temp, block, 64);

		x = temp;

		}

	else
		x = (const uint32 *) block;

	#endif

	uint32 a = state [0];
	uint32 b = state [1];
	uint32 c = state [2];
	uint32 d = state [3];

	/* Round 1 */
	FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
	FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
	FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
	FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
	FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
	FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
	FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
	FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
	FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
	FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
	FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

	/* Round 2 */
	GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
	GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
	GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
	GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
	GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
	GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
	GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
	GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
	GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
	GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
	GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
	GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

	/* Round 3 */
	HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
	HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
	HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
	HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
	HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
	HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
	HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
	HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
	HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
	HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

	/* Round 4 */
	II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
	II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
	II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
	II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
	II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
	II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
	II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
	II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
	II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
	II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

	state [0] += a;
	state [1] += b;
	state [2] += c;
	state [3] += d;

	}

/*****************************************************************************/

// End of RSA Data Security, Inc. derived code.

/*****************************************************************************/
