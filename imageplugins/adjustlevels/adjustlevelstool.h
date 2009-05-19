/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-20
 * Description : image histogram adjust levels.
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ADJUSTLEVELSTOOL_H
#define ADJUSTLEVELSTOOL_H

// Local includes

#include "editortool.h"

namespace Digikam
{
class DColor;
}

namespace DigikamAdjustLevelsImagesPlugin
{

class AdjustLevelsToolPriv;

class AdjustLevelsTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    AdjustLevelsTool(QObject *parent);
    ~AdjustLevelsTool();

private:

    void readSettings();
    void writeSettings();
    void finalRendering();
    void adjustSliders(int minIn, double gamIn, int maxIn, int minOut, int maxOut);
    bool eventFilter(QObject*, QEvent*);

private Q_SLOTS:

    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotResetSettings();
    void slotEffect();
    void slotResetCurrentChannel();
    void slotAutoLevels();
    void slotChannelChanged();
    void slotScaleChanged();
    void slotAdjustSliders();
    void slotGammaInputchanged(double val);
    void slotAdjustMinInputSpinBox(double val);
    void slotAdjustMaxInputSpinBox(double val);
    void slotAdjustMinOutputSpinBox(double val);
    void slotAdjustMaxOutputSpinBox(double val);
    void slotSpotColorChanged(const Digikam::DColor& color);
    void slotColorSelectedFromTarget(const Digikam::DColor& color);
    void slotPickerColorButtonActived();
    void slotShowInputHistogramGuide(double v);
    void slotShowOutputHistogramGuide(double v);

private:

    enum ColorPicker
    {
        BlackTonal=0,
        GrayTonal,
        WhiteTonal
    };

private:

    AdjustLevelsToolPriv* const d;
};

}  // namespace DigikamAdjustLevelsImagesPlugin

#endif /* ADJUSTLEVELSTOOL_H */
