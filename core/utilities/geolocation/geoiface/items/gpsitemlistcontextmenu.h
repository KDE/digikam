/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-07
 * Description : Context menu for GPS list view.
 *
 * Copyright (C) 2010-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2014 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef DIGIKAM_GPS_ITEM_LIST_CONTEXT_MENU_H
#define DIGIKAM_GPS_ITEM_LIST_CONTEXT_MENU_H

// Qt includes:

#include <QObject>

// Local includes

#include "digikam_config.h"
#include "lookupaltitude.h"
#include "gpsdatacontainer.h"
#include "gpsitemlist.h"
#include "digikam_export.h"

class QUrl;

namespace Digikam
{

class GPSUndoCommand;
class GPSBookmarkOwner;

class DIGIKAM_EXPORT GPSItemListContextMenu : public QObject
{
    Q_OBJECT

public:

    explicit GPSItemListContextMenu(GPSItemList* const imagesList,
                                     GPSBookmarkOwner* const bookmarkOwner = 0);
    ~GPSItemListContextMenu();

    void setEnabled(const bool state);

protected:

    void setGPSDataForSelectedItems(const GPSDataContainer& gpsData, const QString& undoDescription);
    bool getCurrentItemPositionAndUrl(GPSDataContainer* const gpsInfo, QUrl* const itemUrl);
    void removeInformationFromSelectedImages(const GPSDataContainer::HasFlags flagsToClear, const QString& undoDescription);

    virtual bool eventFilter(QObject* watched, QEvent* event);

    static bool getCurrentPosition(GPSDataContainer* position, void* mydata);

private Q_SLOTS:

    void copyActionTriggered();
    void pasteSwapActionTriggered();
    void pasteActionTriggered(bool swap = false);
    void slotBookmarkSelected(const GPSDataContainer& position);
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

#endif // DIGIKAM_GPS_ITEM_LIST_CONTEXT_MENU_H
