/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-24
 * Description : a plugin to reduce CCD noise.
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

#ifndef IMAGEEFFECT_NOISEREDUCTION_H
#define IMAGEEFFECT_NOISEREDUCTION_H

// Local includes.

#include "ctrlpaneldlg.h"


namespace KDcrawIface
{
class RDoubleNumInput;
}

namespace DigikamNoiseReductionImagesPlugin
{

class ImageEffect_NoiseReduction : public Digikam::CtrlPanelDlg
{
    Q_OBJECT

public:

    ImageEffect_NoiseReduction(QWidget* parent);
    ~ImageEffect_NoiseReduction();

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

private slots:

    void slotUser2();
    void slotUser3();

private:

    KDcrawIface::RDoubleNumInput *m_radiusInput;
    KDcrawIface::RDoubleNumInput *m_lumToleranceInput;
    KDcrawIface::RDoubleNumInput *m_thresholdInput;
    KDcrawIface::RDoubleNumInput *m_textureInput;
    KDcrawIface::RDoubleNumInput *m_sharpnessInput;

    KDcrawIface::RDoubleNumInput *m_csmoothInput;
    KDcrawIface::RDoubleNumInput *m_lookaheadInput;
    KDcrawIface::RDoubleNumInput *m_gammaInput;
    KDcrawIface::RDoubleNumInput *m_dampingInput;
    KDcrawIface::RDoubleNumInput *m_phaseInput;
};

}  // NameSpace DigikamNoiseReductionImagesPlugin

#endif /* IMAGEEFFECT_NOISEREDUCTION_H */
