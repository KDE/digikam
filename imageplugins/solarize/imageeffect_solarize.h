/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2004-02-14
 * Description : a digiKam image plugin for to solarize
 *               an image.
 *
 * Copyright 2004-2005 by Renchi Raju
 * Copyright 2006-2007 by Gilles Caulier
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

#ifndef IMAGEEFFECT_SOLARIZE_H
#define IMAGEEFFECT_SOLARIZE_H

// Digikam includes.

#include <digikamheaders.h>

class QPushButton;

class KDoubleNumInput;

namespace Digikam
{
class ImageWidget;
}

namespace DigikamSolarizeImagesPlugin
{

class ImageEffect_Solarize : public Digikam::ImageDlgBase
{
    Q_OBJECT

public:

    ImageEffect_Solarize(QWidget *parent, QString title, QFrame* banner);
    ~ImageEffect_Solarize();

private:

    void readUserSettings();
    void writeUserSettings();
    void resetValues();
    void solarize(double factor, uchar *data, int w, int h, bool sb);
    void finalRendering();

private slots:

    void slotEffect();

private:

    KDoubleNumInput      *m_numInput;
    
    Digikam::ImageWidget *m_previewWidget;
};

}  // NameSpace DigikamSolarizeImagesPlugin

#endif /* IMAGEEFFECT_SOLARIZE_H */
