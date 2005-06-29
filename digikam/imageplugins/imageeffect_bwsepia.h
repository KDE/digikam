/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-06
 * Description : Black and White conversion tool.
 * 
 * Copyright 2004-2005 by Renchi Raju and Gilles Caulier
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

#ifndef IMAGEEFFECT_BWSEPIA_H
#define IMAGEEFFECT_BWSEPIA_H

// Qt Includes.

#include <qstring.h>
#include <qpixmap.h>

// KDE include.

#include <kdialogbase.h>

class QLabel;
class QComboBox;

namespace Digikam
{
class ImageWidget;
}

class ImageEffect_BWSepia : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_BWSepia(QWidget *parent);
    ~ImageEffect_BWSepia();

private:

    enum BlackWhiteConversionType
    {
    BWNeutral=0,
    BWGreenFilter,
    BWOrangeFilter,
    BWRedFilter,
    BWYellowFilter,
    BWSepia,
    BWBrown,
    BWCold,
    BWSelenium,
    BWPlatinum
    };
        
protected:

    void closeEvent(QCloseEvent *e);
    
private:

    QWidget              *m_parent;
    
    QComboBox            *m_typeCB;
    
    Digikam::ImageWidget *m_previewWidget;
    
    void blackAndWhiteConversion(uint *data, int w, int h, int type);
    QPixmap previewEffectPic(QString name);

private slots:

    void slotEffect();
    void slotOk();
};

#endif /* IMAGEEFFECT_BWSEPIA_H */
