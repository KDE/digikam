/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-04-07
 * Description : a tool to resize a picture
 * 
 * Copyright 2005-2007 by Gilles Caulier
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

#ifndef IMAGE_RESIZE_H
#define IMAGE_RESIZE_H

// Qt include.

#include <qstring.h>

// KDE include.

#include <kdialogbase.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class ImageResizePriv;

class DIGIKAM_EXPORT ImageResize : public KDialogBase
{
    Q_OBJECT

public:

    ImageResize(QWidget* parent);
    ~ImageResize();
       
protected:

    void closeEvent(QCloseEvent *e);
    
private:

    void customEvent(QCustomEvent *event);
    void writeUserSettings();

private slots:

    void slotOk();
    void slotCancel();
    void slotDefault();
    void slotUser2();
    void slotUser3();
    void processCImgURL(const QString&);
    void slotValuesChanged();
    void readUserSettings();        
    void slotRestorationToggled(bool);    

private:

    ImageResizePriv *d;
};
    
}  // NameSpace Digikam

#endif /* IMAGE_RESIZE_H */
