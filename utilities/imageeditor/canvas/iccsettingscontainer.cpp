/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-12-08
 * Description : ICC Settings Container.
 *
 * Copyright (C) 2005-2007 by F.J. Cruz <fj.cruz@supercable.es>
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "iccsettingscontainer.h"

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "icctransform.h"
#include "iccprofile.h"

namespace Digikam
{

ICCSettingsContainer::ICCSettingsContainer()
{
    // Note: by default, ICC color management is disabled.
    enableCM                = false;

    onProfileMismatch       = Convert;
    useBPC                  = false;
    useManagedView          = false;

    renderingIntent         = IccTransform::Perceptual;
}

void ICCSettingsContainer::readFromConfig(KConfigGroup& group)
{
    enableCM             = group.readEntry("EnableCM", false);

    QString behavior = group.readEntry("OnProfileMismatch", "ask");
    if (!group.hasKey("OnProfileMismatch") && group.hasKey("BehaviourICC")) // legacy
        behavior = group.readEntry("BehaviourICC", false) ? "convert" : "ask";

    if (behavior == "convert")
        onProfileMismatch = ICCSettingsContainer::Convert;
    else if (behavior == "leave")
        onProfileMismatch = ICCSettingsContainer::Leave;
    else
        onProfileMismatch = ICCSettingsContainer::Ask;

    useBPC               = group.readEntry("BPCAlgorithm",false);
    useManagedView       = group.readEntry("ManagedView", false);
    renderingIntent      = group.readEntry("RenderingIntent", (int)IccTransform::Perceptual);
    workspaceProfile     = group.readPathEntry("WorkProfileFile", IccProfile::sRGB().filePath());
    monitorProfile       = group.readPathEntry("MonitorProfileFile", IccProfile::sRGB().filePath());
    defaultInputProfile  = group.readPathEntry("InProfileFile", QString());
    defaultProofProfile  = group.readPathEntry("ProofProfileFile", QString());
    iccFolder            = group.readEntry("DefaultPath", QString());
}

void ICCSettingsContainer::writeToConfig(KConfigGroup& group)
{
    group.writeEntry("EnableCM", enableCM);

    if (!enableCM)
        return;          // No need to write settings in this case.

    if (onProfileMismatch == ICCSettingsContainer::Convert)
        group.writeEntry("OnProfileMismatch", "convert");
    else if (onProfileMismatch == ICCSettingsContainer::Leave)
        group.writeEntry("OnProfileMismatch", "leave");
    else
        group.writeEntry("OnProfileMismatch", "ask");

    group.writeEntry("BPCAlgorithm", useBPC);
    group.writeEntry("ManagedView", useManagedView);
    group.writeEntry("RenderingIntent", renderingIntent);

    group.writePathEntry("WorkProfileFile", workspaceProfile);
    group.writePathEntry("MonitorProfileFile", monitorProfile);
    group.writePathEntry("InProfileFile", defaultInputProfile);
    group.writePathEntry("ProofProfileFile", defaultProofProfile);
    group.writeEntry("DefaultPath", iccFolder);
}

void ICCSettingsContainer::writeManagedViewToConfig(KConfigGroup& group)
{
    group.writeEntry("ManagedView", useManagedView);
}

}  // namespace Digikam

