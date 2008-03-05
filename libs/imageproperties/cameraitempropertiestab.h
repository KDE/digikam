/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-08
 * Description : A tab to display camera item information
 *
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qwidget.h>
#include <qstring.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "digikam_export.h"
#include "navigatebartab.h"

namespace Digikam
{

class GPItemInfo;
class CameraItemPropertiesTabPriv;

class DIGIKAM_EXPORT CameraItemPropertiesTab : public NavigateBarTab
{
    Q_OBJECT

public:

    CameraItemPropertiesTab(QWidget* parent, bool navBar=true);
    ~CameraItemPropertiesTab();

    void setCurrentItem(const GPItemInfo* itemInfo=0,
                        const QString &newFileName=QString(),
                        const QByteArray& exifData=QByteArray(),
                        const KURL &currentURL = KURL());

private:

    CameraItemPropertiesTabPriv* d;
};

}  // NameSpace Digikam

#endif /* CAMERAITEMPROPERTIESTAB_H */
