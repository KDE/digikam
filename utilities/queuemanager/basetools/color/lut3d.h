/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-10-10
 * Description : Lut3D color adjustment tool.
 *
 * Copyright (C) 2015 by Andrej Krutak <dev at andree dot sk>
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

#ifndef LUT3D_H
#define LUT3D_H

// Local includes

#include "batchtool.h"

namespace Digikam
{

class Lut3DSettings;

class Lut3D : public BatchTool
{
    Q_OBJECT

public:

    explicit Lut3D(QObject* const parent = 0);
    ~Lut3D();

    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=0) const { return new Lut3D(parent); };

    void registerSettingsWidget();

public Q_SLOTS:

    void slotResetSettingsToDefault();

private:

    bool toolOperations();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

private:

    DImg           m_preview;
    Lut3DSettings* m_settingsView;
};

}  // namespace Digikam

#endif /* LUT3D_H */
