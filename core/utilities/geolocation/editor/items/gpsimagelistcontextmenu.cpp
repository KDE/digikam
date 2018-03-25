/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-07
 * Description : Context menu for GPS list view.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "gpsimagelistcontextmenu.h"

// Qt includes:

#include <QEvent>
#include <QContextMenuEvent>
#include <QClipboard>
#include <QApplication>
#include <QDomDocument>
#include <QPointer>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "gpsundocommand.h"
#include "gpscommon.h"
#include "gpsimageitem.h"
#include "lookupfactory.h"
#include "gpsbookmarkowner.h"
#include "digikam_debug.h"

namespace Digikam
{

class GPSImageListContextMenu::Private
{
public:

    Private()
      : enabled(true),
        actionBookmark(0),
        bookmarkOwner(0),
        actionCopy(0),
        actionPaste(0),
        actionRemoveCoordinates(0),
        actionRemoveAltitude(0),
        actionRemoveUncertainty(0),
        actionRemoveSpeed(0),
        actionLookupMissingAltitudes(0),
        imagesList(0),
        altitudeLookup(),
        altitudeUndoCommand(0),
        altitudeRequestedCount(0),
        altitudeReceivedCount(0)
    {
    }

    bool                               enabled;

    QAction*                           actionBookmark;
    GPSBookmarkOwner*                  bookmarkOwner;

    QAction*                           actionCopy;
    QAction*                           actionPaste;
    QAction*                           actionRemoveCoordinates;
    QAction*                           actionRemoveAltitude;
    QAction*                           actionRemoveUncertainty;
    QAction*                           actionRemoveSpeed;
    QAction*                           actionLookupMissingAltitudes;

    GPSImageList*                      imagesList;

    // Altitude lookup
    QPointer<LookupAltitude> altitudeLookup;
    GPSUndoCommand*                    altitudeUndoCommand;
    int                                altitudeRequestedCount;
    int                                altitudeReceivedCount;
};

GPSImageListContextMenu::GPSImageListContextMenu(GPSImageList* const imagesList,
                                                 GPSBookmarkOwner* const bookmarkOwner)
    : QObject(imagesList),
      d(new Private)
{
    d->imagesList                   = imagesList;

    d->actionCopy                   = new QAction(i18n("Copy coordinates"),                this);
    d->actionCopy->setIcon(QIcon::fromTheme(QString::fromLatin1("edit-copy")));
    d->actionPaste                  = new QAction(i18n("Paste coordinates"),               this);
    d->actionPaste->setIcon(QIcon::fromTheme(QString::fromLatin1("edit-paste")));
    d->actionRemoveCoordinates      = new QAction(i18n("Remove coordinates"),              this);
    d->actionRemoveAltitude         = new QAction(i18n("Remove altitude"),                 this);
    d->actionRemoveUncertainty      = new QAction(i18n("Remove uncertainty"),              this);
    d->actionRemoveSpeed            = new QAction(i18n("Remove speed"),                    this);
    d->actionLookupMissingAltitudes = new QAction(i18n("Look up missing altitude values"), this);

    connect(d->actionCopy, SIGNAL(triggered()),
            this, SLOT(copyActionTriggered()));

    connect(d->actionPaste, SIGNAL(triggered()),
            this, SLOT(pasteActionTriggered()));

    connect(d->actionRemoveCoordinates, SIGNAL(triggered()),
            this, SLOT(slotRemoveCoordinates()));

    connect(d->actionRemoveAltitude, SIGNAL(triggered()),
            this, SLOT(slotRemoveAltitude()));

    connect(d->actionRemoveUncertainty, SIGNAL(triggered()),
            this, SLOT(slotRemoveUncertainty()));

    connect(d->actionRemoveSpeed, SIGNAL(triggered()),
            this, SLOT(slotRemoveSpeed()));

    connect(d->actionLookupMissingAltitudes, SIGNAL(triggered()),
            this, SLOT(slotLookupMissingAltitudes()));

    if (bookmarkOwner)
    {
        d->bookmarkOwner   = bookmarkOwner;
        d->actionBookmark  = new QAction(i18n("Bookmarks"), this);
        d->actionBookmark->setMenu(d->bookmarkOwner->getMenu());

        connect(d->bookmarkOwner, SIGNAL(positionSelected(GPSDataContainer)),
                this, SLOT(slotBookmarkSelected(GPSDataContainer)));
    }

    d->imagesList->installEventFilter(this);
}

GPSImageListContextMenu::~GPSImageListContextMenu()
{
    delete d->altitudeUndoCommand;
    delete d;
}

bool GPSImageListContextMenu::eventFilter(QObject* watched, QEvent* event)
{
    // We are only interested in context-menu events.

    if ((event->type() == QEvent::ContextMenu) && d->enabled)
    {
        // enable or disable the actions:
        GPSImageModel* const imageModel           = d->imagesList->getModel();
        QItemSelectionModel* const selectionModel = d->imagesList->getSelectionModel();
        const QList<QModelIndex> selectedIndices  = selectionModel->selectedRows();
        const int nSelected                       = selectedIndices.size();

        // "copy" are only available for one selected image with geo data:
        bool copyAvailable                   = (nSelected == 1);
        bool removeAltitudeAvailable         = false;
        bool removeCoordinatesAvailable      = false;
        bool removeUncertaintyAvailable      = false;
        bool removeSpeedAvailable            = false;
        bool lookupMissingAltitudesAvailable = false;

        for (int i = 0 ; i < nSelected ; ++i)
        {
            GPSImageItem* const gpsItem = imageModel->itemFromIndex(selectedIndices.at(i));

            if (gpsItem)
            {
                const GPSDataContainer gpsData   = gpsItem->gpsData();
                const bool itemHasCoordinates    = gpsData.getCoordinates().hasCoordinates();
                copyAvailable                   &= itemHasCoordinates;
                removeCoordinatesAvailable      |= itemHasCoordinates;
                removeAltitudeAvailable         |= gpsData.getCoordinates().hasAltitude();
                removeUncertaintyAvailable      |= gpsData.hasNSatellites() | gpsData.hasDop() | gpsData.hasFixType();
                removeSpeedAvailable            |= gpsData.hasSpeed();
                lookupMissingAltitudesAvailable |= itemHasCoordinates && !gpsData.getCoordinates().hasAltitude();
            }
        }

        d->actionCopy->setEnabled(copyAvailable);
        d->actionRemoveAltitude->setEnabled(removeAltitudeAvailable);
        d->actionRemoveCoordinates->setEnabled(removeCoordinatesAvailable);
        d->actionRemoveUncertainty->setEnabled(removeUncertaintyAvailable);
        d->actionRemoveSpeed->setEnabled(removeSpeedAvailable);
        d->actionLookupMissingAltitudes->setEnabled(lookupMissingAltitudesAvailable);

        if (d->bookmarkOwner)
        {
            d->bookmarkOwner->changeAddBookmark(copyAvailable);
            GPSDataContainer position;
            QUrl             itemUrl;
            getCurrentItemPositionAndUrl(&position, &itemUrl);
            const QString itemFileName = itemUrl.fileName();
            d->bookmarkOwner->setPositionAndTitle(position.getCoordinates(), itemFileName);
        }

        // "paste" is only available if there is geo data in the clipboard
        // and at least one photo is selected:
        bool pasteAvailable = (nSelected >= 1);

        if (pasteAvailable)
        {
            QClipboard* const clipboard = QApplication::clipboard();
            const QMimeData* mimedata   = clipboard->mimeData();
            pasteAvailable              = mimedata->hasFormat(QString::fromLatin1("application/gpx+xml")) || mimedata->hasText();
        }

        d->actionPaste->setEnabled(pasteAvailable);

        // construct the context-menu:
        QMenu* const menu = new QMenu(d->imagesList);
        menu->addAction(d->actionCopy);
        menu->addAction(d->actionPaste);
        menu->addSeparator();
        menu->addAction(d->actionRemoveCoordinates);
        menu->addAction(d->actionRemoveAltitude);
        menu->addAction(d->actionRemoveUncertainty);
        menu->addAction(d->actionRemoveSpeed);
        menu->addAction(d->actionLookupMissingAltitudes);

        if (d->actionBookmark)
        {
            menu->addSeparator();
            menu->addAction(d->actionBookmark);
            d->actionBookmark->setEnabled(nSelected >= 1);
        }

        QContextMenuEvent* const e = static_cast<QContextMenuEvent*>(event);
        menu->exec(e->globalPos());

        delete menu;
        return true;
    }
    else
    {
        return QObject::eventFilter(watched, event);
    }

}

bool GPSImageListContextMenu::getCurrentItemPositionAndUrl(GPSDataContainer* const gpsInfo,
                                                           QUrl* const itemUrl)
{
    // NOTE: currentIndex does not seem to work any more since we use KLinkItemSelectionModel
    GPSImageModel* const imageModel           = d->imagesList->getModel();
    QItemSelectionModel* const selectionModel = d->imagesList->getSelectionModel();
    const QList<QModelIndex> selectedIndices  = selectionModel->selectedRows();

    if (selectedIndices.count() != 1)
    {
        return false;
    }

    const QModelIndex currentIndex = selectedIndices.first();

    if (!currentIndex.isValid())
    {
        return false;
    }

    GPSImageItem* const gpsItem = imageModel->itemFromIndex(currentIndex);

    if (gpsItem)
    {
        if (gpsInfo)
        {
            *gpsInfo = gpsItem->gpsData();
        }

        if (itemUrl)
        {
            *itemUrl = gpsItem->url();
        }

        return true;
    }

    return false;
}

void GPSImageListContextMenu::copyActionTriggered()
{
    GPSDataContainer gpsInfo;
    QUrl itemUrl;

    if (!getCurrentItemPositionAndUrl(&gpsInfo, &itemUrl))
    {
        return;
    }

    coordinatesToClipboard(gpsInfo.getCoordinates(), itemUrl, QString());
}

void GPSImageListContextMenu::pasteActionTriggered()
{
    // extract the coordinates from the clipboard:
    QClipboard* const clipboard = QApplication::clipboard();
    const QMimeData* mimedata   = clipboard->mimeData();

    GPSDataContainer gpsData;
    bool foundData              = false;

    if (mimedata->hasFormat(QString::fromLatin1("application/gpx+xml")))
    {
        const QByteArray data = mimedata->data(QString::fromLatin1("application/gpx+xml"));
        bool xmlOkay          = true;
        bool foundDoubleData  = false;

        // code adapted from gpsdataparser.cpp
        QDomDocument gpxDoc(QString::fromLatin1("gpx"));

        if (!gpxDoc.setContent(data))
        {
            xmlOkay = false;
        }

        if (xmlOkay)
        {
            const QDomElement gpxDocElem = gpxDoc.documentElement();

            if (gpxDocElem.tagName() != QString::fromLatin1("gpx"))
            {
                xmlOkay = false;
            }

            if (xmlOkay)
            {
                for (QDomNode nWpt = gpxDocElem.firstChild(); !nWpt.isNull(); nWpt = nWpt.nextSibling())
                {
                    const QDomElement wptElem = nWpt.toElement();

                    if (wptElem.isNull())
                    {
                        continue;
                    }

                    if (wptElem.tagName() != QString::fromLatin1("wpt"))
                    {
                        continue;
                    }

                    double    ptAltitude  = 0.0;
                    double    ptLatitude  = 0.0;
                    double    ptLongitude = 0.0;
                    bool haveAltitude     = false;

                    // Get GPS position. If not available continue to next point.
                    const QString lat     = wptElem.attribute(QString::fromLatin1("lat"));
                    const QString lon     = wptElem.attribute(QString::fromLatin1("lon"));

                    if (lat.isEmpty() || lon.isEmpty())
                    {
                        continue;
                    }

                    ptLatitude  = lat.toDouble();
                    ptLongitude = lon.toDouble();

                    if (foundData)
                    {
                        foundDoubleData = true;
                        break;
                    }

                    // Get metadata of way point (altitude and time stamp)
                    for (QDomNode nWptMeta = wptElem.firstChild(); !nWptMeta.isNull(); nWptMeta = nWptMeta.nextSibling())
                    {
                        const QDomElement wptMetaElem = nWptMeta.toElement();

                        if (wptMetaElem.isNull())
                        {
                            continue;
                        }

                        if (wptMetaElem.tagName() == QString::fromLatin1("ele"))
                        {
                            // Get GPS point altitude. If not available continue to next point.
                            QString ele = wptMetaElem.text();

                            if (!ele.isEmpty())
                            {
                                ptAltitude  = ele.toDouble(&haveAltitude);
                                break;
                            }
                        }
                    }

                    foundData = true;
                    GeoCoordinates coordinates(ptLatitude, ptLongitude);

                    if (haveAltitude)
                    {
                        coordinates.setAlt(ptAltitude);
                    }

                    gpsData.setCoordinates(coordinates);
                }
            }
        }

        if (foundDoubleData)
        {
            QMessageBox::information(d->imagesList,
                                     i18n("GPS Sync"),
                                     i18n("Found more than one point on the clipboard - can only assign one point at a time."));
        }
    }

    if ((!foundData)&&(mimedata->hasText()))
    {
        const QString textdata                   = mimedata->text();
        bool foundGeoUrl                         = false;
        GeoCoordinates testCoordinates = GeoCoordinates::fromGeoUrl(textdata, &foundGeoUrl);

        if (foundGeoUrl)
        {
            gpsData.setCoordinates(testCoordinates);
            foundData = true;
        }
        else
        {
            /// @todo this is legacy code from before we used geo-url
            const QStringList parts = textdata.split(QLatin1Char(','));

            if ((parts.size() == 3) || (parts.size() == 2))
            {
                bool okay            = true;
                double    ptLatitude = 0.0;
                double    ptAltitude = 0.0;
                bool haveAltitude    = false;

                const double ptLongitude = parts[0].toDouble(&okay);

                if (okay)
                {
                    ptLatitude = parts[1].toDouble(&okay);
                }

                if (okay && (parts.size() == 3))
                {
                    ptAltitude   = parts[2].toDouble(&okay);
                    haveAltitude = okay;
                }

                foundData = okay;

                if (okay)
                {
                    GeoCoordinates coordinates(ptLatitude, ptLongitude);

                    if (haveAltitude)
                    {
                        coordinates.setAlt(ptAltitude);
                    }

                    gpsData.setCoordinates(coordinates);
                }
            }
        }
    }

    if (!foundData)
    {
        QMessageBox::information(d->imagesList,
                                 i18n("Geolocation Editor"),
                                 i18n("Could not find any coordinates on the clipboard."));
        return;
    }

    setGPSDataForSelectedItems(gpsData, i18n("Coordinates pasted"));
}

void GPSImageListContextMenu::setGPSDataForSelectedItems(const GPSDataContainer& gpsData,
                                                         const QString& undoDescription)
{
    GPSImageModel* const imageModel           = d->imagesList->getModel();
    QItemSelectionModel* const selectionModel = d->imagesList->getSelectionModel();
    const QList<QModelIndex> selectedIndices  = selectionModel->selectedRows();
    const int nSelected                       = selectedIndices.size();
    GPSUndoCommand* const undoCommand         = new GPSUndoCommand();

    for (int i = 0 ; i < nSelected ; ++i)
    {
        const QModelIndex itemIndex = selectedIndices.at(i);
        GPSImageItem* const gpsItem = imageModel->itemFromIndex(itemIndex);

        GPSUndoCommand::UndoInfo undoInfo(itemIndex);
        undoInfo.readOldDataFromItem(gpsItem);

        gpsItem->setGPSData(gpsData);
        undoInfo.readNewDataFromItem(gpsItem);

        undoCommand->addUndoInfo(undoInfo);
    }

    undoCommand->setText(undoDescription);
    emit(signalUndoCommand(undoCommand));
}

void GPSImageListContextMenu::slotBookmarkSelected(const GPSDataContainer& position)
{
    setGPSDataForSelectedItems(position, i18n("Bookmark selected"));
}

bool GPSImageListContextMenu::getCurrentPosition(GPSDataContainer* position, void* mydata)
{
    if (!position || !mydata)
    {
        return false;
    }

    GPSImageListContextMenu* const me = reinterpret_cast<GPSImageListContextMenu*>(mydata);

    return me->getCurrentItemPositionAndUrl(position, 0);
}

void GPSImageListContextMenu::removeInformationFromSelectedImages(const GPSDataContainer::HasFlags flagsToClear, const QString& undoDescription)
{
    // enable or disable the actions:
    GPSImageModel* const imageModel           = d->imagesList->getModel();
    QItemSelectionModel* const selectionModel = d->imagesList->getSelectionModel();
    const QList<QModelIndex> selectedIndices  = selectionModel->selectedRows();
    const int nSelected                       = selectedIndices.size();
    GPSUndoCommand* const undoCommand         = new GPSUndoCommand();

    for (int i = 0 ; i < nSelected ; ++i)
    {
        const QModelIndex itemIndex = selectedIndices.at(i);
        GPSImageItem* const gpsItem = imageModel->itemFromIndex(itemIndex);

        GPSUndoCommand::UndoInfo undoInfo(itemIndex);
        undoInfo.readOldDataFromItem(gpsItem);

        GPSDataContainer newGPSData = gpsItem->gpsData();
        bool didSomething           = false;

        if (flagsToClear.testFlag(GPSDataContainer::HasCoordinates))
        {
            if (newGPSData.hasCoordinates())
            {
                didSomething = true;
                newGPSData.clear();
            }
        }

        if (flagsToClear.testFlag(GPSDataContainer::HasAltitude))
        {
            if (newGPSData.hasAltitude())
            {
                didSomething = true;
                newGPSData.clearAltitude();
            }
        }

        if (flagsToClear.testFlag(GPSDataContainer::HasNSatellites))
        {
            if (newGPSData.hasNSatellites())
            {
                didSomething = true;
                newGPSData.clearNSatellites();
            }
        }

        if (flagsToClear.testFlag(GPSDataContainer::HasDop))
        {
            if (newGPSData.hasDop())
            {
                didSomething = true;
                newGPSData.clearDop();
            }
        }

        if (flagsToClear.testFlag(GPSDataContainer::HasFixType))
        {
            if (newGPSData.hasFixType())
            {
                didSomething = true;
                newGPSData.clearFixType();
            }
        }

        if (flagsToClear.testFlag(GPSDataContainer::HasSpeed))
        {
            if (newGPSData.hasSpeed())
            {
                didSomething = true;
                newGPSData.clearSpeed();
            }
        }

        if (didSomething)
        {
            gpsItem->setGPSData(newGPSData);
            undoInfo.readNewDataFromItem(gpsItem);
            undoCommand->addUndoInfo(undoInfo);
        }
    }

    if (undoCommand->affectedItemCount() > 0)
    {
        undoCommand->setText(undoDescription);
        emit(signalUndoCommand(undoCommand));
    }
    else
    {
        delete undoCommand;
    }
}

void GPSImageListContextMenu::slotRemoveCoordinates()
{
    removeInformationFromSelectedImages(GPSDataContainer::HasCoordinates, i18n("Remove coordinates information"));
}

void GPSImageListContextMenu::slotRemoveAltitude()
{
    removeInformationFromSelectedImages(GPSDataContainer::HasAltitude, i18n("Remove altitude information"));
}

void GPSImageListContextMenu::slotRemoveUncertainty()
{
    removeInformationFromSelectedImages(GPSDataContainer::HasNSatellites|GPSDataContainer::HasDop|GPSDataContainer::HasFixType,
                                        i18n("Remove uncertainty information"));
}

void GPSImageListContextMenu::setEnabled(const bool state)
{
    d->enabled = state;
}

void GPSImageListContextMenu::slotRemoveSpeed()
{
    removeInformationFromSelectedImages(GPSDataContainer::HasSpeed, i18n("Remove speed"));
}

void GPSImageListContextMenu::slotLookupMissingAltitudes()
{
    GPSImageModel* const imageModel           = d->imagesList->getModel();
    QItemSelectionModel* const selectionModel = d->imagesList->getSelectionModel();
    const QList<QModelIndex> selectedIndices  = selectionModel->selectedRows();
//  const int nSelected                       = selectedIndices.size();

    // find the indices which have coordinates but no altitude
    LookupAltitude::Request::List altitudeQueries;

    foreach(const QModelIndex& currentIndex, selectedIndices)
    {
        GPSImageItem* const gpsItem = imageModel->itemFromIndex(currentIndex);

        if (!gpsItem)
        {
            continue;
        }

        const GPSDataContainer gpsData            = gpsItem->gpsData();
        const GeoCoordinates coordinates = gpsData.getCoordinates();

        if ((!coordinates.hasCoordinates()) || coordinates.hasAltitude())
        {
            continue;
        }

        // the item has coordinates but no altitude, create a query
        LookupAltitude::Request myLookup;
        myLookup.coordinates = coordinates;
        myLookup.data        = QVariant::fromValue(QPersistentModelIndex(currentIndex));

        altitudeQueries << myLookup;
    }

    if (altitudeQueries.isEmpty())
    {
        return;
    }

    d->altitudeLookup = LookupFactory::getAltitudeLookup(QString::fromLatin1("geonames"), this);

    connect(d->altitudeLookup, SIGNAL(signalRequestsReady(QList<int>)),
            this, SLOT(slotAltitudeLookupReady(QList<int>)));

    connect(d->altitudeLookup, SIGNAL(signalDone()),
            this, SLOT(slotAltitudeLookupDone()));

    emit(signalSetUIEnabled(false, this, QLatin1String(SLOT(slotAltitudeLookupCancel()))));
    emit(signalProgressSetup(altitudeQueries.count(), i18n("Looking up altitudes")));

    d->altitudeUndoCommand    = new GPSUndoCommand();
    d->altitudeRequestedCount = altitudeQueries.count();
    d->altitudeReceivedCount  = 0;
    d->altitudeLookup->addRequests(altitudeQueries);
    d->altitudeLookup->startLookup();
}

void GPSImageListContextMenu::slotAltitudeLookupReady(const QList<int>& readyRequests)
{
    GPSImageModel* const imageModel = d->imagesList->getModel();

    foreach(const int requestIndex, readyRequests)
    {
        const LookupAltitude::Request myLookup = d->altitudeLookup->getRequest(requestIndex);
        const QPersistentModelIndex markerIndex          = myLookup.data.value<QPersistentModelIndex>();

        if (!markerIndex.isValid())
        {
            continue;
        }

        GPSImageItem* const gpsItem = imageModel->itemFromIndex(markerIndex);

        if (!gpsItem)
        {
            continue;
        }

        GPSUndoCommand::UndoInfo undoInfo(markerIndex);
        undoInfo.readOldDataFromItem(gpsItem);

        GPSDataContainer gpsData = gpsItem->gpsData();
        gpsData.setCoordinates(myLookup.coordinates);
        gpsItem->setGPSData(gpsData);
        undoInfo.readNewDataFromItem(gpsItem);

        d->altitudeUndoCommand->addUndoInfo(undoInfo);
        d->altitudeReceivedCount++;
    }

    signalProgressChanged(d->altitudeReceivedCount);
}

void GPSImageListContextMenu::slotAltitudeLookupDone()
{
    LookupAltitude::StatusAltitude requestStatus = d->altitudeLookup->getStatus();

    if (requestStatus == LookupAltitude::StatusError)
    {
        const QString errorMessage = i18n("Altitude lookup failed:\n%1", d->altitudeLookup->errorMessage());
        QMessageBox::information(d->imagesList, i18n("GPS Sync"),errorMessage);
    }

    if (d->altitudeReceivedCount > 0)
    {
        // at least some queries returned a result, save the undo command
        d->altitudeUndoCommand->setText(i18n("Altitude looked up"));
        emit(signalUndoCommand(d->altitudeUndoCommand));
    }
    else
    {
        delete d->altitudeUndoCommand;
    }

    d->altitudeUndoCommand = 0;
    d->altitudeLookup->deleteLater();

    emit(signalSetUIEnabled(true));
}

void GPSImageListContextMenu::slotAltitudeLookupCancel()
{
    if (d->altitudeLookup)
    {
        d->altitudeLookup->cancel();
    }
}

} // namespace Digikam
