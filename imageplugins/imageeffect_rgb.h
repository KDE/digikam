/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-11
 * Description : RGB adjustement plugin for ImageEditor
 * 
 * Copyright 2004 by Gilles Caulier
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

#ifndef IMAGEEFFECT_RGB_H
#define IMAGEEFFECT_RGB_H

// KDE includes.

#include <kdialogbase.h>
#include "digikam_export.h"

class QSpinBox;
class QSlider;

namespace Digikam
{
class ImageWidget;
}

class DIGIKAMIMAGEPLUGINS_EXPORT ImageEffect_RGB : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_RGB(QWidget *parent);
    ~ImageEffect_RGB();

private:

    QSpinBox             *m_rInput;
    QSpinBox             *m_gInput;
    QSpinBox             *m_bInput;
    
    QSlider              *m_rSlider;
    QSlider              *m_gSlider;
    QSlider              *m_bSlider;
    
    Digikam::ImageWidget *m_previewWidget;

    void adjustRGB(double r, double g, double b, double a, uint *data, int w, int h);
    void adjustSliders(int r, int g, int b);
    
private slots:

    void slotUser1();
    void slotEffect();
    void slotOk();
};

#endif /* IMAGEEFFECT_RGB_H */
