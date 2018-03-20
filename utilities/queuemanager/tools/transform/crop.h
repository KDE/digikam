/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-12-28
 * Description : crop image batch tool.
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CROP_H
#define CROP_H

// Local includes

#include "batchtool.h"

namespace Digikam
{

class Crop : public BatchTool
{
    Q_OBJECT

public:

    explicit Crop(QObject* const parent = 0);
    ~Crop();

    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=0) const { return new Crop(parent); };

    void registerSettingsWidget();

Q_SIGNALS:

    void signalAutoCrop();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();
    void slotDisableParameters(bool);

private:

    bool toolOperations();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // CROP_H
