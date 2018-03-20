/*****************************************************************************/
// Copyright 2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_hue_sat_map.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_hue_sat_map__
#define __dng_hue_sat_map__

/*****************************************************************************/

#include "dng_classes.h"
#include "dng_memory.h"
#include "dng_types.h"

/*****************************************************************************/

class dng_hue_sat_map
	{

	public:

		struct HSBModify
			{
			real32 fHueShift;
			real32 fSatScale;
			real32 fValScale;
			};

	private:

		uint32 fHueDivisions;
		uint32 fSatDivisions;
		uint32 fValDivisions;

		uint32 fHueStep;
		uint32 fValStep;

		dng_memory_data fDeltas;

	public:

		dng_hue_sat_map ();

		dng_hue_sat_map (const dng_hue_sat_map &src);

		dng_hue_sat_map & operator= (const dng_hue_sat_map &rhs);

		virtual ~dng_hue_sat_map ();

		bool IsNull () const
			{
			return !IsValid ();
			}

		bool IsValid () const
			{

			return fHueDivisions > 0 &&
				   fSatDivisions > 1 &&
				   fValDivisions > 0 &&
				   fDeltas.Buffer ();

			}

		void SetInvalid ()
			{

			fHueDivisions = 0;
			fSatDivisions = 0;
			fValDivisions = 0;

			fHueStep = 0;
			fValStep = 0;

			fDeltas.Clear ();

			}

		void GetDivisions (uint32 &hueDivisions,
						   uint32 &satDivisions,
						   uint32 &valDivisions) const
			{
			hueDivisions = fHueDivisions;
			satDivisions = fSatDivisions;
			valDivisions = fValDivisions;
			}

		void SetDivisions (uint32 hueDivisions,
						   uint32 satDivisions,
						   uint32 valDivisions = 1);

		void GetDelta (uint32 hueDiv,
					   uint32 satDiv,
					   uint32 valDiv,
					   HSBModify &modify) const;

		void SetDelta (uint32 hueDiv,
					   uint32 satDiv,
					   uint32 valDiv,
					   const HSBModify &modify);

		uint32 DeltasCount () const
			{
			return fValDivisions *
				   fHueDivisions *
				   fSatDivisions;
			}

		HSBModify *GetDeltas ()
			{
			return (HSBModify *) fDeltas.Buffer_real32 ();
			}

		const HSBModify *GetDeltas () const
			{
			return (HSBModify *) fDeltas.Buffer_real32 ();
			}

                bool operator== (const dng_hue_sat_map &rhs) const;

		static dng_hue_sat_map * Interpolate (const dng_hue_sat_map &map1,
											  const dng_hue_sat_map &map2,
											  real64 weight1);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
