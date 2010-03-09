/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : LDR ToneMapper <http://zynaddsubfx.sourceforge.net/other/tonemapping>.
 *
 * Copyright (C) 2009 by Nasca Octavian Paul <zynaddsubfx at yahoo dot com>
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// C++ includes.

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

// KDE includes

#include <kdebug.h>

// Local includes.

#include "tonemappingparameters.h"

#ifdef Q_CC_MSVC
	#pragma warning ( disable : 4800 )	// forcing value to bool 'true' or 'false' (performance warning)
#endif

namespace Digikam
{

class ToneMappingParametersPriv
{
public:

    ToneMappingParametersPriv()
    {
        cancel   = 0;
        data     = 0;
        callBack = 0;
    };

    /** To cancel computation from user interface.
    */
    bool*                  cancel;

    /** For progress CallBack method from User interface
     */
    ToneMappingCallbackPtr callBack;
    void*                  data;  
};  
    
ToneMappingParameters::ToneMappingParameters()
{
    d = new ToneMappingParametersPriv;
    
    info_fast_mode   = true;
    high_saturation  = 100;
    low_saturation   = 100;
    stretch_contrast = true;
    function_id      = 0;

    for (int i = 0 ; i < TONEMAPPING_MAX_STAGES ; i++)
    {
        stage[i].enabled = (i == 0);
        stage[i].power   = 30.0;
        stage[i].blur    = 80.0;
    }

    unsharp_mask.enabled   = false;
    unsharp_mask.power     = 30.0;
    unsharp_mask.blur      = 4.0;
    unsharp_mask.threshold = 0;
}

ToneMappingParameters::~ToneMappingParameters()
{
    delete d;
}

ToneMappingParameters& ToneMappingParameters::operator=(const ToneMappingParameters& prm)
{
    d = prm.d;
    return *this;
}

bool ToneMappingParameters::cancel()
{
    if (d->cancel)
        return *d->cancel;

    return false;
}

void ToneMappingParameters::setCancel(bool* b)
{
    d->cancel = b;
}

void ToneMappingParameters::setProgressCallBackFunction(void* data, ToneMappingCallbackPtr cb)
{
    d->callBack = cb;
    d->data     = data;
}

void ToneMappingParameters::postProgress(int progress)
{
    d->callBack(d->data, progress);
}

float ToneMappingParameters::get_power(int nstage)
{
    float power = stage[nstage].power;
    power          = (float)(pow(power/100.0, 1.5)*100.0);
    return power;
}

float ToneMappingParameters::get_blur(int nstage)
{
    return stage[nstage].blur;
}

float ToneMappingParameters::get_unsharp_mask_power()
{
    float power = unsharp_mask.power;
    power          = (float)(pow(power/100.0, 3.0)*10.0);
    return power;
}

float ToneMappingParameters::get_unsharp_mask_blur()
{
    return unsharp_mask.blur;
}

} // namespace Digikam
