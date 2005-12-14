/* ============================================================
 * File  : imageeffect_solarize.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-02-14
 * Description : a digiKam image plugin for to solarize
 *               an image.
 *
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
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

// KDE includes.

#include <kdialogbase.h>

// Digikam includes.

#include <dimg.h>

class QPushButton;

class KDoubleNumInput;
class KAboutData;

namespace Digikam
{
class ImageWidget;
}

namespace DigikamSolarizeImagesPlugin
{

class ImageEffect_Solarize : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_Solarize(QWidget *parent);
    ~ImageEffect_Solarize();

protected:

    void solarize(double factor, uchar *data, int w, int h, bool sb);

private:

    QWidget              *m_parent;

    QPushButton          *m_helpButton;

    KDoubleNumInput      *m_numInput;
    
    KAboutData           *m_about;
    
    Digikam::ImageWidget *m_previewWidget;

private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
};

}  // NameSpace DigikamSolarizeImagesPlugin

#endif /* IMAGEEFFECT_SOLARIZE_H */
