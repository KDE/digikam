/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : a widget to display GPS info on a world map
 *
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "worldmapwidget.h"
#include "worldmapwidget.moc"

// Qt includes

#include <QVBoxLayout>
#include <QStyle>
#include <QDomDocument>
#include <QTextStream>
#include <QFile>
#include <QLabel>
#include <QAbstractItemModel>

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <ktemporaryfile.h>
#include <kmenu.h>
#include <kiconloader.h>

#include <config-digikam.h>
#ifdef HAVE_MARBLEWIDGET
#include <marble/MarbleWidget.h>
using namespace Marble;

Q_DECLARE_METATYPE(Digikam::GPSInfo)

template<> MarkerClusterHolder::MarkerInfo MarkerClusterHolder::MarkerInfo::fromData<Digikam::GPSInfo>(const Digikam::GPSInfo& yourdata)
{
    return MarkerClusterHolder::MarkerInfo(yourdata.longitude, yourdata.latitude, QVariant::fromValue(yourdata));
}

bool MarkerInfoDataEqualFunction(const QVariant& one, const QVariant& two, void* const yourdata)
{
    Q_UNUSED(yourdata)
    
    const Digikam::GPSInfo oneInfo = one.value<Digikam::GPSInfo>();
    const Digikam::GPSInfo twoInfo = two.value<Digikam::GPSInfo>();
    
    // just compare the urls of the GPSInfos:
    return (oneInfo.url==twoInfo.url);
}

#endif // HAVE_MARBLEWIDGET


// local includes

#include <thumbnailloadthread.h>

namespace Digikam
{
class ClusterUserData
{
public:
    ClusterUserData()
      : thumbnailMarkerIndex(-1),
        thumbnailSettingsHash(0)
    {
    }
    
    int thumbnailMarkerIndex;
    int thumbnailSettingsHash;
};
}

Q_DECLARE_METATYPE(Digikam::ClusterUserData)

namespace Digikam
{
  
class WorldMapWidgetPriv
{

public:

    WorldMapWidgetPriv()
    : list(),
      mapTheme(WorldMapWidget::AtlasMap),
      focusOnAddedItems(true),
      multiMarkerShowSingleImages(false),
      multiMarkerShowGroupImages(false),
      multiMarkerShowHighestRatingFirst(false),
      multiMarkerShowOldestFirst(false),
      multiMarkerShowNumbers(false),
#ifdef HAVE_MARBLEWIDGET
      marbleWidget(0),
      markerClusterHolder(0),
      thumbnailLoadThread(0),
      thumbnailLoadThreadBuncher(0)
#else
      marbleWidget(0)
#endif // HAVE_MARBLEWIDGET
    {
    };

    GPSInfoList              list;
    WorldMapWidget::MapTheme mapTheme;
    bool focusOnAddedItems;
    bool multiMarkerShowSingleImages;
    bool multiMarkerShowGroupImages;
    bool multiMarkerShowHighestRatingFirst;
    bool multiMarkerShowOldestFirst;
    bool multiMarkerShowNumbers;

#ifdef HAVE_MARBLEWIDGET
    MarbleSubClassWidget*    marbleWidget;
    MarkerClusterHolder*     markerClusterHolder;
    ThumbnailLoadThread*     thumbnailLoadThread;
    QTimer*                  thumbnailLoadThreadBuncher;
#else
    QLabel*                  marbleWidget;
#endif // HAVE_MARBLEWIDGET

};

WorldMapWidget::WorldMapWidget(int w, int h, QWidget *parent)
              : QFrame(parent), d(new WorldMapWidgetPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumWidth(w);
    setMinimumHeight(h);
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(style()->pixelMetric(QStyle::PM_DefaultFrameWidth));

#ifdef HAVE_MARBLEWIDGET
    d->marbleWidget = new MarbleSubClassWidget(this);
    d->markerClusterHolder = d->marbleWidget->m_markerClusterHolder;
    d->markerClusterHolder->setMarkerDataEqualFunction(MarkerInfoDataEqualFunction, 0);
    d->markerClusterHolder->setClusterPixmapFunction(getClusterPixmap, this);
    d->thumbnailLoadThread = new ThumbnailLoadThread();
    d->thumbnailLoadThreadBuncher = new QTimer(this);
    d->thumbnailLoadThreadBuncher->setSingleShot(true);
    
    connect(d->markerClusterHolder, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotMapMarkerSelectionChanged()));
    connect(d->markerClusterHolder, SIGNAL(signalSoloChanged()),
            this, SLOT(slotMapMarkerSoloChanged()));
    connect(d->thumbnailLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotThumbnailLoaded(const LoadingDescription&, const QPixmap&)));
    connect(d->thumbnailLoadThreadBuncher, SIGNAL(timeout()),
            d->marbleWidget, SLOT(update()));
            
#if MARBLE_VERSION < 0x000800
    d->marbleWidget->setDownloadUrl("http://download.kde.org/apps/marble/");
#endif // MARBLE_VERSION < 0x000800

#else // HAVE_MARBLEWIDGET
    d->marbleWidget = new QLabel(this);
    d->marbleWidget->setText(i18n("Geolocation using Marble not available"));
    d->marbleWidget->setWordWrap(true);
#endif // HAVE_MARBLEWIDGET

    QVBoxLayout *vlay = new QVBoxLayout(this);
    vlay->addWidget(d->marbleWidget);
    vlay->setMargin(0);
    vlay->setSpacing(0);
}

WorldMapWidget::~WorldMapWidget()
{
    delete d;
}

void WorldMapWidget::slotSetFocusOnAddedItems(const bool doIt)
{
    d->focusOnAddedItems = doIt;
}

#ifdef HAVE_MARBLEWIDGET
MarkerClusterHolder* WorldMapWidget::getMarkerClusterHolder() const
{
    return d->markerClusterHolder;
}
#endif // HAVE_MARBLEWIDGET

QWidget* WorldMapWidget::marbleWidget() const
{
    return d->marbleWidget;
}

double WorldMapWidget::getLatitude()
{
    return d->list.first().latitude;
}

double WorldMapWidget::getLongitude()
{
    return d->list.first().longitude;
}

void WorldMapWidget::clearGPSPositions()
{
//     kDebug(50003) << "WorldMapWidget::clearGPSPositions()";
    d->list.clear();
#ifdef HAVE_MARBLEWIDGET
    // TODO: removing placemark data from Marble does not currently work (2009-08-28, r1014558)
//    QAbstractItemModel* const marbleModel = d->marbleWidget->placemarkModel();
//    kDebug(50003) << QString("clearing PlacemarkModel: %1 rows").arg(marbleModel->rowCount());
//     marbleModel->removeRows(0, marbleModel->rowCount());
//    d->marbleWidget->removePlacemarkKey();
//    kDebug(50003) << QString("PlacemarkModel cleared: %1 rows").arg(marbleModel->rowCount());
    d->markerClusterHolder->clear();
#endif // HAVE_MARBLEWIDGET
}

void WorldMapWidget::setGPSPositions(const GPSInfoList& list)
{
    clearGPSPositions();
    addGPSPositions(list);
}

void WorldMapWidget::addGPSPositions(const GPSInfoList& list)
{
    d->list << list;

#ifdef HAVE_MARBLEWIDGET
    // center on the first item in the list:
    if ( d->focusOnAddedItems && !list.isEmpty() )
    {
        const GPSInfo& firstInfo = list.first();
        const qreal lng = firstInfo.longitude;
        const qreal lat = firstInfo.latitude;
        d->marbleWidget->setHome(lng, lat);
        d->marbleWidget->centerOn(lng, lat);
    }
#endif // HAVE_MARBLEWIDGET

#ifdef HAVE_MARBLEWIDGET
    for (GPSInfoList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it)
    {
        const MarkerClusterHolder::MarkerInfo marker = MarkerClusterHolder::MarkerInfo::fromData<GPSInfo>(*it);
        d->markerClusterHolder->addMarker(marker);
    }
#endif // HAVE_MARBLEWIDGET

#if 0
    return;
    
    // To place mark over a map in marble canvas, we will use KML data

    QDomDocument       kmlDocument;
    QDomImplementation impl;
    QDomProcessingInstruction instr = kmlDocument.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    kmlDocument.appendChild(instr);
    QDomElement kmlRoot = kmlDocument.createElementNS( "http://earth.google.com/kml/2.1","kml");
    kmlDocument.appendChild(kmlRoot);

    QDomElement kmlAlbum = addKmlElement(kmlDocument, kmlRoot, "Document");
    QDomElement kmlName  = addKmlTextElement(kmlDocument, kmlAlbum, "name", "Geolocation");

    if (!list.isEmpty())
    {
        for (GPSInfoList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it)
        {
            QDomElement kmlPlacemark = addKmlElement(kmlDocument, kmlAlbum, "Placemark");
            addKmlTextElement(kmlDocument, kmlPlacemark, "name", (*it).url.fileName());

            QDomElement kmlGeometry  = addKmlElement(kmlDocument, kmlPlacemark, "Point");
            addKmlTextElement(kmlDocument, kmlGeometry, "coordinates", QString("%1,%2")
                            .arg((*it).longitude)
                            .arg((*it).latitude));
            addKmlTextElement(kmlDocument, kmlGeometry, "altitudeMode", "clampToGround");
            addKmlTextElement(kmlDocument, kmlGeometry, "extrude", "1");

            QDomElement kmlTimeStamp = addKmlElement(kmlDocument, kmlPlacemark, "TimeStamp");
            addKmlTextElement(kmlDocument, kmlTimeStamp, "when",
                              (*it).dateTime.toString("yyyy-MM-ddThh:mm:ssZ"));
        }
    }

#ifdef HAVE_MARBLEWIDGET

#ifdef MARBLE_VERSION

    // For Marble > 0.5.1
    if (!list.isEmpty())
    {
#if MARBLE_VERSION >= 0x00800
        d->marbleWidget->addPlacemarkData(kmlDocument.toString());
#else
        d->marbleWidget->addPlaceMarkData(kmlDocument.toString(), "digikam");
#endif
    }
#else // MARBLE_VERSION

    // For Marble 0.5.1, there is no method to place a mark over the map using string.
    // The only way is to use a temp file with all KML information.
    KTemporaryFile KMLFile;
    KMLFile.setSuffix(".kml");
    KMLFile.setAutoRemove(true);
    KMLFile.open();
    QFile file(KMLFile.fileName());
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);
    stream << kmlDocument.toString();
    file.close();

    d->marbleWidget->addPlaceMarkFile(KMLFile.fileName());

#endif // MARBLE_VERSION

#endif // HAVE_MARBLEWIDGET
#endif // 0
}

QDomElement WorldMapWidget::addKmlElement(QDomDocument& kmlDocument, QDomElement& target, const QString& tag)
{
    QDomElement kmlElement = kmlDocument.createElement(tag);
    target.appendChild(kmlElement);
    return kmlElement;
}

QDomElement WorldMapWidget::addKmlTextElement(QDomDocument& kmlDocument, QDomElement& target,
                                              const QString& tag, const QString& text)
{
    QDomElement kmlElement  = kmlDocument.createElement(tag);
    target.appendChild(kmlElement);
    QDomText kmlTextElement = kmlDocument.createTextNode(text);
    kmlElement.appendChild(kmlTextElement);
    return kmlElement;
}

void WorldMapWidget::slotZoomIn()
{
#ifdef HAVE_MARBLEWIDGET
    d->marbleWidget->zoomIn();
    d->marbleWidget->repaint();
#endif // HAVE_MARBLEWIDGET
}

void WorldMapWidget::slotZoomOut()
{
#ifdef HAVE_MARBLEWIDGET
    d->marbleWidget->zoomOut();
    d->marbleWidget->repaint();
#endif // HAVE_MARBLEWIDGET
}

void WorldMapWidget::getCenterPosition(double& lat, double& lng)
{
#ifdef HAVE_MARBLEWIDGET
    lat = d->marbleWidget->centerLatitude();
    lng = d->marbleWidget->centerLongitude();
#else // HAVE_MARBLEWIDGET
    // GPS location : Paris
    lat = 48.850258199721495;
    lng = 2.3455810546875;
#endif // HAVE_MARBLEWIDGET
}

void WorldMapWidget::setCenterPosition(double lat, double lng)
{
#ifdef HAVE_MARBLEWIDGET
    d->marbleWidget->setCenterLatitude(lat);
    d->marbleWidget->setCenterLongitude(lng);
#else // HAVE_MARBLEWIDGET
    Q_UNUSED(lat)
    Q_UNUSED(lng)
#endif // HAVE_MARBLEWIDGET
}

int WorldMapWidget::getZoomLevel()
{
#ifdef HAVE_MARBLEWIDGET
    return d->marbleWidget->zoom();
#else // HAVE_MARBLEWIDGET
    return 5;
#endif // HAVE_MARBLEWIDGET
}

void WorldMapWidget::setZoomLevel(int l)
{
#ifdef HAVE_MARBLEWIDGET
    d->marbleWidget->zoomView(l);
#else // HAVE_MARBLEWIDGET
    Q_UNUSED(l)
#endif // HAVE_MARBLEWIDGET
}

void WorldMapWidget::readConfig(KConfigGroup& group)
{
    setMapTheme(MapTheme(group.readEntry("Map Theme", int(AtlasMap))));
    setZoomLevel(group.readEntry("Zoom Level", 5));
    const bool multiMarkerShowSingleImages = group.readEntry("Preview single images", true);
    const bool multiMarkerShowGroupImages = group.readEntry("Preview grouped images", false);
    const bool multiMarkerShowHighestRatingFirst = group.readEntry("Show highest rated first", true);
    const bool multiMarkerShowOldestFirst = group.readEntry("Show oldest first", false);
    const bool multiMarkerShowNumbers = group.readEntry("Show numbers", true);
    setMultiMarkerSettings(multiMarkerShowSingleImages, multiMarkerShowGroupImages,
                           multiMarkerShowHighestRatingFirst, multiMarkerShowOldestFirst,
                           multiMarkerShowNumbers);
    // Default GPS location : Paris
    setCenterPosition(group.readEntry("Latitude",  48.850258199721495),
                      group.readEntry("Longitude", 2.3455810546875));

    emit signalSettingsChanged();
}

void WorldMapWidget::writeConfig(KConfigGroup& group)
{
    group.writeEntry("Map Theme",  int(getMapTheme()));
    group.writeEntry("Zoom Level", getZoomLevel());
    group.writeEntry("Preview single images", d->multiMarkerShowSingleImages);
    group.writeEntry("Preview grouped images", d->multiMarkerShowGroupImages);
    group.writeEntry("Show highest rated first", d->multiMarkerShowHighestRatingFirst);
    group.writeEntry("Show oldest first", d->multiMarkerShowOldestFirst);
    group.writeEntry("Show numbers", d->multiMarkerShowNumbers);
    double lat, lng;
    getCenterPosition(lat, lng);
    group.writeEntry("Latitude",  lat);
    group.writeEntry("Longitude", lng);
}

void WorldMapWidget::setMapTheme(MapTheme theme)
{
    d->mapTheme = theme;

#ifdef HAVE_MARBLEWIDGET

#ifdef MARBLE_VERSION
    switch(theme)
    {
        case OpenStreetMap:
            d->marbleWidget->setMapThemeId("earth/openstreetmap/openstreetmap.dgml");
            break;
        default: // AtlasMap
            d->marbleWidget->setMapThemeId("earth/srtm/srtm.dgml");
            break;
    }
#endif // MARBLE_VERSION

#endif // HAVE_MARBLEWIDGET
}

void WorldMapWidget::slotSetSelectedImages(const GPSInfoList &infoList)
{
#ifdef HAVE_MARBLEWIDGET
  if (infoList.isEmpty())
  {
      // clear any selections:
      d->markerClusterHolder->clearSelection();
      return;
  }
  
  QList<MarkerClusterHolder::MarkerInfo> markers;
  for (GPSInfoList::const_iterator it = infoList.constBegin(); it!=infoList.constEnd(); ++it)
  {
      const MarkerClusterHolder::MarkerInfo marker = MarkerClusterHolder::MarkerInfo::fromData<GPSInfo>(*it);
      markers << marker;
  }
  d->markerClusterHolder->setSelectedMarkers(markers, true, true);
#else
  Q_UNUSED(infoList)
#endif // HAVE_MARBLEWIDGET
}

void WorldMapWidget::slotMapMarkerSelectionChanged()
{
#ifdef HAVE_MARBLEWIDGET
    const MarkerClusterHolder::MarkerInfo::List markerList = d->markerClusterHolder->selectedMarkers();
//     kDebug(50003)<<"markerList.size():"<<markerList.size();
    GPSInfoList gpsList;
    for (MarkerClusterHolder::MarkerInfoList::const_iterator it = markerList.constBegin(); it!=markerList.constEnd(); ++it)
    {
        gpsList << it->data<GPSInfo>();
    }
    emit(signalSelectedItems(gpsList));
#endif // HAVE_MARBLEWIDGET
}

void WorldMapWidget::slotMapMarkerSoloChanged()
{
#ifdef HAVE_MARBLEWIDGET
    const MarkerClusterHolder::MarkerInfo::List markerList = d->markerClusterHolder->soloMarkers();
//     kDebug(50003)<<"markerList.size():"<<markerList.size();
    GPSInfoList gpsList;
    for (MarkerClusterHolder::MarkerInfoList::const_iterator it = markerList.constBegin(); it!=markerList.constEnd(); ++it)
    {
        gpsList << it->data<GPSInfo>();
    }
    emit(signalSoloItems(gpsList));
#endif // HAVE_MARBLEWIDGET
}

void WorldMapWidget::slotSetAllowItemSelection(const bool allow)
{
#ifdef HAVE_MARBLEWIDGET
    d->markerClusterHolder->setAllowSelection(allow);
#else
    Q_UNUSED(allow)
#endif // HAVE_MARBLEWIDGET
}

void WorldMapWidget::slotSetAllowItemFiltering(const bool allow)
{
#ifdef HAVE_MARBLEWIDGET
    d->markerClusterHolder->setAllowFiltering(allow);
#else
    Q_UNUSED(allow)
#endif // HAVE_MARBLEWIDGET
}

#ifdef HAVE_MARBLEWIDGET
QString MarkerClusterHolderToolTipFunction(const int clusterIndex, MarkerClusterHolder* const mch, void* const yourdata)
{
  Q_UNUSED(yourdata)
  
  const MarkerClusterHolder::ClusterInfo& cluster = mch->cluster(clusterIndex);
  const int nMarkers = cluster.markerCount();
  if (nMarkers>3)
      return QString();
  
  QString toolText;
  for (int i=0; i<cluster.markerIndices.size(); ++i)
  {
      const GPSInfo info = mch->marker(cluster.markerIndices.at(i)).data<GPSInfo>();
      
      QString currentText = info.url.fileName();
      if (i>0)
      {
          toolText+='\n';
      }
      toolText+=currentText;
  }
  return toolText;
}
#endif // HAVE_MARBLEWIDGET

void WorldMapWidget::slotSetEnableTooltips(const bool doIt)
{
#ifdef HAVE_MARBLEWIDGET
    d->markerClusterHolder->setTooltipFunction( doIt ? MarkerClusterHolderToolTipFunction : 0, 0);
#else
    Q_UNUSED(doIt)
#endif // HAVE_MARBLEWIDGET  
}

#ifdef HAVE_MARBLEWIDGET
/**
 * @brief Callback for generating pixmaps for clusters
 *
 * This function is called by MarkerClusterHolder to ask whether we want to display a
 * pixmap instead of the default circle for a cluster.
 *
 * @param clusterIndex Index of the cluster whose pixmap is requested
 * @param mch Pointer to the MarkerClusterHolder requesting the pixmap
 * @param maxSize Maximum size of the pixmap
 * @param yourdata Pointer to an instance of WorldMapWidget
 * @param clusterPixmap Pointer to a QPixmap to hold the pixmap for the cluster
 * @return true if a pixmap was generated, false otherwise
 */
MarkerClusterHolder::PixmapOperations WorldMapWidget::getClusterPixmap(const int clusterIndex, MarkerClusterHolder* const mch, const QSize& maxSize, void* const yourdata, QPixmap* const clusterPixmap)
{
    WorldMapWidget* const me = reinterpret_cast<WorldMapWidget*>(yourdata);
    
    MarkerClusterHolder::ClusterInfo& cluster = mch->cluster(clusterIndex);
    ClusterUserData clusterUserData = cluster.userData.value<ClusterUserData>();
    
    // build a simple hash to check whether the settings for thumbnails have changed
    const int thumbnailSettingsHash = 
        (me->d->multiMarkerShowHighestRatingFirst ? 2 : 0) + 
        (me->d->multiMarkerShowOldestFirst ? 1 : 0);

    // do we show multiple images?
    if ((cluster.markerCount()>1)&&!me->d->multiMarkerShowGroupImages)
        return MarkerClusterHolder::PixmapInvalid;
    
    // do we show single images?
    if ((cluster.markerCount()==1)&&!me->d->multiMarkerShowSingleImages)
        return MarkerClusterHolder::PixmapInvalid;
    
    MarkerClusterHolder::MarkerInfo markerInfo;
    MarkerClusterHolder::PixmapOperations pixmapOperations = MarkerClusterHolder::PixmapInvalid;
    if (cluster.markerCount()==1)
    {
        markerInfo = mch->marker(cluster.markerIndices.first());
        pixmapOperations|= MarkerClusterHolder::PixmapNoAddNumber;
    }
    else if ((clusterUserData.thumbnailMarkerIndex>=0)&&(clusterUserData.thumbnailSettingsHash==thumbnailSettingsHash))
    {
        // the index of the thumbnail-marker is cached
        markerInfo = mch->marker(clusterUserData.thumbnailMarkerIndex);
        if (!me->d->multiMarkerShowNumbers)
        {
          pixmapOperations|= MarkerClusterHolder::PixmapNoAddNumber;
        }
    }
    else
    {
        // determine which marker to show:
        QList<int> markerIndices = cluster.markerIndices;
        
        // find the oldest/youngest image respecting the rating:
        QDateTime maxDate[6];
        int maxIndex[6];
        for (int i = 0; i<=5; ++i)
        {
            maxDate[i] = QDateTime();
            maxIndex[i] = -1;
        }
        for (QList<int>::const_iterator it = markerIndices.constBegin(); it!=markerIndices.constEnd(); ++it)
        {
            const GPSInfo thisInfo = mch->marker(*it).data<GPSInfo>();
            const QDateTime thisDate = thisInfo.dateTime;
            // TODO: sometimes rating is -1. What does it mean?
            // restrict range to [0..5]
            const int thisRating = qMin(qMax(0, thisInfo.rating), 5);

            bool isBetter = maxIndex[thisRating]<0;
            if ( !isBetter )
            {
                if (me->d->multiMarkerShowOldestFirst)
                {
                    isBetter = thisDate < maxDate[thisRating];
                }
                else
                {
                    isBetter = maxDate[thisRating] < thisDate;
                }
            }
            if (isBetter)
            {
                maxIndex[thisRating] = *it;
                maxDate[thisRating] = thisDate;
            }
        }
        
//         kDebug(50003)<<QString("nMarkers=%1").arg(markerList->count());
//         for (int i=0; i<=5; ++i)
//         {
//             kDebug(50003)<<QString("rating: %1, bestdate: %2, index: %3").arg(i).arg(maxDate[i].toString()).arg(maxIndex[i]);
//         }
        
        // now pick the best match:
        int bestIndex = -1;
        if (me->d->multiMarkerShowHighestRatingFirst)
        {
            for (int i=5; i>=0; --i)
            {
                if (maxIndex[i]>=0)
                {
                    bestIndex = maxIndex[i];
                    break;
                }
            }
        }
        else
        {
            QDateTime bestDate;
            for (int i = 0; i<=5; ++i)
            {
                if (maxIndex[i]<0)
                    continue;
                
                bool isBetter = bestIndex < 0;
                if (!isBetter)
                {
                    if (me->d->multiMarkerShowOldestFirst)
                    {
                        isBetter = bestDate < maxDate[i];
                    }
                    else
                    {
                        isBetter = maxDate[i] < bestDate;
                    }
                }
                if (isBetter)
                {
                    bestDate = maxDate[i];
                    bestIndex = maxIndex[i];
                }
            }
        }
        
//         kDebug(50003)<<QString("bestIndex: %1").arg(bestIndex);
        if (bestIndex>=0)
        {
            markerInfo = mch->marker(bestIndex);
            if (!me->d->multiMarkerShowNumbers)
            {
              pixmapOperations|= MarkerClusterHolder::PixmapNoAddNumber;
            }
            clusterUserData.thumbnailMarkerIndex = bestIndex;
            clusterUserData.thumbnailSettingsHash = thumbnailSettingsHash;
            cluster.userData.setValue(clusterUserData);
        }
        else
        {
            // TODO: algorithm not checked thoroughly enough, make sure we at least don't crash:
            kDebug(50003)<<"Index invalid";
            return MarkerClusterHolder::PixmapInvalid;
        }
    }
    const GPSInfo gpsInfo = markerInfo.data<GPSInfo>();
    
    // determine the maximum size of the pixmap:
    QSize imageSize = gpsInfo.dimensions;
    int maxOfWidths = qMin(maxSize.width(), maxSize.height());
    // size returned for the image can be 0,0. in that case be conservative about the size:
    // TODO: why is 0,0 returned in some cases? maybe the image has never been loaded before?
    if (!imageSize.isNull())
    {
//         kDebug(50003)<<imageSize;
        imageSize.scale(maxSize, Qt::KeepAspectRatio);
        maxOfWidths = qMax(imageSize.width(), imageSize.height());
//         kDebug(50003)<<maxSize<<imageSize<<maxOfWidths;
    }
    bool havePixmap = me->d->thumbnailLoadThread->find(gpsInfo.url.path() , *clusterPixmap, maxOfWidths);
    if (havePixmap)
    {
        pixmapOperations|= MarkerClusterHolder::PixmapValid;
        return pixmapOperations;
    }
    
    return MarkerClusterHolder::PixmapInvalid;
}
#endif // HAVE_MARBLEWIDGET

/**
 * @brief Gets called when a new thumbnail is available from ThumbnailLoadThread
 */
void WorldMapWidget::slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& pix)
{
    Q_UNUSED(loadingDescription)
    Q_UNUSED(pix)
    
#ifdef HAVE_MARBLEWIDGET
    // tell the map to redraw itself, but only after 200ms in case more
    // thumbnails come in
    if (!d->thumbnailLoadThreadBuncher->isActive())
    {
        d->thumbnailLoadThreadBuncher->start(200);
    }
#endif // HAVE_MARBLEWIDGET
}

#ifdef HAVE_MARBLEWIDGET
/**
 * @brief Simple forwarding function to set the custom painting function in MarkerClusterHolder
 */
void WorldMapWidget::setCustomPaintFunction(const MarkerClusterHolder::CustomPaintFunction customPaintFunction, void* const yourdata)
{
    d->markerClusterHolder->setCustomPaintFunction(customPaintFunction, yourdata);
}
#endif // HAVE_MARBLEWIDGET

// ------------------------------------------------------------------------

void WorldMapWidget::setMultiMarkerSettings(const bool showSingleImages, const bool showGroupImages, const bool showHighestRatingFirst, const bool showOldestFirst, const bool showNumbers)
{
    d->multiMarkerShowSingleImages = showSingleImages;
    d->multiMarkerShowGroupImages = showGroupImages;
    d->multiMarkerShowHighestRatingFirst = showHighestRatingFirst;
    d->multiMarkerShowOldestFirst = showOldestFirst;
    d->multiMarkerShowNumbers = showNumbers;
#ifdef HAVE_MARBLEWIDGET
    d->marbleWidget->update();
#endif // HAVE_MARBLEWIDGET
}

void WorldMapWidget::getMultiMarkerSettings(bool* const showSingleImages, bool* const showGroupImages, bool* const showHighestRatingFirst, bool* const showOldestFirst, bool* const showNumbers) const
{
    *showSingleImages = d->multiMarkerShowSingleImages;
    *showGroupImages = d->multiMarkerShowGroupImages;
    *showHighestRatingFirst = d->multiMarkerShowHighestRatingFirst;
    *showOldestFirst = d->multiMarkerShowOldestFirst;
    *showNumbers = d->multiMarkerShowNumbers;
}

WorldMapWidget::MapTheme WorldMapWidget::getMapTheme()
{
    return d->mapTheme;
}

class WorldMapThemeBtnPriv
{

public:

    WorldMapThemeBtnPriv()
    : atlasMapAction(0),
      openStreetMapAction(0),
      multiMarkerShowSingleImagesAction(0),
      multiMarkerShowGroupImagesAction(0),
      multiMarkerShowHighestRatingFirstAction(0),
      multiMarkerShowYoungestFirstAction(0),
      multiMarkerShowOldestFirstAction(0),
      multiMarkerShowNumbersAction(0),
      mapThemeMenu(0),
      map(0)
    {
    };

    QAction*        atlasMapAction;
    QAction*        openStreetMapAction;
    
    QAction*        multiMarkerShowSingleImagesAction;
    QAction*        multiMarkerShowGroupImagesAction;
    QAction*        multiMarkerShowHighestRatingFirstAction;
    QAction*        multiMarkerShowYoungestFirstAction;
    QAction*        multiMarkerShowOldestFirstAction;
    QAction*        multiMarkerShowNumbersAction;

    KMenu*          mapThemeMenu;

    WorldMapWidget* map;
};

WorldMapThemeBtn::WorldMapThemeBtn(WorldMapWidget *map, QWidget *parent)
                : QToolButton(parent), d(new WorldMapThemeBtnPriv)
{
    d->map          = map;
    d->mapThemeMenu = new KMenu(this);
    setToolTip(i18n("Map settings"));
    setIcon(SmallIcon("applications-internet"));
    setMenu(d->mapThemeMenu);
    setPopupMode(QToolButton::InstantPopup);
    
    // settings for map type:
    d->atlasMapAction      = d->mapThemeMenu->addAction(i18n("Atlas"));
    d->atlasMapAction->setCheckable(true);
    d->openStreetMapAction = d->mapThemeMenu->addAction(i18n("OpenStreetMap"));
    d->openStreetMapAction->setCheckable(true);
    QActionGroup* const actionGroupMapType = new QActionGroup(this);
    actionGroupMapType->setExclusive(true);
    actionGroupMapType->addAction(d->atlasMapAction);
    actionGroupMapType->addAction(d->openStreetMapAction);
    
    d->mapThemeMenu->addSeparator();
    
    // settings for preview of the images in the clusters:
    d->multiMarkerShowSingleImagesAction = d->mapThemeMenu->addAction(i18n("Previews single images"));
    d->multiMarkerShowSingleImagesAction->setCheckable(true);
    d->multiMarkerShowGroupImagesAction = d->mapThemeMenu->addAction(i18n("Preview grouped images"));
    d->multiMarkerShowGroupImagesAction->setCheckable(true);
    
    d->mapThemeMenu->addSeparator();
    
    // settings for preview of grouped images:
    d->multiMarkerShowHighestRatingFirstAction = d->mapThemeMenu->addAction(i18n("Show highest rated first"));
    d->multiMarkerShowHighestRatingFirstAction->setCheckable(true);
    d->multiMarkerShowYoungestFirstAction = d->mapThemeMenu->addAction(i18n("Show youngest first"));
    d->multiMarkerShowYoungestFirstAction->setCheckable(true);
    d->multiMarkerShowOldestFirstAction = d->mapThemeMenu->addAction(i18n("Show oldest first"));
    d->multiMarkerShowOldestFirstAction->setCheckable(true);
    QActionGroup* const actionGroupDateSort = new QActionGroup(this);
    actionGroupDateSort->setExclusive(true);
    actionGroupDateSort->addAction(d->multiMarkerShowYoungestFirstAction);
    actionGroupDateSort->addAction(d->multiMarkerShowOldestFirstAction);
    d->multiMarkerShowNumbersAction = d->mapThemeMenu->addAction(i18n("Show numbers"));
    d->multiMarkerShowNumbersAction->setCheckable(true);
    

    connect(d->mapThemeMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotMapThemeChanged(QAction*)));

    connect(d->map, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotUpdateMenu()));
}

WorldMapThemeBtn::~WorldMapThemeBtn()
{
    delete d;
}

void WorldMapThemeBtn::slotMapThemeChanged(QAction *action)
{
    if (action == d->atlasMapAction)
    {
        d->map->setMapTheme(WorldMapWidget::AtlasMap);
    }
    else if (action == d->openStreetMapAction)
    {
        d->map->setMapTheme(WorldMapWidget::OpenStreetMap);
    }
    else if (  (action == d->multiMarkerShowSingleImagesAction)
            || (action == d->multiMarkerShowGroupImagesAction)
            || (action == d->multiMarkerShowHighestRatingFirstAction)
            || (action == d->multiMarkerShowYoungestFirstAction)
            || (action == d->multiMarkerShowOldestFirstAction)
            || (action == d->multiMarkerShowNumbersAction)
            )
            
    {
        d->map->setMultiMarkerSettings(
            d->multiMarkerShowSingleImagesAction->isChecked(),
            d->multiMarkerShowGroupImagesAction->isChecked(),
            d->multiMarkerShowHighestRatingFirstAction->isChecked(),
            d->multiMarkerShowOldestFirstAction->isChecked(),
            d->multiMarkerShowNumbersAction->isChecked()
          );
    }
    slotUpdateMenu();
}

void WorldMapThemeBtn::slotUpdateMenu()
{
    // update the map theme
    const WorldMapWidget::MapTheme currentMapTheme = d->map->getMapTheme();
    d->atlasMapAction->setChecked( currentMapTheme == WorldMapWidget::AtlasMap );
    d->openStreetMapAction->setChecked( currentMapTheme == WorldMapWidget::OpenStreetMap );
    
    // update the preview settings for the clusters
    bool multiMarkerShowSingleImages(false);
    bool multiMarkerShowGroupImages(false);
    bool multiMarkerShowHighestRatingFirst(false);
    bool multiMarkerShowOldestFirst(false);
    bool multiMarkerShowNumbers(false);
    d->map->getMultiMarkerSettings(
          &multiMarkerShowSingleImages,
          &multiMarkerShowGroupImages,
          &multiMarkerShowHighestRatingFirst,
          &multiMarkerShowOldestFirst,
          &multiMarkerShowNumbers
        );
   d->multiMarkerShowSingleImagesAction->setChecked(multiMarkerShowSingleImages);
   d->multiMarkerShowGroupImagesAction->setChecked(multiMarkerShowGroupImages);
   d->multiMarkerShowHighestRatingFirstAction->setChecked(multiMarkerShowHighestRatingFirst);
   d->multiMarkerShowYoungestFirstAction->setChecked(!multiMarkerShowOldestFirst);
   d->multiMarkerShowOldestFirstAction->setChecked(multiMarkerShowOldestFirst);
   d->multiMarkerShowNumbersAction->setChecked(multiMarkerShowNumbers);
   
   d->multiMarkerShowHighestRatingFirstAction->setEnabled(multiMarkerShowGroupImages);
   d->multiMarkerShowYoungestFirstAction->setEnabled(multiMarkerShowGroupImages);
   d->multiMarkerShowOldestFirstAction->setEnabled(multiMarkerShowGroupImages);
   d->multiMarkerShowNumbersAction->setEnabled(multiMarkerShowGroupImages);
}

}  // namespace Digikam
