/*****************************************************************************/
// Copyright 2006 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_temperature.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

#include "dng_temperature.h"

#include "dng_xy_coord.h"

/*****************************************************************************/

// Scale factor between distances in uv space to a more user friendly "tint"
// parameter.

static const real64 kTintScale = -3000.0;

/*****************************************************************************/

// Table from Wyszecki & Stiles, "Color Science", second edition, page 228.

struct ruvt
	{
	real64 r;
	real64 u;
	real64 v;
	real64 t;
	};

static const ruvt kTempTable [] =
	{
	{   0, 0.18006, 0.26352, -0.24341 },
	{  10, 0.18066, 0.26589, -0.25479 },
	{  20, 0.18133, 0.26846, -0.26876 },
	{  30, 0.18208, 0.27119, -0.28539 },
	{  40, 0.18293, 0.27407, -0.30470 },
	{  50, 0.18388, 0.27709, -0.32675 },
	{  60, 0.18494, 0.28021, -0.35156 },
	{  70, 0.18611, 0.28342, -0.37915 },
	{  80, 0.18740, 0.28668, -0.40955 },
	{  90, 0.18880, 0.28997, -0.44278 },
	{ 100, 0.19032, 0.29326, -0.47888 },
	{ 125, 0.19462, 0.30141, -0.58204 },
	{ 150, 0.19962, 0.30921, -0.70471 },
	{ 175, 0.20525, 0.31647, -0.84901 },
	{ 200, 0.21142, 0.32312, -1.0182 },
	{ 225, 0.21807, 0.32909, -1.2168 },
	{ 250, 0.22511, 0.33439, -1.4512 },
	{ 275, 0.23247, 0.33904, -1.7298 },
	{ 300, 0.24010, 0.34308, -2.0637 },
	{ 325, 0.24702, 0.34655, -2.4681 },
	{ 350, 0.25591, 0.34951, -2.9641 },
	{ 375, 0.26400, 0.35200, -3.5814 },
	{ 400, 0.27218, 0.35407, -4.3633 },
	{ 425, 0.28039, 0.35577, -5.3762 },
	{ 450, 0.28863, 0.35714, -6.7262 },
	{ 475, 0.29685, 0.35823, -8.5955 },
	{ 500, 0.30505, 0.35907, -11.324 },
	{ 525, 0.31320, 0.35968, -15.628 },
	{ 550, 0.32129, 0.36011, -23.325 },
	{ 575, 0.32931, 0.36038, -40.770 },
	{ 600, 0.33724, 0.36051, -116.45 }
	};

/*****************************************************************************/

void dng_temperature::Set_xy_coord (const dng_xy_coord &xy)
	{

	// Convert to uv space.

	real64 u = 2.0 * xy.x / (1.5 - xy.x + 6.0 * xy.y);
	real64 v = 3.0 * xy.y / (1.5 - xy.x + 6.0 * xy.y);

	// Search for line pair coordinate is between.

	real64 last_dt = 0.0;

	real64 last_dv = 0.0;
	real64 last_du = 0.0;

	for (uint32 index = 1; index <= 30; index++)
		{

		// Convert slope to delta-u and delta-v, with length 1.

		real64 du = 1.0;
		real64 dv = kTempTable [index] . t;

		real64 len = sqrt (1.0 + dv * dv);

		du /= len;
		dv /= len;

		// Find delta from black body point to test coordinate.

		real64 uu = u - kTempTable [index] . u;
		real64 vv = v - kTempTable [index] . v;

		// Find distance above or below line.

		real64 dt = - uu * dv + vv * du;

		// If below line, we have found line pair.

		if (dt <= 0.0 || index == 30)
			{

			// Find fractional weight of two lines.

			if (dt > 0.0)
				dt = 0.0;

			dt = -dt;

			real64 f;

			if (index == 1)
				{
				f = 0.0;
				}
			else
				{
				f = dt / (last_dt + dt);
				}

			// Interpolate the temperature.

			fTemperature = 1.0E6 / (kTempTable [index - 1] . r * f +
								    kTempTable [index    ] . r * (1.0 - f));

			// Find delta from black body point to test coordinate.

			uu = u - (kTempTable [index - 1] . u * f +
					  kTempTable [index    ] . u * (1.0 - f));

			vv = v - (kTempTable [index - 1] . v * f +
					  kTempTable [index    ] . v * (1.0 - f));

			// Interpolate vectors along slope.

			du = du * (1.0 - f) + last_du * f;
			dv = dv * (1.0 - f) + last_dv * f;

			len = sqrt (du * du + dv * dv);

			du /= len;
			dv /= len;

			// Find distance along slope.

			fTint = (uu * du + vv * dv) * kTintScale;

			break;

			}

		// Try next line pair.

		last_dt = dt;

		last_du = du;
		last_dv = dv;

		}

	}

/*****************************************************************************/

dng_xy_coord dng_temperature::Get_xy_coord () const
	{

	dng_xy_coord result;

	// Find inverse temperature to use as index.

	real64 r = 1.0E6 / fTemperature;

	// Convert tint to offset is uv space.

	real64 offset = fTint * (1.0 / kTintScale);

	// Search for line pair containing coordinate.

	for (uint32 index = 0; index <= 29; index++)
		{

		if (r < kTempTable [index + 1] . r || index == 29)
			{

			// Find relative weight of first line.

			real64 f = (kTempTable [index + 1] . r - r) /
					   (kTempTable [index + 1] . r - kTempTable [index] . r);

			// Interpolate the black body coordinates.

			real64 u = kTempTable [index    ] . u * f +
					   kTempTable [index + 1] . u * (1.0 - f);

			real64 v = kTempTable [index    ] . v * f +
					   kTempTable [index + 1] . v * (1.0 - f);

			// Find vectors along slope for each line.

			real64 uu1 = 1.0;
			real64 vv1 = kTempTable [index] . t;

			real64 uu2 = 1.0;
			real64 vv2 = kTempTable [index + 1] . t;

			real64 len1 = sqrt (1.0 + vv1 * vv1);
			real64 len2 = sqrt (1.0 + vv2 * vv2);

			uu1 /= len1;
			vv1 /= len1;

			uu2 /= len2;
			vv2 /= len2;

			// Find vector from black body point.

			real64 uu3 = uu1 * f + uu2 * (1.0 - f);
			real64 vv3 = vv1 * f + vv2 * (1.0 - f);

			real64 len3 = sqrt (uu3 * uu3 + vv3 * vv3);

			uu3 /= len3;
			vv3 /= len3;

			// Adjust coordinate along this vector.

			u += uu3 * offset;
			v += vv3 * offset;

			// Convert to xy coordinates.

			result.x = 1.5 * u / (u - 4.0 * v + 2.0);
			result.y =       v / (u - 4.0 * v + 2.0);

			break;

			}

		}

	return result;

	}

/*****************************************************************************/
