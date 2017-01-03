/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2006-05-16
 * @brief  A tool to edit geolocation
 *
 * @author Copyright (C) 2006-2017 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2010, 2014 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010 by Gabriel Voicu
 *         <a href="mailto:ping dot gabi at gmail dot com">ping dot gabi at gmail dot com</a>
 * @author Copyright (C) 2014 by Justus Schwartz
 *         <a href="mailto:justus at gmx dot li">justus at gmx dot li</a>
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

#ifndef GEOLOCATIONEDIT_H
#define GEOLOCATIONEDIT_H

// Qt includes

#include <QModelIndex>
#include <QWidget>
#include <QDialog>
#include <QUrl>

// Local includes

#include "geoiface_types.h"
#include "modelhelper.h"
#include "tracks.h"
#include "gpsimageitem.h"
#include "digikam_export.h"

class QAbstractItemModel;

namespace GeoIface
{
    class MapWidget;
}
using namespace GeoIface;

namespace Digikam
{

class GPSUndoCommand;

class DIGIKAM_EXPORT GeolocationEdit : public QDialog
{
    Q_OBJECT

public:

    explicit GeolocationEdit(QAbstractItemModel* const externTagModel, QWidget* const parent);
    ~GeolocationEdit();

    /* Populate items in dialog list based on url. To be used in case of non database as with Showfoto.
     */
    void setImages(const QList<QUrl>& images);

    /* Populate items in dialog list based dedicated GPSImageItem instances filled with DB info
     */
    void setItems(const QList<GPSImageItem*>& items);

protected:

    void closeEvent(QCloseEvent* e);
    bool eventFilter(QObject*, QEvent*);

private:

    void readSettings();
    void saveSettings();
    void saveChanges(const bool closeAfterwards);
    MapWidget* makeMapWidget(QWidget** const pvbox);
    void adjustMapLayout(const bool syncSettings);

private Q_SLOTS:

    void slotImageActivated(const QModelIndex& index);
    void slotSetUIEnabled(const bool enabledState, QObject* const cancelObject, const QString& cancelSlot);
    void slotSetUIEnabled(const bool enabledState);
    void slotApplyClicked();
    void slotFileChangesSaved(int beginIndex, int endIndex);
    void slotFileMetadataLoaded(int beginIndex, int endIndex);
    void slotProgressChanged(const int currentProgress);
    void slotProgressSetup(const int maxProgress, const QString& progressText);
    void slotGPSUndoCommand(GPSUndoCommand* undoCommand);
    void slotSortOptionTriggered(QAction* sortAction);
    void setCurrentTab(const int index);
    void slotProgressCancelButtonClicked();
    void slotCurrentTabChanged(int);
    void slotBookmarkVisibilityToggled();
    void slotLayoutChanged(int);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* GEOLOCATIONEDIT_H */
