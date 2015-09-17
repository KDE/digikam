/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-08-06
 * @brief  A widget to show details about images
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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

class KipiImageModel;
class GPSUndoCommand;
class GPSDataContainer;

class GPSImageDetails : public QWidget
{
    Q_OBJECT

public:

    GPSImageDetails(QWidget* const parent, KipiImageModel* const imageModel);
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

#endif /* GPSIMAGEDETAILS_H */
