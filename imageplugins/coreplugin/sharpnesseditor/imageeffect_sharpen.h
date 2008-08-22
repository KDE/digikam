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

// Digikam includes.

#include "ctrlpaneldlg.h"

class QStackedWidget;
class QComboBox;

class KIntNumInput;
class KDoubleNumInput;

namespace KDcrawIface
{
}

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

    QStackedWidget  *m_stack;

    QComboBox       *m_sharpMethod;

    KIntNumInput    *m_matrixSize;
    KIntNumInput    *m_radiusInput;
    KIntNumInput    *m_radiusInput2;

    KDoubleNumInput *m_radius;
    KDoubleNumInput *m_gauss;
    KDoubleNumInput *m_correlation;
    KDoubleNumInput *m_noise;
    KDoubleNumInput *m_amountInput;
    KDoubleNumInput *m_thresholdInput;

    Digikam::DImg    m_img;
};

}  // NameSpace DigikamImagesPluginCore

#endif /* IMAGEEFFECT_SHARPEN_H */
