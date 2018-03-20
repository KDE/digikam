/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-14
 * Description : remove metadata batch tool.
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

#ifndef REMOVE_METADATA_H
#define REMOVE_METADATA_H

// Local includes

#include "batchtool.h"

class QCheckBox;

namespace Digikam
{

class RemoveMetadata : public BatchTool
{
    Q_OBJECT

public:

    explicit RemoveMetadata(QObject* const parent = 0);
    virtual ~RemoveMetadata();

    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=0) const { return new RemoveMetadata(parent); };

    void registerSettingsWidget();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

private:

    enum RemoveAction
    {
        Tiny = 0,
        Small,
        Medium,
        Big,
        Large,
        Huge
    };

    QCheckBox* m_removeExif;
    QCheckBox* m_removeIptc;
    QCheckBox* m_removeXmp;

private:

    bool toolOperations();
};

} // namespace Digikam

#endif // REMOVE_METADATA_H
