/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-16
 * Description : 8 to 16 bits color depth converter batch tool.
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

#ifndef CONVERT8TO16_H
#define CONVERT8TO16_H

// Local includes

#include "batchtool.h"

namespace Digikam
{

class Convert8to16 : public BatchTool
{
    Q_OBJECT

public:

    explicit Convert8to16(QObject* const parent = 0);
    ~Convert8to16();

    BatchToolSettings defaultSettings()
    {
        return BatchToolSettings();
    };

    BatchTool* clone(QObject* const parent=0) const { return new Convert8to16(parent); };

private:

    bool toolOperations();

private Q_SLOTS:

    void slotAssignSettings2Widget() {};
    void slotSettingsChanged()       {};
};

}  // namespace Digikam

#endif /* CONVERT8TO16_H */
