/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-05-07
 * @brief  Context menu for GPS list view.
 *
 * @author Copyright (C) 2009-2011,2014 by Michael G. Hansen
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

#include "gpslistviewcontextmenu.h"

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

// KDE includes

#include <klocalizedstring.h>

// libkgeomap includes

#include <KGeoMap/LookupFactory>

// Libkipi includes

#include <KIPI/Interface>

// Local includes

#include "gpsbookmarkowner.h"
#include "gpsundocommand.h"
#include "gpssync_common.h"
#include "kipiimageitem.h"

namespace Digikam
{

class GPSListViewContextMenu::Private
{
public:

    Private()
      : enabled(true),
        actionCopy(0),
        actionPaste(0),
        actionBookmark(0),
        actionRemoveCoordinates(0),
        actionRemoveAltitude(0),
        actionRemoveUncertainty(0),
        actionRemoveSpeed(0),
        actionLookupMissingAltitudes(0),
        bookmarkOwner(0),
        imagesList(0),
        altitudeLookup(),
        altitudeUndoCommand(0),
        altitudeRequestedCount(0),
        altitudeReceivedCount(0)
    {

    }

    bool                              enabled;

    QAction*                          actionCopy;
    QAction*                          actionPaste;
    QAction*                          actionBookmark;
    QAction*                          actionRemoveCoordinates;
    QAction*                          actionRemoveAltitude;
    QAction*                          actionRemoveUncertainty;
    QAction*                          actionRemoveSpeed;
    QAction*                          actionLookupMissingAltitudes;

    GPSBookmarkOwner*                 bookmarkOwner;

    KipiImageList*                    imagesList;

    // Altitude lookup
    QPointer<KGeoMap::LookupAltitude> altitudeLookup;
    GPSUndoCommand*                   altitudeUndoCommand;
    int                               altitudeRequestedCount;
    int                               altitudeReceivedCount;
};

GPSListViewContextMenu::GPSListViewContextMenu(KipiImageList* const imagesList, GPSBookmarkOwner* const bookmarkOwner)
    : QObject(imagesList),
      d(new Private)
{
    d->imagesList                   = imagesList;

    d->actionCopy                   = new QAction(i18n("Copy coordinates"),                this);
    d->actionCopy->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")));
    d->actionPaste                  = new QAction(i18n("Paste coordinates"),               this);
    d->actionPaste->setIcon(QIcon::fromTheme(QStringLiteral("edit-paste")));
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
        d->bookmarkOwner  = bookmarkOwner;
        d->actionBookmark = new QAction(i18n("Bookmarks"), this);
        d->actionBookmark->setMenu(d->bookmarkOwner->getMenu());

        connect(d->bookmarkOwner, SIGNAL(positionSelected(GPSDataContainer)),
                this, SLOT(slotBookmarkSelected(GPSDataContainer)));
    }

    d->imagesList->installEventFilter(this);
}

GPSListViewContextMenu::~GPSListViewContextMenu()
{
    delete d->altitudeUndoCommand;
    delete d;
}

bool GPSListViewContextMenu::eventFilter(QObject *watched, QEvent *event)
{
    // we are only interested in context-menu events:
    if ((event->type()==QEvent::ContextMenu)&&d->enabled)
    {
        // enable or disable the actions:
        KipiImageModel* const imageModel          = d->imagesList->getModel();
        QItemSelectionModel* const selectionModel = d->imagesList->getSelectionModel();
        const QList<QModelIndex> selectedIndices  = selectionModel->selectedRows();
        const int nSelected                       = selectedIndices.size();

        // "copy" and "Add bookmark" are only available for one selected image with geo data:
        bool copyAvailable                   = (nSelected==1);
        bool removeAltitudeAvailable         = false;
        bool removeCoordinatesAvailable      = false;
        bool removeUncertaintyAvailable      = false;
        bool removeSpeedAvailable            = false;
        bool lookupMissingAltitudesAvailable = false;

        for (int i=0; i<nSelected; ++i)
        {
            KipiImageItem* const gpsItem = imageModel->itemFromIndex(selectedIndices.at(i));

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
            QClipboard * const clipboard = QApplication::clipboard();
            const QMimeData * mimedata   = clipboard->mimeData();
            pasteAvailable               = mimedata->hasFormat(QStringLiteral("application/gpx+xml")) || mimedata->hasText();
        }

        d->actionPaste->setEnabled(pasteAvailable);

        // construct the context-menu:
        QMenu * const menu = new QMenu(d->imagesList);
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
            d->actionBookmark->setEnabled(nSelected>=1);
        }

        QContextMenuEvent * const e = static_cast<QContextMenuEvent*>(event);
        menu->exec(e->globalPos());

        delete menu;
        return true;
    }
    else
    {
        return QObject::eventFilter(watched, event);
    }

}

bool GPSListViewContextMenu::getCurrentItemPositionAndUrl(GPSDataContainer* const gpsInfo, QUrl* const itemUrl)
{
    // NOTE: currentIndex does not seem to work any more since we use KLinkItemSelectionModel
    KipiImageModel* const imageModel          = d->imagesList->getModel();
    QItemSelectionModel* const selectionModel = d->imagesList->getSelectionModel();
    const QList<QModelIndex> selectedIndices  = selectionModel->selectedRows();

    if (selectedIndices.count()!=1)
    {
        return false;
    }

    const QModelIndex currentIndex = selectedIndices.first();

    if (!currentIndex.isValid())
    {
        return false;
    }

    KipiImageItem* const gpsItem = imageModel->itemFromIndex(currentIndex);

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

void GPSListViewContextMenu::copyActionTriggered()
{
    GPSDataContainer gpsInfo;
    QUrl itemUrl;

    if (!getCurrentItemPositionAndUrl(&gpsInfo, &itemUrl))
    {
        return;
    }

    CoordinatesToClipboard(gpsInfo.getCoordinates(), itemUrl, QString());
}

void GPSListViewContextMenu::pasteActionTriggered()
{
    // extract the coordinates from the clipboard:
    QClipboard * const clipboard = QApplication::clipboard();
    const QMimeData * mimedata   = clipboard->mimeData();

    GPSDataContainer gpsData;
    bool foundData       = false;
    if (mimedata->hasFormat(QStringLiteral("application/gpx+xml")))
    {
        const QByteArray data = mimedata->data(QStringLiteral("application/gpx+xml"));
        bool xmlOkay          = true;
        bool foundDoubleData = false;

        // code adapted from gpsdataparser.cpp
        QDomDocument gpxDoc(QStringLiteral("gpx"));

        if (!gpxDoc.setContent(data))
        {
            xmlOkay = false;
        }

        if (xmlOkay)
        {
            const QDomElement gpxDocElem = gpxDoc.documentElement();

            if (gpxDocElem.tagName() != QStringLiteral("gpx"))
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

                    if (wptElem.tagName() != QStringLiteral("wpt"))
                    {
                        continue;
                    }

                    double    ptAltitude  = 0.0;
                    double    ptLatitude  = 0.0;
                    double    ptLongitude = 0.0;
                    bool haveAltitude     = false;

                    // Get GPS position. If not available continue to next point.
                    const QString lat = wptElem.attribute(QStringLiteral("lat"));
                    const QString lon = wptElem.attribute(QStringLiteral("lon"));

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

                        if (wptMetaElem.tagName() == QStringLiteral("ele"))
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
                    KGeoMap::GeoCoordinates coordinates(ptLatitude, ptLongitude);

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
            QMessageBox::information(d->imagesList, i18n("GPS Sync"), i18n("Found more than one point on the clipboard - can only assign one point at a time."));
        }
    }

    if ((!foundData)&&(mimedata->hasText()))
    {
        const QString textdata                  = mimedata->text();
        bool foundGeoUrl                        = false;
        KGeoMap::GeoCoordinates testCoordinates = KGeoMap::GeoCoordinates::fromGeoUrl(textdata, &foundGeoUrl);

        if (foundGeoUrl)
        {
            gpsData.setCoordinates(testCoordinates);
            foundData = true;
        }
        else
        {
            /// @todo this is legacy code from before we used geo-url
            const QStringList parts = textdata.split(QLatin1Char(','));

            if ((parts.size()==3)||(parts.size()==2))
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

                if (okay&&(parts.size()==3))
                {
                    ptAltitude   = parts[2].toDouble(&okay);
                    haveAltitude = okay;
                }

                foundData = okay;

                if (okay)
                {
                    KGeoMap::GeoCoordinates coordinates(ptLatitude, ptLongitude);

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
        QMessageBox::information(d->imagesList, i18n("GPS Sync"), i18n("Could not find any coordinates on the clipboard."));
        return;
    }

    setGPSDataForSelectedItems(gpsData, i18n("Coordinates pasted"));
}

void GPSListViewContextMenu::setGPSDataForSelectedItems(const GPSDataContainer& gpsData, const QString& undoDescription)
{
    KipiImageModel* const imageModel          = d->imagesList->getModel();
    QItemSelectionModel* const selectionModel = d->imagesList->getSelectionModel();
    const QList<QModelIndex> selectedIndices  = selectionModel->selectedRows();
    const int nSelected                       = selectedIndices.size();
    GPSUndoCommand* const undoCommand         = new GPSUndoCommand();

    for (int i=0; i<nSelected; ++i)
    {
        const QModelIndex itemIndex  = selectedIndices.at(i);
        KipiImageItem* const gpsItem = imageModel->itemFromIndex(itemIndex);

        GPSUndoCommand::UndoInfo undoInfo(itemIndex);
        undoInfo.readOldDataFromItem(gpsItem);

        gpsItem->setGPSData(gpsData);
        undoInfo.readNewDataFromItem(gpsItem);

        undoCommand->addUndoInfo(undoInfo);
    }

    undoCommand->setText(undoDescription);
    emit(signalUndoCommand(undoCommand));
}

void GPSListViewContextMenu::slotBookmarkSelected(GPSDataContainer bookmarkPosition)
{
    setGPSDataForSelectedItems(bookmarkPosition, i18n("Bookmark selected"));
}

bool GPSListViewContextMenu::getCurrentPosition(GPSDataContainer* position, void* mydata)
{
    if (!position || !mydata)
    {
        return false;
    }

    GPSListViewContextMenu* const me = reinterpret_cast<GPSListViewContextMenu*>(mydata);

    return me->getCurrentItemPositionAndUrl(position, 0);
}

void GPSListViewContextMenu::removeInformationFromSelectedImages(const GPSDataContainer::HasFlags flagsToClear, const QString& undoDescription)
{
    // enable or disable the actions:
    KipiImageModel* const imageModel          = d->imagesList->getModel();
    QItemSelectionModel* const selectionModel = d->imagesList->getSelectionModel();
    const QList<QModelIndex> selectedIndices  = selectionModel->selectedRows();
    const int nSelected                       = selectedIndices.size();
    GPSUndoCommand* const undoCommand         = new GPSUndoCommand();

    for (int i=0; i<nSelected; ++i)
    {
        const QModelIndex itemIndex  = selectedIndices.at(i);
        KipiImageItem* const gpsItem = imageModel->itemFromIndex(itemIndex);

        GPSUndoCommand::UndoInfo undoInfo(itemIndex);
        undoInfo.readOldDataFromItem(gpsItem);

        GPSDataContainer newGPSData  = gpsItem->gpsData();
        bool didSomething            = false;

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

    if (undoCommand->affectedItemCount()>0)
    {
        undoCommand->setText(undoDescription);
        emit(signalUndoCommand(undoCommand));
    }
    else
    {
        delete undoCommand;
    }
}

void GPSListViewContextMenu::slotRemoveCoordinates()
{
    removeInformationFromSelectedImages(GPSDataContainer::HasCoordinates, i18n("Remove coordinates information"));
}

void GPSListViewContextMenu::slotRemoveAltitude()
{
    removeInformationFromSelectedImages(GPSDataContainer::HasAltitude, i18n("Remove altitude information"));
}

void GPSListViewContextMenu::slotRemoveUncertainty()
{
    removeInformationFromSelectedImages(GPSDataContainer::HasNSatellites|GPSDataContainer::HasDop|GPSDataContainer::HasFixType,
                                        i18n("Remove uncertainty information"));
}

void GPSListViewContextMenu::setEnabled(const bool state)
{
    d->enabled = state;
}

void GPSListViewContextMenu::slotRemoveSpeed()
{
    removeInformationFromSelectedImages(GPSDataContainer::HasSpeed, i18n("Remove speed"));
}

void GPSListViewContextMenu::slotLookupMissingAltitudes()
{
    KipiImageModel* const imageModel          = d->imagesList->getModel();
    QItemSelectionModel* const selectionModel = d->imagesList->getSelectionModel();
    const QList<QModelIndex> selectedIndices  = selectionModel->selectedRows();
//     const int nSelected = selectedIndices.size();

    // find the indices which have coordinates but no altitude
    KGeoMap::LookupAltitude::Request::List altitudeQueries;

    Q_FOREACH (const QModelIndex& currentIndex, selectedIndices)
    {
        KipiImageItem* const gpsItem = imageModel->itemFromIndex(currentIndex);

        if (!gpsItem)
        {
            continue;
        }

        const GPSDataContainer gpsData            = gpsItem->gpsData();
        const KGeoMap::GeoCoordinates coordinates = gpsData.getCoordinates();

        if ((!coordinates.hasCoordinates()) || coordinates.hasAltitude())
        {
            continue;
        }

        // the item has coordinates but no altitude, create a query
        KGeoMap::LookupAltitude::Request myLookup;
        myLookup.coordinates = coordinates;
        myLookup.data        = QVariant::fromValue(QPersistentModelIndex(currentIndex));

        altitudeQueries << myLookup;
    }

    if (altitudeQueries.isEmpty())
    {
        return;
    }

    d->altitudeLookup = KGeoMap::LookupFactory::getAltitudeLookup(QStringLiteral("geonames"), this);

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

void GPSListViewContextMenu::slotAltitudeLookupReady(const QList<int>& readyRequests)
{
    KipiImageModel* const imageModel = d->imagesList->getModel();

    Q_FOREACH(const int requestIndex, readyRequests)
    {
        const KGeoMap::LookupAltitude::Request myLookup = d->altitudeLookup->getRequest(requestIndex);
        const QPersistentModelIndex markerIndex         = myLookup.data.value<QPersistentModelIndex>();

        if (!markerIndex.isValid())
        {
            continue;
        }

        KipiImageItem* const gpsItem = imageModel->itemFromIndex(markerIndex);

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

void GPSListViewContextMenu::slotAltitudeLookupDone()
{
    KGeoMap::LookupAltitude::Status requestStatus = d->altitudeLookup->getStatus();

    if (requestStatus==KGeoMap::LookupAltitude::StatusError)
    {
        const QString errorMessage = i18n("Altitude lookup failed:\n%1", d->altitudeLookup->errorMessage());
        QMessageBox::information(d->imagesList, i18n("GPS Sync"),errorMessage);
    }

    if (d->altitudeReceivedCount>0)
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

void GPSListViewContextMenu::slotAltitudeLookupCancel()
{
    if (d->altitudeLookup)
    {
        d->altitudeLookup->cancel();
    }
}

} // namespace Digikam
