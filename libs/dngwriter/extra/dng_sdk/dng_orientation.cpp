/*****************************************************************************/
// Copyright 2006 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_orientation.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

#include "dng_orientation.h"

/*****************************************************************************/

void dng_orientation::SetTIFF (uint32 tiff)
	{

	switch (tiff)
		{

		case 1:
			{
			fAdobeOrientation = kNormal;
			break;
			}

		case 2:
			{
			fAdobeOrientation = kMirror;
			break;
			}

		case 3:
			{
			fAdobeOrientation = kRotate180;
			break;
			}

		case 4:
			{
			fAdobeOrientation = kMirror180;
			break;
			}

		case 5:
			{
			fAdobeOrientation = kMirror90CCW;
			break;
			}

		case 6:
			{
			fAdobeOrientation = kRotate90CW;
			break;
			}

		case 7:
			{
			fAdobeOrientation = kMirror90CW;
			break;
			}

		case 8:
			{
			fAdobeOrientation = kRotate90CCW;
			break;
			}

		case 9:
			{
			fAdobeOrientation = kUnknown;
			break;
			}

		default:
			{
			fAdobeOrientation = kNormal;
			break;
			}

		}

	}

/*****************************************************************************/

uint32 dng_orientation::GetTIFF () const
	{

	switch (fAdobeOrientation)
		{

		case kNormal:
			{
			return 1;
			}

		case kMirror:
			{
			return 2;
			}

		case kRotate180:
			{
			return 3;
			}

		case kMirror180:
			{
			return 4;
			}

		case kMirror90CCW:
			{
			return 5;
			}

		case kRotate90CW:
			{
			return 6;
			}

		case kMirror90CW:
			{
			return 7;
			}

		case kRotate90CCW:
			{
			return 8;
			}

		case kUnknown:
			{
			return 9;
			}

		default:
			break;

		}

	return 1;

	}

/*****************************************************************************/

bool dng_orientation::FlipD () const
	{

	return (fAdobeOrientation & 1) != 0;

	}

/*****************************************************************************/

bool dng_orientation::FlipH () const
	{

	if (fAdobeOrientation & 4)
		return (fAdobeOrientation & 2) == 0;

	else
		return (fAdobeOrientation & 2) != 0;

	}

/*****************************************************************************/

bool dng_orientation::FlipV () const
	{

	if (fAdobeOrientation & 4)
		return FlipD () == FlipH ();

	else
		return FlipD () != FlipH ();

	}

/*****************************************************************************/

dng_orientation dng_orientation::operator- () const
	{

	uint32 x = GetAdobe ();

	if ((x & 5) == 5)
		{

		x ^= 2;

		}

	dng_orientation result;

	result.SetAdobe (((4 - x) & 3) | (x & 4));

	return result;

	}

/*****************************************************************************/

dng_orientation dng_orientation::operator+ (const dng_orientation &b) const
	{

	uint32 x = GetAdobe ();

	uint32 y = b.GetAdobe ();

	if (y & 4)
		{

		if (x & 1)
			x ^= 6;
		else
			x ^= 4;

		}

	dng_orientation result;

	result.SetAdobe (((x + y) & 3) | (x & 4));

	return result;

	}

/*****************************************************************************/
