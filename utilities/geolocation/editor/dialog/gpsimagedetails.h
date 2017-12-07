/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-06
 * Description : A widget to show details about images
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef GPSIMAGEDETAILS_H
#define GPSIMAGEDETAILS_H

// Qt includes

#include <QUrl>
#include <QModelIndex>
#include <QWidget>

// local includes

class KConfigGroup;

namespace Digikam
{

class GPSImageModel;
class GPSUndoCommand;
class GPSDataContainer;

class GPSImageDetails : public QWidget
{
    Q_OBJECT

public:

    GPSImageDetails(QWidget* const parent, GPSImageModel* const imageModel);
    ~GPSImageDetails();

    void setUIEnabledExternal(const bool state);
    void saveSettingsToGroup(KConfigGroup* const group);
    void readSettingsFromGroup(const KConfigGroup* const group);

Q_SIGNALS:

    void signalUndoCommand(GPSUndoCommand* undoCommand);

public Q_SLOTS:

    void slotSetCurrentImage(const QModelIndex& index);
    void slotSetActive(const bool state);

private Q_SLOTS:

    void updateUIState();
    void slotModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void slotApply();

protected:

    void displayGPSDataContainer(const GPSDataContainer* const gpsData);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // GPSIMAGEDETAILS_H
