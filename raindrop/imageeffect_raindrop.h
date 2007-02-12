/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2004-09-30
 * Description : a digiKam image plugin to add
 *               raindrops on an image.
 * 
 * Copyright 2004-2005 by Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier and Marcel Wiesweg
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

#ifndef IMAGEEFFECT_RAINDROP_H
#define IMAGEEFFECT_RAINDROP_H

// Digikam includes.

#include <digikamheaders.h>

class KIntNumInput;

namespace DigikamRainDropImagesPlugin
{

class ImageEffect_RainDrop : public Digikam::ImageGuideDlg
{
    Q_OBJECT

public:

    ImageEffect_RainDrop(QWidget *parent, QString title, QFrame* banner);
    ~ImageEffect_RainDrop();

private slots:

    void readUserSettings();

private:

    void writeUserSettings();
    void resetValues();     
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private:

    KIntNumInput *m_dropInput;
    KIntNumInput *m_amountInput;
    KIntNumInput *m_coeffInput;
};

}  // NameSpace DigikamRainDropImagesPlugin

#endif /* IMAGEEFFECT_RAINDROP_H */
