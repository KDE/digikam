/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-09
 * Description : Sharpen image filter for ImageEditor
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

#ifndef IMAGEEFFECT_SHARPEN_H
#define IMAGEEFFECT_SHARPEN_H

// KDE include.

#include <kdialogbase.h>

class KIntNumInput;

namespace Digikam
{
class ImagePreviewWidget;
}

class ImageEffect_Sharpen : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_Sharpen(QWidget *parent);
    ~ImageEffect_Sharpen();

private:

    QWidget      *m_parent;
    
    KIntNumInput *m_radiusInput;
    
    Digikam::ImagePreviewWidget *m_imagePreviewWidget;
    
private slots:

    void slotEffect();
    void slotOk();
};

#endif /* IMAGEEFFECT_SHARPEN_H */
