/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_utils.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_utils__
#define __dng_utils__

/*****************************************************************************/

#include "dng_classes.h"
#include "dng_flags.h"
#include "dng_types.h"

/*****************************************************************************/

inline uint32 Abs_int32 (int32 x)
	{

	#if 0

	// Reference version.

	return (uint32) (x < 0 ? -x : x);

	#else

	// Branchless version.

    uint32 mask = x >> 31;

    return (uint32) ((x + mask) ^ mask);

	#endif

	}

inline int32 Min_int32 (int32 x, int32 y)
	{

	return (x <= y ? x : y);

	}

inline int32 Max_int32 (int32 x, int32 y)
	{

	return (x >= y ? x : y);

	}

inline int32 Pin_int32 (int32 min, int32 x, int32 max)
	{

	return Max_int32 (min, Min_int32 (x, max));

	}

inline int32 Pin_int32_between (int32 a, int32 x, int32 b)
	{

	int32 min, max;
	if (a < b) { min = a; max = b; }
	else { min = b; max = a; }

	return Pin_int32 (min, x, max);

	}

/*****************************************************************************/

inline uint16 Min_uint16 (uint16 x, uint16 y)
	{

	return (x <= y ? x : y);

	}

inline uint16 Max_uint16 (uint16 x, uint16 y)
	{

	return (x >= y ? x : y);

	}

inline int16 Pin_int16 (int32 x)
	{

	x = Pin_int32 (-32768, x, 32767);

	return (int16) x;

	}

/*****************************************************************************/

inline uint32 Min_uint32 (uint32 x, uint32 y)
	{

	return (x <= y ? x : y);

	}

inline uint32 Min_uint32 (uint32 x, uint32 y, uint32 z)
	{

	return Min_uint32 (x, Min_uint32 (y, z));

	}

inline uint32 Max_uint32 (uint32 x, uint32 y)
	{

	return (x >= y ? x : y);

	}

inline uint32 Max_uint32 (uint32 x, uint32 y, uint32 z)
	{

	return Max_uint32 (x, Max_uint32 (y, z));

	}

inline uint32 Pin_uint32 (uint32 min, uint32 x, uint32 max)
	{

	return Max_uint32 (min, Min_uint32 (x, max));

	}

/*****************************************************************************/

inline uint16 Pin_uint16 (int32 x)
	{

	#if 0

	// Reference version.

	x = Pin_int32 (0, x, 0x0FFFF);

	#else

	// Single branch version.

	if (x & ~65535)
		{

		x = ~x >> 31;

		}

	#endif

	return (uint16) x;

	}

/*****************************************************************************/

inline uint32 RoundUp2 (uint32 x)
	{

	return (x + 1) & ~1;

	}

inline uint32 RoundUp4 (uint32 x)
	{

	return (x + 3) & ~3;

	}

inline uint32 RoundUp8 (uint32 x)
	{

	return (x + 7) & ~7;

	}

inline uint32 RoundUp16 (uint32 x)
	{

	return (x + 15) & ~15;

	}

inline uint32 RoundUp4096 (uint32 x)
	{

	return (x + 4095) & ~4095;

	}

/******************************************************************************/

inline uint32 RoundDown2 (uint32 x)
	{

	return x & ~1;

	}

inline uint32 RoundDown4 (uint32 x)
	{

	return x & ~3;

	}

inline uint32 RoundDown8 (uint32 x)
	{

	return x & ~7;

	}

inline uint32 RoundDown16 (uint32 x)
	{

	return x & ~15;

	}

/******************************************************************************/

inline uint32 RoundUpForPixelSize (uint32 x, uint32 pixelSize)
	{

	switch (pixelSize)
		{

		case 1:
			return RoundUp16 (x);

		case 2:
			return RoundUp8 (x);

		case 4:
			return RoundUp4 (x);

		case 8:
			return RoundUp2 (x);

		default:
			return RoundUp16 (x);

		}

	}

/******************************************************************************/

inline uint64 Abs_int64 (int64 x)
	{

	return (uint64) (x < 0 ? -x : x);

	}

inline int64 Min_int64 (int64 x, int64 y)
	{

	return (x <= y ? x : y);

	}

inline int64 Max_int64 (int64 x, int64 y)
	{

	return (x >= y ? x : y);

	}

inline int64 Pin_int64 (int64 min, int64 x, int64 max)
	{

	return Max_int64 (min, Min_int64 (x, max));

	}

/******************************************************************************/

inline uint64 Min_uint64 (uint64 x, uint64 y)
	{

	return (x <= y ? x : y);

	}

inline uint64 Max_uint64 (uint64 x, uint64 y)
	{

	return (x >= y ? x : y);

	}

inline uint64 Pin_uint64 (uint64 min, uint64 x, uint64 max)
	{

	return Max_uint64 (min, Min_uint64 (x, max));

	}

/*****************************************************************************/

inline real32 Abs_real32 (real32 x)
	{

	return (x < 0.0f ? -x : x);

	}

inline real32 Min_real32 (real32 x, real32 y)
	{

	return (x < y ? x : y);

	}

inline real32 Max_real32 (real32 x, real32 y)
	{

	return (x > y ? x : y);

	}

inline real32 Pin_real32 (real32 min, real32 x, real32 max)
	{

	return Max_real32 (min, Min_real32 (x, max));

	}

inline real32 Pin_real32 (real32 x)
	{

	return Pin_real32 (0.0f, x, 1.0f);

	}

inline real32 Lerp_real32 (real32 a, real32 b, real32 t)
	{

	return a + t * (b - a);

	}

/*****************************************************************************/

inline real64 Abs_real64 (real64 x)
	{

	return (x < 0.0 ? -x : x);

	}

inline real64 Min_real64 (real64 x, real64 y)
	{

	return (x < y ? x : y);

	}

inline real64 Max_real64 (real64 x, real64 y)
	{

	return (x > y ? x : y);

	}

inline real64 Pin_real64 (real64 min, real64 x, real64 max)
	{

	return Max_real64 (min, Min_real64 (x, max));

	}

inline real64 Pin_real64 (real64 x)
	{

	return Pin_real64 (0.0, x, 1.0);

	}

inline real64 Lerp_real64 (real64 a, real64 b, real64 t)
	{

	return a + t * (b - a);

	}

/*****************************************************************************/

inline int32 Round_int32 (real32 x)
	{

	return (int32) (x > 0.0f ? x + 0.5f : x - 0.5f);

	}

inline int32 Round_int32 (real64 x)
	{

	return (int32) (x > 0.0 ? x + 0.5 : x - 0.5);

	}

inline uint32 Floor_uint32 (real32 x)
	{

	return (uint32) Max_real32 (0.0f, x);

	}

inline uint32 Floor_uint32 (real64 x)
	{

	return (uint32) Max_real64 (0.0, x);

	}

inline uint32 Round_uint32 (real32 x)
	{

	return Floor_uint32 (x + 0.5f);

	}

inline uint32 Round_uint32 (real64 x)
	{

	return Floor_uint32 (x + 0.5);

	}

/******************************************************************************/

inline int64 Round_int64 (real64 x)
	{

	return (int64) (x >= 0.0 ? x + 0.5 : x - 0.5);

	}

/*****************************************************************************/

const int64 kFixed64_One  = (((int64) 1) << 32);
const int64 kFixed64_Half = (((int64) 1) << 31);

/******************************************************************************/

inline int64 Real64ToFixed64 (real64 x)
	{

	return Round_int64 (x * (real64) kFixed64_One);

	}

/******************************************************************************/

inline real64 Fixed64ToReal64 (int64 x)
	{

	return x * (1.0 / (real64) kFixed64_One);

	}

/*****************************************************************************/

inline char ForceUppercase (char c)
	{

	if (c >= 'a' && c <= 'z')
		{

		c -= 'a' - 'A';

		}

	return c;

	}

/*****************************************************************************/

inline uint16 SwapBytes16 (uint16 x)
	{

	return (x << 8) |
		   (x >> 8);

	}

inline uint32 SwapBytes32 (uint32 x)
	{

	return (x << 24) +
		   ((x << 8) & 0x00FF0000) +
		   ((x >> 8) & 0x0000FF00) +
		   (x >> 24);

	}

/*****************************************************************************/

inline bool IsAligned16 (const void *p)
	{

	return (((uintptr) p) & 1) == 0;

	}

inline bool IsAligned32 (const void *p)
	{

	return (((uintptr) p) & 3) == 0;

	}

inline bool IsAligned64 (const void *p)
	{

	return (((uintptr) p) & 7) == 0;

	}

inline bool IsAligned128 (const void *p)
	{

	return (((uintptr) p) & 15) == 0;

	}

/******************************************************************************/

// Converts from RGB values (range 0.0 to 1.0) to HSV values (range 0.0 to
// 6.0 for hue, and 0.0 to 1.0 for saturation and value).

inline void DNG_RGBtoHSV (real32 r,
					      real32 g,
					      real32 b,
					      real32 &h,
					      real32 &s,
					      real32 &v)
	{

	v = Max_real32 (r, Max_real32 (g, b));

	real32 gap = v - Min_real32 (r, Min_real32 (g, b));

	if (gap > 0.0f)
		{

		if (r == v)
			{

			h = (g - b) / gap;

			if (h < 0.0f)
				{
				h += 6.0f;
				}

			}

		else if (g == v)
			{
			h = 2.0f + (b - r) / gap;
			}

		else
			{
			h = 4.0f + (r - g) / gap;
			}

		s = gap / v;

		}

	else
		{
		h = 0.0f;
		s = 0.0f;
		}

	}

/*****************************************************************************/

// Converts from HSV values (range 0.0 to 6.0 for hue, and 0.0 to 1.0 for
// saturation and value) to RGB values (range 0.0 to 1.0).

inline void DNG_HSVtoRGB (real32 h,
						  real32 s,
						  real32 v,
						  real32 &r,
						  real32 &g,
						  real32 &b)
	{

	if (s > 0.0f)
		{

		if (h < 0.0f)
			h += 6.0f;

		if (h >= 6.0f)
			h -= 6.0f;

		int32  i = (int32) h;
		real32 f = h - (real32) i;

		real32 p = v * (1.0f - s);

		#define q	(v * (1.0f - s * f))
		#define t	(v * (1.0f - s * (1.0f - f)))

		switch (i)
			{
			case 0: r = v; g = t; b = p; break;
			case 1: r = q; g = v; b = p; break;
			case 2: r = p; g = v; b = t; break;
			case 3: r = p; g = q; b = v; break;
			case 4: r = t; g = p; b = v; break;
			case 5: r = v; g = p; b = q; break;
			}

		#undef q
		#undef t

		}

	else
		{
		r = v;
		g = v;
		b = v;
		}

	}

/******************************************************************************/

// High resolution timer, for code profiling.

real64 TickTimeInSeconds ();

// Lower resolution timer, but more stable.

real64 TickCountInSeconds ();

/******************************************************************************/

class dng_timer
	{

	public:

		dng_timer (const char *message);

		~dng_timer ();

	private:

		// Hidden copy constructor and assignment operator.

		dng_timer (const dng_timer &timer);

		dng_timer & operator= (const dng_timer &timer);

	private:

		const char *fMessage;

		real64 fStartTime;

	};

/*****************************************************************************/

// Returns the maximum squared Euclidean distance from the specified point to the
// specified rectangle rect.

real64 MaxSquaredDistancePointToRect (const dng_point_real64 &point,
									  const dng_rect_real64 &rect);

/*****************************************************************************/

// Returns the maximum Euclidean distance from the specified point to the specified
// rectangle rect.

real64 MaxDistancePointToRect (const dng_point_real64 &point,
							   const dng_rect_real64 &rect);

/*****************************************************************************/

#endif

/*****************************************************************************/
