/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-20
 * Description : image histogram adjust levels.
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "dcolor.h"

namespace Digikam
{

class AdjustLevelsTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit AdjustLevelsTool(QObject* const parent);
    ~AdjustLevelsTool();

private Q_SLOTS:

    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotResetSettings();
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
    void slotPickerColorButtonActived(int);
    void slotShowInputHistogramGuide(double v);
    void slotShowOutputHistogramGuide(double v);

private:

    void readSettings();
    void writeSettings();
    void preparePreview();
    void prepareFinal();
    void abortPreview();
    void setPreviewImage();
    void setFinalImage();

    void adjustSliders(int minIn, double gamIn, int maxIn, int minOut, int maxOut);
    void adjustSlidersAndSpinboxes(int minIn, double gamIn, int maxIn, int minOut, int maxOut);
    bool eventFilter(QObject*, QEvent*);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* ADJUSTLEVELSTOOL_H */
