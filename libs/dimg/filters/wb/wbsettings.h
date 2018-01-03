/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-26
 * Description : White Balance settings view.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef WBSETTINGS_H
#define WBSETTINGS_H

// Local includes

#include <QWidget>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_export.h"
#include "wbfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT WBSettings : public QWidget
{
    Q_OBJECT

public:

    explicit WBSettings(QWidget* const parent);
    ~WBSettings();

    WBContainer defaultSettings() const;
    void resetToDefault();

    WBContainer settings() const;
    void setSettings(const WBContainer& settings);

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void loadSettings();
    void saveAsSettings();

    bool pickTemperatureIsOn();
    void setOnPickTemperature(bool b);

    void showAdvancedButtons(bool b);

Q_SIGNALS:

    void signalSettingsChanged();
    void signalPickerColorButtonActived();
    void signalAutoAdjustExposure();

private Q_SLOTS:

    void slotTemperatureChanged(double temperature);
    void slotTemperaturePresetChanged(int tempPreset);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* WBSETTINGS_H */
