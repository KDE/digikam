/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-20
 * Description : Metadata Settings Container.
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metadatasettingscontainer.h"

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "metadatasettings.h"

namespace Digikam
{

MetadataSettingsContainer::MetadataSettingsContainer()
{
    UseXMPSidecar = false;
}

void MetadataSettingsContainer::readFromConfig(KConfigGroup& group)
{
    UseXMPSidecar = group.readEntry("Use XMP Sidecar", false);
}

void MetadataSettingsContainer::writeToConfig(KConfigGroup& group) const
{
    group.writeEntry("Use XMP Sidecar", UseXMPSidecar);
}

}  // namespace Digikam
