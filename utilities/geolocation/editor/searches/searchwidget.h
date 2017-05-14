/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-01
 * Description : A widget to search for places.
 *
 * Copyright (C) 2010-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

// Qt includes

#include <QAbstractItemModel>
#include <QWidget>

// Local includes

#include "digikam_config.h"
#include "modelhelper.h"
#include "searchbackend.h"

class QEvent;
class KConfigGroup;

namespace GeoIface
{
    class MapWidget;
}

namespace Digikam
{

class GPSUndoCommand;

#ifdef HAVE_KBOOKMARKS
class GPSBookmarkOwner;
#endif

class SearchWidget : public QWidget
{
    Q_OBJECT

public:

    SearchWidget(
#ifdef HAVE_KBOOKMARKS
                 GPSBookmarkOwner* const gpsBookmarkOwner,
#endif
                 GPSImageModel* const gpsImageModel,
                 QItemSelectionModel* const gpsImageSelectionModel,
                 QWidget* const parent
                );
    ~SearchWidget();

    GeoIface::ModelHelper* getModelHelper() const;

    void saveSettingsToGroup(KConfigGroup* const group);
    void readSettingsFromGroup(const KConfigGroup* const group);
    void setPrimaryMapWidget(GeoIface::MapWidget* const mapWidget);

private Q_SLOTS:

    void slotSearchCompleted();
    void slotTriggerSearch();
    void slotCurrentlySelectedResultChanged(const QModelIndex& current, const QModelIndex& previous);
    void slotClearSearchResults();
    void slotVisibilityChanged(bool state);
    void slotCopyCoordinates();
    void slotMoveSelectedImagesToThisResult();
    void slotUpdateActionAvailability();
    void slotRemoveSelectedFromResultsList();

protected:

    virtual bool eventFilter(QObject* watched, QEvent* event);

Q_SIGNALS:

    void signalUndoCommand(GPSUndoCommand* undoCommand);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // SEARCHWIDGET_H
