/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-30
 * Description : GPS search sidebar tab contents.
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "gpssearchview.moc"

// Qt includes

#include <QFrame>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>
#include <QSplitter>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <khbox.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdebug.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "imageinfo.h"
#include "imageinfojob.h"
#include "searchxml.h"
#include "searchtextbar.h"
#include "gpssearchwidget.h"
#include "gpssearchfolderview.h"

namespace Digikam
{

class GPSSearchViewPriv
{

public:

    GPSSearchViewPriv() :
        saveBtn(0),
        zoomInBtn(0),
        zoomOutBtn(0),
        nameEdit(0),
        imageInfoJob(),
        searchGPSBar(0),
        gpsSearchFolderView(0),
        splitter(0),
        gpsSearchWidget(0),
        mapThemeBtn(0)
    {}

    QToolButton*         saveBtn;
    QToolButton*         zoomInBtn;
    QToolButton*         zoomOutBtn;

    KLineEdit*           nameEdit;

    ImageInfoJob         imageInfoJob;

    SearchTextBar*       searchGPSBar;

    GPSSearchFolderView* gpsSearchFolderView;

    QSplitter*           splitter;

    GPSSearchWidget*     gpsSearchWidget;
    WorldMapThemeBtn*    mapThemeBtn;
};

GPSSearchView::GPSSearchView(QWidget *parent)
             : QWidget(parent), d(new GPSSearchViewPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAcceptDrops(true);

    // ---------------------------------------------------------------

    QVBoxLayout *vlay  = new QVBoxLayout(this);

    QFrame *mapPanel   = new QFrame(this);
    QVBoxLayout *vlay2 = new QVBoxLayout(mapPanel);
    d->gpsSearchWidget = new GPSSearchWidget(mapPanel);
    d->gpsSearchWidget->setWhatsThis(i18n("To perform a search over the map, use CTRL+left mouse button "
                                          "to draw a rectangle where you want to find items.\n\n"
                                          "Once you have found items, click on an item using "
                                          "SHIFT+left mouse button to select it, "
                                          "click using CTRL+left mouse button to filter using an item "
                                          "(add SHIFT to filter using multiple items) and click "
                                          "using CTRL+right mouse button to zoom into an item."
                                          ));

    mapPanel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    mapPanel->setLineWidth(style()->pixelMetric(QStyle::PM_DefaultFrameWidth));

    vlay2->addWidget(d->gpsSearchWidget);
    vlay2->setMargin(0);
    vlay2->setSpacing(0);

    // ---------------------------------------------------------------

    KHBox *hbox = new KHBox(this);
    hbox->setMargin(0);
    hbox->setSpacing(KDialog::spacingHint());

    d->mapThemeBtn = new WorldMapThemeBtn(d->gpsSearchWidget, hbox);
    d->zoomOutBtn  = new QToolButton(hbox);
    d->zoomInBtn   = new QToolButton(hbox);
    d->zoomOutBtn->setIcon(SmallIcon("zoom-out"));
    d->zoomInBtn->setIcon(SmallIcon("zoom-in"));

    d->nameEdit = new KLineEdit(hbox);
    d->nameEdit->setClearButtonShown(true);
    d->nameEdit->setWhatsThis(i18n("Enter the name of the current map search to save in the "
                                   "\"My Map Searches\" view."));

    d->saveBtn  = new QToolButton(hbox);
    d->saveBtn->setIcon(SmallIcon("document-save"));
    d->saveBtn->setEnabled(false);
    d->saveBtn->setToolTip(i18n("Save current map search to a new virtual album."));
    d->saveBtn->setWhatsThis(i18n("If this button is pressed, the current map search "
                                  "will be saved to a new search "
                                  "virtual album using the name "
                                  "set on the left side."));

    // ---------------------------------------------------------------

    d->gpsSearchFolderView = new GPSSearchFolderView(this);
    d->searchGPSBar        = new SearchTextBar(this, "GPSSearchViewSearchGPSBar");

    // ---------------------------------------------------------------

    d->splitter = new QSplitter(Qt::Vertical, this);
    d->splitter->setOpaqueResize(KGlobalSettings::opaqueResize());

    QFrame* const frameTop = new QFrame(d->splitter);
    QVBoxLayout* const vlayTop = new QVBoxLayout(frameTop);
    vlayTop->addWidget(mapPanel);
    vlayTop->addWidget(hbox);
    vlayTop->setStretchFactor(mapPanel, 10);
    vlayTop->setMargin(0);
    vlayTop->setSpacing(KDialog::spacingHint());
    QFrame* const frameBottom = new QFrame(d->splitter);
    QVBoxLayout* const vlayBottom = new QVBoxLayout(frameBottom);
    vlayBottom->addWidget(d->gpsSearchFolderView);
    vlayBottom->addWidget(d->searchGPSBar);
    vlayBottom->setMargin(0);
    vlayBottom->setSpacing(KDialog::spacingHint());

    d->splitter->addWidget(frameTop);
    d->splitter->addWidget(frameBottom);

    // ---------------------------------------------------------------

    vlay->addWidget(d->splitter);

    readConfig();

    // ---------------------------------------------------------------

    connect(d->gpsSearchFolderView, SIGNAL(signalAlbumSelected(SAlbum*)),
            this, SLOT(slotAlbumSelected(SAlbum*)));

    connect(d->gpsSearchFolderView, SIGNAL(signalRenameAlbum(SAlbum*)),
            this, SLOT(slotRenameAlbum(SAlbum*)));

    connect(d->gpsSearchFolderView, SIGNAL(signalTextSearchFilterMatch(bool)),
            d->searchGPSBar, SLOT(slotSearchResult(bool)));

    connect(d->searchGPSBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            d->gpsSearchFolderView, SLOT(slotTextSearchFilterChanged(const SearchTextSettings&)));

    connect(d->saveBtn, SIGNAL(clicked()),
            this, SLOT(slotSaveGPSSAlbum()));

    connect(d->nameEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotCheckNameEditGPSConditions()));

    connect(d->nameEdit, SIGNAL(returnPressed(const QString&)),
            d->saveBtn, SLOT(animateClick()));

    connect(d->gpsSearchWidget, SIGNAL(signalNewSelectionFromMap()),
            this, SLOT(slotSelectionChanged()));

    connect(d->gpsSearchWidget, SIGNAL(signalSelectedItems(const GPSInfoList)),
            this, SLOT(slotMapSelectedItems(const GPSInfoList&)));

    connect(d->gpsSearchWidget, SIGNAL(signalSoloItems(const GPSInfoList)),
            this, SLOT(slotMapSoloItems(const GPSInfoList&)));

    connect(d->zoomInBtn, SIGNAL(released()),
            d->gpsSearchWidget, SLOT(slotZoomIn()));

    connect(d->zoomOutBtn, SIGNAL(released()),
            d->gpsSearchWidget, SLOT(slotZoomOut()));

    connect(&d->imageInfoJob, SIGNAL(signalItemsInfo(const ImageInfoList&)),
            this, SLOT(slotItemsInfo(const ImageInfoList&)));

    // ---------------------------------------------------------------

    slotCheckNameEditGPSConditions();
}

GPSSearchView::~GPSSearchView()
{
    writeConfig();
    delete d;
}

void GPSSearchView::readConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group        = config->group(QString("GPSSearch SideBar"));

    if (group.hasKey("SplitterState"))
    {
        const QByteArray splitterState = QByteArray::fromBase64(group.readEntry(QString("SplitterState"), QByteArray()));
        if (!splitterState.isEmpty())
        {
            d->splitter->restoreState(splitterState);
        }
    }

    d->gpsSearchWidget->readConfig(group);
}

void GPSSearchView::writeConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("GPSSearch SideBar"));

    group.writeEntry(QString("SplitterState"), d->splitter->saveState().toBase64());
    d->gpsSearchWidget->writeConfig(group);

    config->sync();
}

GPSSearchFolderView* GPSSearchView::folderView() const
{
    return d->gpsSearchFolderView;
}

SearchTextBar* GPSSearchView::searchBar() const
{
    return d->searchGPSBar;
}

void GPSSearchView::setActive(bool val)
{
    if (!val)
    {
        // make sure we reset the custom filters set by the MarkerClusterer:
        emit(signalMapSoloItems(KUrl::List(), "gpssearch"));
    }
    if (d->gpsSearchFolderView->selectedItem())
    {
        d->gpsSearchFolderView->setActive(val);
    }
    else if (val)
    {
        // TODO
    }
}

void GPSSearchView::slotSaveGPSSAlbum()
{
    QString name = d->nameEdit->text();
    if (!checkName(name))
        return;

    createNewGPSSearchAlbum(name);
}

void GPSSearchView::slotSelectionChanged()
{
    slotCheckNameEditGPSConditions();
    createNewGPSSearchAlbum(GPSSearchFolderView::currentGPSSearchName());
}

void GPSSearchView::createNewGPSSearchAlbum(const QString& name)
{
    AlbumManager::instance()->setCurrentAlbum(0);

    // clear positions shown on the map:
    d->gpsSearchWidget->clearGPSPositions();

    if (!d->gpsSearchWidget->hasSelection())
        return;

    // We query database here

    // NOTE: coordinates as lon1, lat1, lon2, lat2 (or West, North, East, South)
    // as left/top, right/bottom rectangle.
    QList<double> coordinates = d->gpsSearchWidget->selectionCoordinates();

    kDebug() << "West, North, East, South: " << coordinates;

    SearchXmlWriter writer;
    writer.writeGroup();
    writer.writeField("position", SearchXml::Inside);
    writer.writeAttribute("type", "rectangle");
    writer.writeValue(coordinates);
    writer.finishField();
    writer.finishGroup();

    SAlbum* salbum = AlbumManager::instance()->createSAlbum(name, DatabaseSearch::MapSearch, writer.xml());
    AlbumManager::instance()->setCurrentAlbum(salbum);
    d->imageInfoJob.allItemsFromAlbum(salbum);
}

void GPSSearchView::slotAlbumSelected(SAlbum* salbum)
{
    if (!salbum)
        return;

    // clear positions shown on the map:
    d->gpsSearchWidget->clearGPSPositions();

    AlbumManager::instance()->setCurrentAlbum(salbum);

    SearchXmlReader reader(salbum->query());
    reader.readToFirstField();
    QStringRef type = reader.attributes().value("type");

    if (type == "rectangle")
    {
        QList<double> list;
        list << reader.valueToDoubleList();
        d->gpsSearchWidget->setSelectionCoordinates(list);
        slotCheckNameEditGPSConditions();
    }

    d->imageInfoJob.allItemsFromAlbum(salbum);
}

void GPSSearchView::slotItemsInfo(const ImageInfoList& items)
{
    GPSInfoList list;
    foreach(ImageInfo inf, items)
    {
        ImagePosition pos = inf.imagePosition();
        if (!pos.isEmpty())
        {
            GPSInfo gps;
            gps.latitude  = pos.latitudeNumber();
            gps.longitude = pos.longitudeNumber();
            gps.altitude  = pos.altitude();
            gps.dateTime  = inf.dateTime();
            gps.rating    = inf.rating();
            gps.url       = inf.fileUrl();
            gps.dimensions= inf.dimensions();
            list << gps;
        }
    }
    d->gpsSearchWidget->addGPSPositions(list);
}

bool GPSSearchView::checkName(QString& name)
{
    bool checked = checkAlbum(name);

    while (!checked)
    {
        QString label = i18n( "Search name already exists.\n"
                              "Please enter a new name:" );
        bool ok;
        QString newTitle = KInputDialog::getText(i18n("Name exists"), label, name, &ok, this);
        if (!ok) return false;

        name    = newTitle;
        checked = checkAlbum(name);
    }

    return true;
}

bool GPSSearchView::checkAlbum(const QString& name) const
{
    const AlbumList list = AlbumManager::instance()->allSAlbums();

    for (AlbumList::ConstIterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        SAlbum *album = (SAlbum*)(*it);
        if ( album->title() == name )
            return false;
    }
    return true;
}

void GPSSearchView::slotCheckNameEditGPSConditions()
{
    if (d->gpsSearchWidget->hasSelection())
    {
        d->nameEdit->setEnabled(true);

        if (!d->nameEdit->text().isEmpty())
            d->saveBtn->setEnabled(true);
    }
    else
    {
        d->nameEdit->setEnabled(false);
        d->saveBtn->setEnabled(false);
    }
}

void GPSSearchView::slotRenameAlbum(SAlbum* salbum)
{
    if (!salbum) return;

    if (salbum->title() == GPSSearchFolderView::currentGPSSearchName())
        return;

    QString oldName(salbum->title());
    bool    ok;

    QString name = KInputDialog::getText(i18n("Rename Album (%1)",oldName),
                                         i18n("Enter new album name:"),
                                         oldName, &ok, this);

    if (!ok || name == oldName || name.isEmpty()) return;

    if (!checkName(name)) return;

    AlbumManager::instance()->updateSAlbum(salbum, salbum->query(), name);
}

/**
 * @brief Slot which gets called when no item is selected in the icon view
 */
void GPSSearchView::slotDigikamViewNoCurrentItem()
{
    d->gpsSearchWidget->slotSetSelectedImages(GPSInfoList());
}

/**
 * @brief Slot which gets called when the user selects images in the icon view
 * @param selectedImages List of selected images
 */
void GPSSearchView::slotDigikamViewImageSelected(const ImageInfoList &selectedImage, bool hasPrevious, bool hasNext, const ImageInfoList &allImages)
{
    Q_UNUSED(hasPrevious)
    Q_UNUSED(hasNext)
    Q_UNUSED(allImages)
    GPSInfoList list;
    foreach(ImageInfo inf, selectedImage)
    {
        ImagePosition pos = inf.imagePosition();
        if (!pos.isEmpty())
        {
            GPSInfo gps;
            gps.latitude  = pos.latitudeNumber();
            gps.longitude = pos.longitudeNumber();
            gps.altitude  = pos.altitude();
            gps.dateTime  = inf.dateTime();
            gps.rating    = inf.rating();
            gps.url       = inf.fileUrl();
            list << gps;
            kDebug()<<gps.url;
        }
    }

    d->gpsSearchWidget->slotSetSelectedImages(list);
}

/**
 * @brief Slot which gets called when the user selected items on the map
 * @param gpsList List of GPSInfos of selected items
 */
void GPSSearchView::slotMapSelectedItems(const GPSInfoList& gpsList)
{
    KUrl::List urlList;
    for (GPSInfoList::const_iterator it = gpsList.constBegin(); it!=gpsList.constEnd(); ++it)
    {
        urlList << it->url;
        kDebug()<<it->url;
    }
    emit(signalMapSelectedItems(urlList));
}

/**
 * @brief Slot which gets called when the user makes items 'solo' on the map
 * @param gpsList List of GPSInfos which are 'solo'
 */
void GPSSearchView::slotMapSoloItems(const GPSInfoList& gpsList)
{
    KUrl::List urlList;
    for (GPSInfoList::const_iterator it = gpsList.constBegin(); it!=gpsList.constEnd(); ++it)
    {
        urlList << it->url;
    }
    emit(signalMapSoloItems(urlList, "gpssearch"));
}

}  // namespace Digikam
