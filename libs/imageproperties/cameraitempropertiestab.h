/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-08
 * Description : A tab to display camera item information
 *
 * Copyright (C) 2006-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CAMERAITEMPROPERTIESTAB_H
#define CAMERAITEMPROPERTIESTAB_H

// Qt includes

#include <QString>

// KDE includes

#include <kurl.h>

// LibKDcraw includes

#include <libkdcraw/rexpanderbox.h>

// Local includes

#include "digikam_export.h"
#include "dmetadata.h"
#include "camiteminfo.h"

using namespace KDcrawIface;

namespace Digikam
{

class CameraItemPropertiesTab : public RExpanderBox
{
    Q_OBJECT

public:

    explicit CameraItemPropertiesTab(QWidget* const parent);
    ~CameraItemPropertiesTab();

    void setCurrentItem(const CamItemInfo& itemInfo=CamItemInfo(),
                        const DMetadata& meta=DMetadata());

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* CAMERAITEMPROPERTIESTAB_H */
