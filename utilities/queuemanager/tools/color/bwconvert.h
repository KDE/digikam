/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-23
 * Description : Black and White conversion batch tool.
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

#ifndef BW_CONVERT_H
#define BW_CONVERT_H

// Local includes

#include "batchtool.h"

namespace Digikam
{

class BWSepiaSettings;

class BWConvert : public BatchTool
{
    Q_OBJECT

public:

    explicit BWConvert(QObject* const parent = 0);
    ~BWConvert();

    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=0) const { return new BWConvert(parent); };

    void registerSettingsWidget();

public Q_SLOTS:

    void slotResetSettingsToDefault();

private:

    bool toolOperations();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

private:

    DImg             m_preview;
    BWSepiaSettings* m_settingsView;
};

}  // namespace Digikam

#endif // BW_CONVERT_H
