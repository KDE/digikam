/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description : A tab to display Exif image informations
 *
 * Copyright 2004-2006 by Gilles Caulier
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

#ifndef IMAGEPROPERTIESEXIFTAB_H
#define IMAGEPROPERTIESEXIFTAB_H

// LibPNG includes.

extern "C"
{
#include <png.h>
}

// Qt includes.

#include <qwidget.h>
#include <qcstring.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class ImagePropertiesEXIFTabPriv;

class DIGIKAM_EXPORT ImagePropertiesEXIFTab : public QWidget
{
    Q_OBJECT

public:

    ImagePropertiesEXIFTab(QWidget* parent, bool navBar=true);
    ~ImagePropertiesEXIFTab();

    void setCurrentURL(const KURL& url=KURL::KURL(), int itemType=0);
    void setCurrentData(const QByteArray& data=QByteArray(), 
                        const QString& filename=QString::null, int itemType=0);

signals:
    
    void signalFirstItem(void);    
    void signalPrevItem(void);    
    void signalNextItem(void);    
    void signalLastItem(void); 
        
private slots:

    void slotLevelChanged(int);

private:

    QByteArray loadRawExifProfileFromPNG(const KURL& url);
    uchar*     readRawProfile(png_textp text, png_uint_32 *length, int ii);
    
    QByteArray loadExifWithExiv2(const KURL& url);

private:

    ImagePropertiesEXIFTabPriv* d;
};

}  // NameSpace Digikam

#endif /* IMAGEPROPERTIESEXIFTAB_H */
