/* ============================================================
 * File  : imageeffect_blur.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-09
 * Description : Blur image filter for ImageEditor
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

#ifndef IMAGEEFFECT_BLUR_H
#define IMAGEEFFECT_BLUR_H

// KDE include.

#include <kdialogbase.h>

class KIntNumInput;

namespace Digikam
{
class ImageWidget;
}

class ImageEffect_Blur : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_Blur(QWidget *parent);
    ~ImageEffect_Blur();

private:

    KIntNumInput *m_radiusInput;
    Digikam::ImageWidget *m_previewWidget;
    
    void blur(uint* data, int w, int h, int r);
    
private slots:

    void slotEffect();
    void slotOk();
};

#endif /* IMAGEEFFECT_BLUR_H */
