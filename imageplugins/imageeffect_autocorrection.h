/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-31
 * Description : Auto-Color correction tool.
 * 
 * Copyright 2005 by  Gilles Caulier
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

#ifndef IMAGEEFFECT_AUTOCORRECTION_H
#define IMAGEEFFECT_AUTOCORRECTION_H

// Qt Includes.

#include <qstring.h>
#include <qpixmap.h>

// KDE include.

#include <kdialogbase.h>

class QLabel;
class QComboBox;

namespace Digikam
{
class DImg;
class ImageWidget;
}

class ImageEffect_AutoCorrection : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_AutoCorrection(QWidget *parent);
    ~ImageEffect_AutoCorrection();

private:

    enum AutoCorrectionType
    {
    AutoLevelsCorrection=0,
    NormalizeCorrection,
    EqualizeCorrection,
    StretchContrastCorrection
    };
        
protected:

    void closeEvent(QCloseEvent *e);
    
private:

    QWidget              *m_parent;
    
    QComboBox            *m_typeCB;
    
    Digikam::ImageWidget *m_previewWidget;
    
    void autoCorrection(Digikam::DImg& image, int type);
    QPixmap previewEffectPic(QString name);

private slots:

    void slotEffect();
    void slotOk();
};

#endif /* IMAGEEFFECT_AUTOCORRECTION_H */
