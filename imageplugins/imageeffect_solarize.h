/* ============================================================
 * File  : imageeffect_solarize.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-02-14
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#ifndef IMAGEEFFECT_SOLARIZE_H
#define IMAGEEFFECT_SOLARIZE_H

#include <kdialogbase.h>

class KDoubleNumInput;

namespace Digikam
{
class ImageWidget;
}

class ImageEffect_Solarize : public KDialogBase
{
    Q_OBJECT
    
public:

    ImageEffect_Solarize(QWidget *parent);
    ~ImageEffect_Solarize();

protected:

    void solarize(double factor, uint *data, int w, int h);
    
    void closeEvent(QCloseEvent *e);
    
private:
    
    KDoubleNumInput      *m_numInput;
    Digikam::ImageWidget *m_previewWidget;

private slots:

    void slotEffect();
    void slotOk();
};

#endif /* IMAGEEFFECT_SOLARIZE_H */
