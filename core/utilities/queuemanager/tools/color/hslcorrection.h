/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-11
 * Description : Hue/Saturation/Lightness batch tool.
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

#ifndef HSLCORRECTION_H
#define HSLCORRECTION_H

// Local includes

#include "batchtool.h"

namespace Digikam
{

class HSLSettings;

class HSLCorrection : public BatchTool
{
    Q_OBJECT

public:

    explicit HSLCorrection(QObject* const parent = 0);
    ~HSLCorrection();

    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=0) const { return new HSLCorrection(parent); };

    void registerSettingsWidget();

private:

    bool toolOperations();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

private:

    HSLSettings* m_settingsView;
};

}  // namespace Digikam

#endif /* HSLCORRECTION_H */
