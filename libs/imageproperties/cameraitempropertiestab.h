/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-08
 * Description : A tab to display camera item informations
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

#ifndef CAMERAITEMPROPERTIESTAB_H
#define CAMERAITEMPROPERTIESTAB_H

// Qt includes.

#include <qwidget.h>
#include <qstring.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class GPItemInfo;
class CameraItemPropertiesTabPriv;

class DIGIKAM_EXPORT CameraItemPropertiesTab : public QWidget
{
    Q_OBJECT

public:

    CameraItemPropertiesTab(QWidget* parent, bool navBar=true);
    ~CameraItemPropertiesTab();

    void setCurrentItem(const GPItemInfo* itemInfo=0, int itemType=0,
                        const QString &newFileName=QString::null,
                        const QByteArray& exifData=QByteArray());

signals:
    
    void signalFirstItem(void);    
    void signalPrevItem(void);    
    void signalNextItem(void);    
    void signalLastItem(void); 
        
private:

    CameraItemPropertiesTabPriv* d;
};

}  // NameSpace Digikam

#endif /* CAMERAITEMPROPERTIESTAB_H */
