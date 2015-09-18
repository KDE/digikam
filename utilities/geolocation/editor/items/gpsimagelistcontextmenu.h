/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-05-07
 * @brief  Context menu for GPS list view.
 *
 * @author Copyright (C) 2009-2011 by Michael G. Hansen
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

#ifndef GPSIMAGELISTCONTEXTMENU_H
#define GPSIMAGELISTCONTEXTMENU_H

// Qt includes:

#include <QObject>

// libkgeomap includes

#include <KGeoMap/LookupAltitude>

// local includes:

#include "gpsdatacontainer.h"
#include "gpsimagelist.h"

class QUrl;

namespace Digikam
{

class GPSBookmarkOwner;
class GPSUndoCommand;

class GPSImageListContextMenu : public QObject
{
    Q_OBJECT

public:

    explicit GPSImageListContextMenu(GPSImageList* const imagesList, GPSBookmarkOwner* const bookmarkOwner = 0);
    ~GPSImageListContextMenu();

    void setEnabled(const bool state);

protected:

    void setGPSDataForSelectedItems(const GPSDataContainer& gpsData, const QString& undoDescription);
    bool getCurrentItemPositionAndUrl(GPSDataContainer* const gpsInfo, QUrl* const itemUrl);
    void removeInformationFromSelectedImages(const GPSDataContainer::HasFlags flagsToClear, const QString& undoDescription);

    virtual bool eventFilter(QObject* watched, QEvent* event);

    static bool getCurrentPosition(GPSDataContainer* position, void* mydata);

private Q_SLOTS:

    void copyActionTriggered();
    void pasteActionTriggered();
    void slotBookmarkSelected(GPSDataContainer bookmarkPosition);
    void slotRemoveCoordinates();
    void slotRemoveAltitude();
    void slotRemoveUncertainty();
    void slotRemoveSpeed();
    void slotLookupMissingAltitudes();
    void slotAltitudeLookupReady(const QList<int>& readyRequests);
    void slotAltitudeLookupDone();
    void slotAltitudeLookupCancel();

Q_SIGNALS:

    void signalSetUIEnabled(const bool enabledState);
    void signalSetUIEnabled(const bool enabledState, QObject* const cancelObject, const QString& cancelSlot);
    void signalProgressSetup(const int maxProgress, const QString& progressText);
    void signalProgressChanged(const int currentProgress);
    void signalUndoCommand(GPSUndoCommand* undoCommand);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* GPSIMAGELISTCONTEXTMENU_H */
