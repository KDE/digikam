/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-19
 * Description : A tab to display general image information
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

#ifndef IMAGEPROPERTIESTAB_H
#define IMAGEPROPERTIESTAB_H

// Qt includes.

#include <QFrame>
#include <QString>
#include <QColor>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class ImagePropertiesTabPriv;

class DIGIKAM_EXPORT ImagePropertiesTab : public QFrame
{
    Q_OBJECT

public:

    ImagePropertiesTab(QWidget* parent);
    ~ImagePropertiesTab();

    void setCurrentURL(const KUrl& url=KUrl());
    void colorChanged(const QColor& back, const QColor& fore);

private:

    ImagePropertiesTabPriv* d;
};

}  // NameSpace Digikam

#endif /* IMAGEPROPERTIESTAB_H */
