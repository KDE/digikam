/*****************************************************************************/
// Copyright 2006 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_temperature.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

#ifndef __dng_temperature__
#define __dng_temperature__

/*****************************************************************************/

#include "dng_classes.h"
#include "dng_types.h"

/*****************************************************************************/

class dng_temperature
	{

	private:

		real64 fTemperature;

		real64 fTint;

	public:

		dng_temperature ()

			:	fTemperature (0.0)
			,	fTint        (0.0)

			{
			}

		dng_temperature (real64 temperature,
						 real64 tint)

			:	fTemperature (temperature)
			,	fTint        (tint       )

			{

			}

		dng_temperature (const dng_xy_coord &xy)

			:	fTemperature (0.0)
			,	fTint        (0.0)

			{
			Set_xy_coord (xy);
			}

		void SetTemperature (real64 temperature)
			{
			fTemperature = temperature;
			}

		real64 Temperature () const
			{
			return fTemperature;
			}

		void SetTint (real64 tint)
			{
			fTint = tint;
			}

		real64 Tint () const
			{
			return fTint;
			}

		void Set_xy_coord (const dng_xy_coord &xy);

		dng_xy_coord Get_xy_coord () const;

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
