/* ============================================================
 * Authors: F.J. Cruz <fj.cruz@supercable.es>
 * Date   : 2005-12-08
 * Description : ICC Settings Container.
 * 
 * Copyright 2005-2007 by  F.J. Cruz
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

#ifndef ICCSETTINGSCONTAINER_H
#define ICCSETTINGSCONTAINER_H

// Qt includes.

#include <qstring.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ICCSettingsContainer
{

public:
    
    ICCSettingsContainer()
    {
        enableCMSetting         = false;  // NOTE: by default, ICC color management is disable.

        askOrApplySetting       = false;
        BPCSetting              = false;
        managedViewSetting      = false;
        CMInRawLoadingSetting   = false;

        renderingSetting        = 0;

        workspaceSetting        = QString();
        monitorSetting          = QString();
        inputSetting            = QString();
        proofSetting            = QString();
    };
    
    ~ICCSettingsContainer(){};

public:

    bool    enableCMSetting;
    
    // FALSE -> apply
    // TRUE  -> ask
    bool    askOrApplySetting;
    bool    BPCSetting;
    bool    managedViewSetting;
    bool    CMInRawLoadingSetting;

    int     renderingSetting;

    QString workspaceSetting;
    QString monitorSetting;
    QString inputSetting;
    QString proofSetting;
};

}  // namespace Digikam

#endif  // ICCSETTINGSCONTAINER_H
