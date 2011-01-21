/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Wavelets Noise Reduction batch tool.
 *
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef NOISEREDUCTION_H
#define NOISEREDUCTION_H

// Local includes

#include "batchtool.h"


namespace Digikam
{

class NRSettings;

class NoiseReduction : public BatchTool
{
    Q_OBJECT

public:

    NoiseReduction(QObject* parent=0);
    ~NoiseReduction();

    BatchToolSettings defaultSettings();

private:

    bool toolOperations();
    QWidget* createSettingsWidget();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

private:

    NRSettings* m_settingsView;
};

}  // namespace Digikam

#endif /* NOISEREDUCTION_H */
