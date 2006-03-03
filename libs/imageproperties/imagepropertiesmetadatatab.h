/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2004-11-17
 * Description : A tab to display metadata image informations
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

#ifndef IMAGEPROPERTIESMETADATATAB_H
#define IMAGEPROPERTIESMETADATATAB_H

// Qt includes.

#include <qwidget.h>
#include <qcstring.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class ImagePropertiesMetadataTabPriv;

class DIGIKAM_EXPORT ImagePropertiesMetaDataTab : public QWidget
{
    Q_OBJECT

public:

    ImagePropertiesMetaDataTab(QWidget* parent, bool navBar=true);
    ~ImagePropertiesMetaDataTab();

    void setCurrentURL(const KURL& url=KURL::KURL(), int itemType=0);
    void setCurrentData(const QByteArray& exifData=QByteArray(), 
                        const QByteArray& iptcData=QByteArray(), 
                        const QString& filename=QString::null, int itemType=0);

signals:
    
    void signalFirstItem(void);    
    void signalPrevItem(void);    
    void signalNextItem(void);    
    void signalLastItem(void); 
        
private:

//    QByteArray loadExifWithExiv2(const KURL& url);

private:

    ImagePropertiesMetadataTabPriv* d;
};

}  // NameSpace Digikam

#endif /* IMAGEPROPERTIESMETADATATAB_H */
