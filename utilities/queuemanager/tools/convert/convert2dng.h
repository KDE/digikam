/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-07-18
 * Description : DNG Raw Converter batch tool.
 *
 * Copyright (C) 2015-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CONVERT2DNG_H
#define CONVERT2DNG_H

// Local includes

#include "dngsettings.h"
#include "dngwriter.h"
#include "batchtool.h"

namespace Digikam
{

class DNGSettings;

class Convert2DNG : public BatchTool
{
    Q_OBJECT

public:

    explicit Convert2DNG(QObject* const parent = 0);
    ~Convert2DNG();

    void cancel();
    QString outputSuffix() const;
    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=0) const { return new Convert2DNG(parent); };

    void registerSettingsWidget();

private Q_SLOTS:

    void slotSettingsChanged();
    void slotAssignSettings2Widget();

private:

    bool toolOperations();

private:

    DNGSettings* m_settings;
    DNGWriter    m_dngProcessor;
    bool         m_changeSettings;
};

}  // namespace Digikam

#endif /* CONVERT2DNG_H */
