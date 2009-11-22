/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-22
 * Description : noise reduction settings view.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef NOISEREDUCTIONSETTINGS_H
#define NOISEREDUCTIONSETTINGS_H

// Local includes

#include <QWidget>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

using namespace KDcrawIface;

namespace DigikamImagesPluginCore
{

class NoiseReductionSettingsPriv;

class NoiseReductionSettings : public QWidget
{
    Q_OBJECT

public:

    NoiseReductionSettings(QWidget* parent);
    ~NoiseReductionSettings();

    RDoubleNumInput* thresholdInput();
    RDoubleNumInput* softnessInput();

private:

    NoiseReductionSettingsPriv* const d;
};

}  // namespace DigikamImagesPluginCore

#endif /* NOISEREDUCTIONTOOL_H */
