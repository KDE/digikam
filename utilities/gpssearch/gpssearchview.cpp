/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-30
 * Description : GPS search sidebar tab contents.
 *
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QSplitter>
#include <QStyle>
#include <QToolButton>

// KDE includes

#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <khbox.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// libkmap includes

#include <libkmap/itemmarkertiler.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "editablesearchtreeview.h"
#include "gpssearchwidget.h"
#include "imageinfo.h"
#include "imageinfojob.h"
#include "imageposition.h"
#include "searchtextbar.h"
#include "searchxml.h"
#include "gpsmarkertiler.h"
#include "mapwidgetview.h"

namespace Digikam
{

class GPSSearchViewPriv
{

public:

    GPSSearchViewPriv() :
        configSplitterStateEntry("SplitterState"),
        saveBtn(0),
        nameEdit(0),
        imageInfoJob(),
        searchGPSBar(0),
        searchTreeView(0),
        splitter(0)
    {}

    const QString               configSplitterStateEntry;

    QToolButton*                saveBtn;

    KLineEdit*                  nameEdit;

    ImageInfoJob                imageInfoJob;

    SearchTextBar*              searchGPSBar;

    EditableSearchTreeView*     searchTreeView;

    QSplitter*                  splitter;

    GPSSearchWidget*            gpsSearchWidget;

    KMapIface::KMap*            mapSearchWidget;
    GPSMarkerTiler*             gpsMarkerTiler;
   
    KAction*                    actionRemoveCurrentSelection;
    ImageAlbumModel*            imageAlbumModel;
    QItemSelectionModel*        selectionModel;
    MapViewModelHelper*         mapViewModelHelper;
    KMapIface::ItemMarkerTiler* markerTilerModelBased;
    QToolButton*                removeCurrentSelectionButton;
    bool                        existsSelection;
};

GPSSearchView::GPSSearchView(QWidget* parent, SearchModel* searchModel,
                             SearchModificationHelper* searchModificationHelper)
             : QWidget(parent), StateSavingObject(this),
               d(new GPSSearchViewPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAcceptDrops(true);

    d->imageAlbumModel    = new ImageAlbumModel(this);
    d->selectionModel     = new QItemSelectionModel(d->imageAlbumModel);
    d->mapViewModelHelper = new MapViewModelHelper(d->imageAlbumModel, d->selectionModel, this);

    d->markerTilerModelBased = new KMapIface::ItemMarkerTiler(d->mapViewModelHelper, this);

    // ---------------------------------------------------------------

    QVBoxLayout* vlay  = new QVBoxLayout(this);

    QFrame* mapPanel   = new QFrame(this);
    mapPanel->setMinimumWidth(256);
    mapPanel->setMinimumHeight(256);
    QVBoxLayout* vlay2 = new QVBoxLayout(mapPanel);
    d->mapSearchWidget = new KMapIface::KMap(mapPanel);
    d->mapSearchWidget->setBackend("marble");


    d->gpsMarkerTiler = new GPSMarkerTiler(this);
    d->mapSearchWidget->setGroupedModel(d->gpsMarkerTiler);

    mapPanel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    mapPanel->setLineWidth(style()->pixelMetric(QStyle::PM_DefaultFrameWidth));

    vlay2->addWidget(d->mapSearchWidget);
    vlay2->setMargin(0);
    vlay2->setSpacing(0);

    // ---------------------------------------------------------------

    KHBox* hbox = new KHBox(this);
    hbox->setMargin(0);
    hbox->setSpacing(KDialog::spacingHint());

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

    d->searchTreeView = new EditableSearchTreeView(this, searchModel, searchModificationHelper);
    d->searchTreeView->filteredModel()->setFilterSearchType(DatabaseSearch::MapSearch);
    d->searchTreeView->filteredModel()->setListTemporarySearches(true);
    d->searchTreeView->setAlbumManagerCurrentAlbum(true);
    d->searchGPSBar   = new SearchTextBar(this, "GPSSearchViewSearchGPSBar");
    d->searchGPSBar->setModel(d->searchTreeView->filteredModel(), AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->searchGPSBar->setFilterModel(d->searchTreeView->albumFilterModel());

    // ---------------------------------------------------------------

    d->splitter = new QSplitter(Qt::Vertical, this);
    d->splitter->setOpaqueResize(KGlobalSettings::opaqueResize());

    QFrame* const frameTop     = new QFrame(d->splitter);
    QVBoxLayout* const vlayTop = new QVBoxLayout(frameTop);
    vlayTop->addWidget(mapPanel);
    vlayTop->addWidget(d->mapSearchWidget->getControlWidget());

    d->actionRemoveCurrentSelection = new KAction(this);
    d->actionRemoveCurrentSelection->setIcon(SmallIcon(""));
    d->actionRemoveCurrentSelection->setToolTip(i18n("Removes the current selection and shows all images on the map."));
    

    d->removeCurrentSelectionButton = new QToolButton(this);
    d->removeCurrentSelectionButton->setDefaultAction(d->actionRemoveCurrentSelection);
    d->mapSearchWidget->addWidgetToControlWidget(d->removeCurrentSelectionButton);
    d->removeCurrentSelectionButton->setEnabled(false);
    d->existsSelection              = false; 

    vlayTop->addWidget(hbox);
    vlayTop->setStretchFactor(mapPanel, 10);
    vlayTop->setMargin(0);
    vlayTop->setSpacing(KDialog::spacingHint());
    QFrame* const frameBottom     = new QFrame(d->splitter);
    QVBoxLayout* const vlayBottom = new QVBoxLayout(frameBottom);
    vlayBottom->addWidget(d->searchTreeView);
    vlayBottom->addWidget(d->searchGPSBar);
    vlayBottom->setMargin(0);
    vlayBottom->setSpacing(KDialog::spacingHint());

    d->splitter->addWidget(frameTop);
    d->splitter->addWidget(frameBottom);

    // ---------------------------------------------------------------

    vlay->addWidget(d->splitter);

    // ---------------------------------------------------------------

    connect(d->actionRemoveCurrentSelection, SIGNAL(triggered()),
            this, SLOT(slotRemoveCurrentSelection()));

    connect(d->searchTreeView, SIGNAL(currentAlbumChanged(Album*)),
            this, SLOT(slotAlbumSelected(Album*)));

    connect(d->saveBtn, SIGNAL(clicked()),
            this, SLOT(slotSaveGPSSAlbum()));

    connect(d->nameEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotCheckNameEditGPSConditions()));

    connect(d->nameEdit, SIGNAL(returnPressed(const QString&)),
            d->saveBtn, SLOT(animateClick()));

    connect(d->mapSearchWidget, SIGNAL(signalNewSelectionFromMap()),
            this, SLOT(slotSelectionChanged()));

//    connect(d->gpsSearchWidget, SIGNAL(signalNewSelectionFromMap()),
//             this, SLOT(slotSelectionChanged()));
// 
//     connect(d->gpsSearchWidget, SIGNAL(signalSelectedItems(const GPSInfoList)),
//             this, SLOT(slotMapSelectedItems(const GPSInfoList&)));
// 
//     connect(d->gpsSearchWidget, SIGNAL(signalSoloItems(const GPSInfoList)),
//             this, SLOT(slotMapSoloItems(const GPSInfoList&)));
// 
//     connect(&d->imageInfoJob, SIGNAL(signalItemsInfo(const ImageInfoList&)),
//             this, SLOT(slotItemsInfo(const ImageInfoList&)));*/

    // ---------------------------------------------------------------

    slotCheckNameEditGPSConditions();
}

GPSSearchView::~GPSSearchView()
{
    delete d;
}

void GPSSearchView::setConfigGroup(KConfigGroup /*group*/)
{
    /*
    StateSavingObject::setConfigGroup(group);
    d->searchTreeView->setConfigGroup(group);
    */
}

void GPSSearchView::doLoadState()
{
   /*
   KConfigGroup group = getConfigGroup();

    if (group.hasKey(entryName(d->configSplitterStateEntry)))
    {
        const QByteArray splitterState = QByteArray::fromBase64(group.readEntry(entryName(d->configSplitterStateEntry), QByteArray()));
        if (!splitterState.isEmpty())
        {
            d->splitter->restoreState(splitterState);
        }
    }

    d->gpsSearchWidget->readConfig(group);
    d->searchTreeView->loadState();
    */
}

void GPSSearchView::doSaveState()
{
  /*
    KConfigGroup group = getConfigGroup();

    group.writeEntry(entryName(d->configSplitterStateEntry), d->splitter->saveState().toBase64());
    d->gpsSearchWidget->writeConfig(group);
    d->searchTreeView->saveState();

    group.sync();
    */
}

void GPSSearchView::setActive(bool /*val*/)
{
/*
    if (!val)
    {
        // make sure we reset the custom filters set by the MarkerClusterer:
        emit(signalMapSoloItems(KUrl::List(), "gpssearch"));
    }

    if (val && d->searchTreeView->currentAlbum())
    {
        AlbumManager::instance()->setCurrentAlbum(
                        d->searchTreeView->currentAlbum());
    }
    else if (val)
    {
        // TODO
    }
    */
}

void GPSSearchView::changeAlbumFromHistory(SAlbum* album)
{
    d->searchTreeView->slotSelectAlbum(album);
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
    d->existsSelection = true;
    slotCheckNameEditGPSConditions();
    createNewGPSSearchAlbum(SAlbum::getTemporaryTitle(DatabaseSearch::MapSearch));
}

void GPSSearchView::createNewGPSSearchAlbum(const QString& name)
{

    AlbumManager::instance()->setCurrentAlbum(0);

    // clear positions shown on the map:
    //d->gpsSearchWidget->clearGPSPositions();

    if (!d->mapSearchWidget->hasSelection())
        return;

    // We query database here

    // NOTE: coordinates as lon1, lat1, lon2, lat2 (or West, North, East, South)
    // as left/top, right/bottom rectangle.
    QList<double> coordinates = d->mapSearchWidget->selectionCoordinates();

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
    d->searchTreeView->slotSelectAlbum(salbum);
    
    d->imageAlbumModel->openAlbum(salbum);
    if(d->existsSelection) 
    {
        d->mapSearchWidget->setGroupedModel(d->markerTilerModelBased); 
        d->removeCurrentSelectionButton->setEnabled(true);
    }
}

void GPSSearchView::slotAlbumSelected(Album* a)
{

    SAlbum *salbum = dynamic_cast<SAlbum*> (a);


    if (!salbum)
        return;

    // clear positions shown on the map:
    //d->gpsSearchWidget->clearGPSPositions();

    SearchXmlReader reader(salbum->query());
    reader.readToFirstField();
    QStringRef type = reader.attributes().value("type");

    if (type == "rectangle")
    {
        QList<double> list;
        list << reader.valueToDoubleList();

        d->mapSearchWidget->setSelectionCoordinates(list);
        slotCheckNameEditGPSConditions();
    }

    d->imageInfoJob.allItemsFromAlbum(salbum);
}

void GPSSearchView::slotItemsInfo(const ImageInfoList& /*items*/)
{
/*
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
*/
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
    

    //return false;
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
    

    //return false;
}

void GPSSearchView::slotRemoveCurrentSelection()
{
    d->existsSelection = false;
    d->removeCurrentSelectionButton->setEnabled(d->existsSelection);
    d->mapSearchWidget->setGroupedModel(d->gpsMarkerTiler);
}

void GPSSearchView::slotCheckNameEditGPSConditions()
{
    //d->gpsMarkerTiler->secondTestDatabase(5.1,80.3,20.4,130.6);

    if (d->mapSearchWidget->hasSelection())
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

/**
 * @brief Slot which gets called when no item is selected in the icon view
 */
void GPSSearchView::slotDigikamViewNoCurrentItem()
{
//    d->gpsSearchWidget->slotSetSelectedImages(GPSInfoList());
}

/**
 * @brief Slot which gets called when the user selects images in the icon view
 * @param selectedImage List of selected images
 */
void GPSSearchView::slotDigikamViewImageSelected(const ImageInfoList& selectedImage, bool hasPrevious, bool hasNext, 
                                                 const ImageInfoList& allImages)
{
    Q_UNUSED(selectedImage)
    Q_UNUSED(hasPrevious)
    Q_UNUSED(hasNext)
    Q_UNUSED(allImages)
/*
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
//             kDebug()<<gps.url;
        }
    }

    d->gpsSearchWidget->slotSetSelectedImages(list);
*/
}

/**
 * @brief Slot which gets called when the user selected items on the map
 * @param gpsList List of GPSInfos of selected items
 */
void GPSSearchView::slotMapSelectedItems(const GPSInfoList& /*gpsList*/)
{
/*
    KUrl::List urlList;
    for (GPSInfoList::const_iterator it = gpsList.constBegin(); it!=gpsList.constEnd(); ++it)
    {
        urlList << it->url;
        kDebug()<<it->url;
    }
    emit(signalMapSelectedItems(urlList));
*/
}

/**
 * @brief Slot which gets called when the user makes items 'solo' on the map
 * @param gpsList List of GPSInfos which are 'solo'
 */
void GPSSearchView::slotMapSoloItems(const GPSInfoList& /*gpsList*/)
{
/*
    KUrl::List urlList;
    for (GPSInfoList::const_iterator it = gpsList.constBegin(); it!=gpsList.constEnd(); ++it)
    {
        urlList << it->url;
    }
    emit(signalMapSoloItems(urlList, "gpssearch"));
*/
}



}  // namespace Digikam
