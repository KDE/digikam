/* ============================================================
 * File  : imageeffect_cmy.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-16
 * Description : CMY adjustement plugin for ImageEditor
 * 
 * Copyright 2004 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef IMAGEEFFECT_CMY_H
#define IMAGEEFFECT_CMY_H

// KDE includes.

#include <kdialogbase.h>

class KIntNumInput;

namespace Digikam
{
class ImageWidget;
}

class ImageEffect_CMY : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_CMY(QWidget *parent);
    ~ImageEffect_CMY();

private:

    KIntNumInput         *m_cInput;
    KIntNumInput         *m_mInput;
    KIntNumInput         *m_yInput;
    Digikam::ImageWidget *m_previewWidget;

    void adjustCMY(int cy, int ma, int ye, int al, uint *data, int w, int h);
        
private slots:

    void slotUser1();
    void slotEffect();
    void slotOk();
};

#endif /* IMAGEEFFECT_HSL_H */
