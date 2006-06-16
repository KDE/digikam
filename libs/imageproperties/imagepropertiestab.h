/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2006-04-19
 * Description : A tab to display general image informations
 *
 * Copyright 2006 by Gilles Caulier
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

#include <qwidget.h>
#include <qstring.h>
#include <qcolor.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class ImagePropertiesTabPriv;

class DIGIKAM_EXPORT ImagePropertiesTab : public QWidget
{
    Q_OBJECT

public:

    ImagePropertiesTab(QWidget* parent, bool navBar=true);
    ~ImagePropertiesTab();

    void setCurrentURL(const KURL& url=KURL(), int itemType=0);
    void colorChanged(const QColor& back, const QColor& fore);

signals:

    void signalFirstItem(void);
    void signalPrevItem(void);
    void signalNextItem(void);
    void signalLastItem(void);

private:

    ImagePropertiesTabPriv* d;
};

}  // NameSpace Digikam

#endif /* IMAGEPROPERTIESTAB_H */
