/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-16
 * Description : HSL adjustement plugin for ImageEditor
 * 
 * Copyright 2004-2005 by Gilles Caulier
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

#ifndef IMAGEEFFECT_HSL_H
#define IMAGEEFFECT_HSL_H

// KDE includes.

#include <kdialogbase.h>

class KDoubleNumInput;

namespace Digikam
{
class ImageWidget;
}

class ImageEffect_HSL : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_HSL(QWidget *parent);
    ~ImageEffect_HSL();

private:

    KDoubleNumInput      *m_hInput;
    KDoubleNumInput      *m_sInput;
    KDoubleNumInput      *m_lInput;
    
    Digikam::ImageWidget *m_previewWidget;
    
private slots:

    void slotUser1();
    void slotEffect();
    void slotOk();
};

#endif /* IMAGEEFFECT_HSL_H */
