/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-09
 * Description : a tool to sharp an image
 *
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Digikam include.

#include "ctrlpaneldlg.h"

class QComboBox;
class QWidgetStack;

class KDcrawIface::RIntNumInput;
class KDcrawIface::RDoubleNumInput;

namespace Digikam
{
    class DImg;
}

namespace DigikamImagesPluginCore
{

class ImageEffect_Sharpen : public Digikam::CtrlPanelDlg
{
    Q_OBJECT

public:

    ImageEffect_Sharpen(QWidget *parent);
    ~ImageEffect_Sharpen();

private slots:

    void slotUser2();
    void slotUser3();
    void readUserSettings();
    void slotSharpMethodActived(int);

private:

    void writeUserSettings();
    void resetValues();  
    void prepareEffect();
    void prepareFinal();
    void abortPreview();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private:
    
    enum SharpingMethods 
    {
        SimpleSharp=0,
        UnsharpMask,
        Refocus
    };

    QWidgetStack    *m_stack;
    
    QComboBox       *m_sharpMethod;

    KDcrawIface::RIntNumInput    *m_matrixSize;
    KDcrawIface::RIntNumInput    *m_radiusInput;
    KDcrawIface::RIntNumInput    *m_radiusInput2;

    KDcrawIface::RDoubleNumInput *m_radius;
    KDcrawIface::RDoubleNumInput *m_gauss;
    KDcrawIface::RDoubleNumInput *m_correlation;
    KDcrawIface::RDoubleNumInput *m_noise;
    KDcrawIface::RDoubleNumInput *m_amountInput;
    KDcrawIface::RDoubleNumInput *m_thresholdInput;

    Digikam::DImg    m_img;
};

}  // NameSpace DigikamImagesPluginCore

#endif /* IMAGEEFFECT_SHARPEN_H */
