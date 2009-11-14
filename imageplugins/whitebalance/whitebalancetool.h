/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-11
 * Description : a digiKam image editor plugin to correct
 *               image white balance
 *
 * Copyright (C) 2008-2009 by Guillaume Castagnino <casta at xwing dot info>
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef WHITEBALANCETOOL_H
#define WHITEBALANCETOOL_H

// Local includes

#include "editortool.h"

namespace Digikam
{
class DColor;
}

namespace DigikamWhiteBalanceImagesPlugin
{

class WhiteBalanceToolPriv;

class WhiteBalanceTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    WhiteBalanceTool(QObject* parent);
    ~WhiteBalanceTool();

private Q_SLOTS:

    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotResetSettings();
    void slotEffect();
    void slotColorSelectedFromOriginal(const Digikam::DColor& color);
    void slotColorSelectedFromTarget(const Digikam::DColor& color);
    void slotTemperatureChanged(double temperature);
    void slotTemperaturePresetChanged(int tempPreset);
    void slotAutoAdjustExposure();
    void slotPickerColorButtonActived();

private:

    void readSettings();
    void writeSettings();
    void finalRendering();
    void blockWidgetSignals(bool b);

private:

    WhiteBalanceToolPriv* const d;
};

}  // namespace DigikamWhiteBalanceImagesPlugin

#endif /* WHITEBALANCETOOL_H */
