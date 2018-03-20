/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-17
 * Description : resize image batch tool.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef RESIZE_H
#define RESIZE_H

// Local includes

#include "batchtool.h"

namespace Digikam
{

class Resize : public BatchTool
{
    Q_OBJECT

public:

    explicit Resize(QObject* const parent = 0);
    ~Resize();

    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=0) const { return new Resize(parent); };

    void registerSettingsWidget();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

private:

    bool toolOperations();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // RESIZE_H
