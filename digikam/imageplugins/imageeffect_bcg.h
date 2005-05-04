/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-05
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju
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

#ifndef IMAGEEFFECT_BCG_H
#define IMAGEEFFECT_BCG_H

// KDE includes.

#include <kdialogbase.h>

class KDoubleNumInput;

namespace Digikam
{
class ImageWidget;
}

class ImageEffect_BCG : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_BCG(QWidget *parent);
    ~ImageEffect_BCG();

private:

    KDoubleNumInput      *m_bInput;
    KDoubleNumInput      *m_cInput;
    KDoubleNumInput      *m_gInput;
    Digikam::ImageWidget *m_previewWidget;

private slots:

    void slotUser1();
    void slotEffect();
    void slotOk();
};

#endif /* IMAGEEFFECT_BCG_H */
