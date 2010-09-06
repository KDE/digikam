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
#include <QTimer>
#include <QMenu>
#include <QActionGroup>
#include <QAction>

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

#include <libkmap/kmap_widget.h>
#include <libkmap/itemmarkertiler.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "editablesearchtreeview.h"
#include "imageinfo.h"
#include "imageinfojob.h"
#include "imageposition.h"
#include "searchtextbar.h"
#include "searchxml.h"
#include "gpsmarkertiler.h"
#include "mapwidgetview.h"

namespace Digikam
{

class GPSSearchView::GPSSearchViewPriv
{

public:

    GPSSearchViewPriv() :
        configSplitterStateEntry("SplitterState"),
        saveBtn(0),
        nameEdit(0),
        imageInfoJob(),
        searchGPSBar(0),
        searchTreeView(0),
        splitter(0),
        sortActionOldestFirst(),
        sortActionYoungestFirst(),
        sortActionRating()
    {}

    const QString               configSplitterStateEntry;
    QToolButton*                saveBtn;
    KLineEdit*                  nameEdit;
    ImageInfoJob                imageInfoJob;
    SearchTextBar*              searchGPSBar;
    EditableSearchTreeView*     searchTreeView;
    QSplitter*                  splitter;
    KMap::KMapWidget*           mapSearchWidget;
    GPSMarkerTiler*             gpsMarkerTiler;
    ImageAlbumModel*            imageAlbumModel;
    ImageFilterModel*           imageFilterModel;
    QItemSelectionModel*        selectionModel;
    MapViewModelHelper*         mapViewModelHelper;
    KMap::ItemMarkerTiler*      markerTilerModelBased;
    bool                        existsSelection;
    SearchModel*                searchModel;
    KAction*                    sortActionOldestFirst;
    KAction*                    sortActionYoungestFirst;
    KAction*                    sortActionRating; 
};

/**
 * @brief Constructor
 * @param parent Parent object.
 * @param searchModel The model that stores the searches.
 * @param imageFilterModel The image model used by displaying the selected images on map.
 * @param itemSelectionModel The selection model corresponding to the imageFilterModel.
 */
GPSSearchView::GPSSearchView(QWidget* parent, SearchModel* searchModel,
                             SearchModificationHelper* searchModificationHelper,
                             ImageFilterModel* imageFilterModel, QItemSelectionModel* itemSelectionModel)
             : QWidget(parent), StateSavingObject(this),
               d(new GPSSearchViewPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAcceptDrops(true);

    d->imageAlbumModel       = qobject_cast<ImageAlbumModel*>(imageFilterModel->sourceModel());      
    d->selectionModel        = itemSelectionModel;
    d->imageFilterModel      = imageFilterModel;
    d->searchModel           = searchModel;

    d->mapViewModelHelper    = new MapViewModelHelper(d->selectionModel, d->imageFilterModel, this);
    d->markerTilerModelBased = new KMap::ItemMarkerTiler(d->mapViewModelHelper, this);

    // ---------------------------------------------------------------

    QVBoxLayout* vlay  = new QVBoxLayout(this);

    QFrame* mapPanel   = new QFrame(this);
    mapPanel->setMinimumWidth(256);
    mapPanel->setMinimumHeight(256);
    QVBoxLayout* vlay2 = new QVBoxLayout(mapPanel);
    d->mapSearchWidget = new KMap::KMapWidget(mapPanel);
    d->mapSearchWidget->setBackend("marble");

    d->gpsMarkerTiler = new GPSMarkerTiler(this);
    d->mapSearchWidget->setGroupedModel(d->gpsMarkerTiler);

    mapPanel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    mapPanel->setLineWidth(style()->pixelMetric(QStyle::PM_DefaultFrameWidth));

    QMenu* const sortMenu = new QMenu(this);
    sortMenu->setTitle(i18n("Sorting"));
    QActionGroup* const sortOrderExclusive = new QActionGroup(sortMenu);
    sortOrderExclusive->setExclusive(true);
    connect(sortOrderExclusive, SIGNAL(triggered(QAction*)),
            this, SLOT(slotSortOptionTriggered(QAction*)));

    d->sortActionOldestFirst = new KAction(i18n("Show oldest first"), sortOrderExclusive);
    d->sortActionOldestFirst->setCheckable(true);
    sortMenu->addAction(d->sortActionOldestFirst);

    d->sortActionYoungestFirst = new KAction(i18n("Show youngest first"), sortOrderExclusive);
    d->sortActionYoungestFirst->setCheckable(true);
    sortMenu->addAction(d->sortActionYoungestFirst);

    d->sortActionRating = new KAction(i18n("Sort by rating"), sortOrderExclusive);
    d->sortActionRating->setCheckable(true);
    sortMenu->addAction(d->sortActionRating);

    d->mapSearchWidget->setSortOptionsMenu(sortMenu);
 

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

    d->searchTreeView = new EditableSearchTreeView(this, d->searchModel, searchModificationHelper);
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
    d->mapSearchWidget->setAvailableMouseModes(KMap::MouseModePan|KMap::MouseModeSelection|KMap::MouseModeZoom|KMap::MouseModeFilter|KMap::MouseModeSelectThumbnail);
    d->mapSearchWidget->setVisibleMouseModes(KMap::MouseModePan|KMap::MouseModeSelection|KMap::MouseModeZoom|KMap::MouseModeFilter|KMap::MouseModeSelectThumbnail);

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

    connect(d->mapSearchWidget, SIGNAL(signalRemoveCurrentSelection()),
            this, SLOT(slotRemoveCurrentSelection()));

    connect(d->mapViewModelHelper, SIGNAL(signalFilteredImages(const QList<qlonglong>&)),
            this, SLOT(slotMapSoloItems(const QList<qlonglong>&)));

    connect(d->mapSearchWidget, SIGNAL(signalRemoveCurrentFilter()),
            this, SLOT(slotRemoveCurrentFilter()));

    // ---------------------------------------------------------------

    slotCheckNameEditGPSConditions();
}

/**
 * @brief Destructor
 */
GPSSearchView::~GPSSearchView()
{
    delete d;
}

void GPSSearchView::setConfigGroup(KConfigGroup group)
{
    StateSavingObject::setConfigGroup(group);
    d->searchTreeView->setConfigGroup(group);
}

void GPSSearchView::doLoadState()
{
   KConfigGroup group = getConfigGroup();

    if (group.hasKey(entryName(d->configSplitterStateEntry)))
    {
        const QByteArray splitterState = QByteArray::fromBase64(group.readEntry(entryName(d->configSplitterStateEntry), QByteArray()));
        if (!splitterState.isEmpty())
        {
            d->splitter->restoreState(splitterState);
        }
    }

    const KConfigGroup groupMapWidget = KConfigGroup(&group, "GPSSearch Map Widget");
    d->mapSearchWidget->readSettingsFromGroup(&groupMapWidget);
    d->searchTreeView->loadState();

    d->searchTreeView->clearSelection();
    d->imageAlbumModel->clearImageInfos();
}

void GPSSearchView::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    group.writeEntry(entryName(d->configSplitterStateEntry), d->splitter->saveState().toBase64());

    KConfigGroup groupMapWidget = KConfigGroup(&group, "GPSSearch Map Widget");
    d->mapSearchWidget->saveSettingsToGroup(&groupMapWidget);
    d->searchTreeView->saveState();

    group.sync();
}

/**
 * @brief Sets the widget active or inactive. 
 * 
 * This function is called when the GPSSearch widget is switched with another widget.
 *
 * @param val When true, the widget is enabled.
 */
void GPSSearchView::setActive(bool val)
{
    if (!val)
    {
        // make sure we reset the custom filters set by the MarkerClusterer:
     //   emit(signalMapSoloItems(KUrl::List(), "gpssearch"));
        d->mapSearchWidget->setActive(false);
    }

    if (val && d->searchTreeView->currentAlbum())
    {
        d->mapSearchWidget->setActive(true);
        AlbumManager::instance()->setCurrentAlbum(
                        d->searchTreeView->currentAlbum());
    }
    else if (val)
    {
        d->mapSearchWidget->setActive(true);
        d->imageAlbumModel->clearImageInfos();
    }
}

void GPSSearchView::changeAlbumFromHistory(SAlbum* album)
{
    d->searchTreeView->slotSelectAlbum(album);
}

/**
 * This slot saves the current album.
 */
void GPSSearchView::slotSaveGPSSAlbum()
{
    QString name = d->nameEdit->text();
    if (!checkName(name))
        return;

    createNewGPSSearchAlbum(name);
}

/**
 * This slot is called when a new selection is made. It creates a new Search Album.
 */
void GPSSearchView::slotSelectionChanged()
{
    d->existsSelection = true;
    d->mapSearchWidget->setSelectionStatus(d->existsSelection);
    slotCheckNameEditGPSConditions();
    createNewGPSSearchAlbum(SAlbum::getTemporaryTitle(DatabaseSearch::MapSearch));
}

/**
 * @brief This function creates a new Search Album.
 * @param name The name of the new album.
 */
void GPSSearchView::createNewGPSSearchAlbum(const QString& name)
{
    AlbumManager::instance()->setCurrentAlbum(0);

    if (!d->mapSearchWidget->getSelectionStatus())
        return;

    // We query database here

    const KMap::GeoCoordinates::Pair coordinates = d->mapSearchWidget->getSelectionRectangle();

    // NOTE: coordinates as lon1, lat1, lon2, lat2 (or West, North, East, South)
    // as left/top, right/bottom rectangle.
    const QList<qreal> coordinatesList = QList<qreal>() <<
                                   coordinates.first.lon() << coordinates.first.lat() <<
                                   coordinates.second.lon() << coordinates.second.lat();

    kDebug() << "West, North, East, South: " << coordinatesList;

    SearchXmlWriter writer;
    writer.writeGroup();
    writer.writeField("position", SearchXml::Inside);
    writer.writeAttribute("type", "rectangle");
    writer.writeValue(coordinatesList);
    writer.finishField();
    writer.finishGroup();

    SAlbum* salbum = AlbumManager::instance()->createSAlbum(name, DatabaseSearch::MapSearch, writer.xml());
    AlbumManager::instance()->setCurrentAlbum(salbum);
    d->imageInfoJob.allItemsFromAlbum(salbum);
    d->searchTreeView->slotSelectAlbum(salbum);

    d->imageAlbumModel->openAlbum(salbum);
    if (d->existsSelection) 
    {
        d->mapSearchWidget->setGroupedModel(d->markerTilerModelBased);
    }
}

/**
 * @brief An album is selected in the saved searches list.
 * @param a This album will be selected.
 */
void GPSSearchView::slotAlbumSelected(Album* a)
{
    SAlbum* salbum = dynamic_cast<SAlbum*> (a);

    if (!salbum)
        return;

    SearchXmlReader reader(salbum->query());
    reader.readToFirstField();
    QStringRef type = reader.attributes().value("type");

    if (type == "rectangle")
    {
        const QList<double> list = reader.valueToDoubleList();

        const KMap::GeoCoordinates::Pair coordinates(KMap::GeoCoordinates(list.at(1), list.at(0)),
                                                     KMap::GeoCoordinates(list.at(3), list.at(2)));

        d->mapSearchWidget->setSelectionStatus(true);
        d->mapSearchWidget->setSelectionCoordinates(coordinates);
        slotCheckNameEditGPSConditions();
    }

    d->imageInfoJob.allItemsFromAlbum(salbum);
}

/**
 * @brief Checks whether the newly added search name already exists.
 * @param name The name of the current search.
 */
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

/**
 * @brief Checks whether the newly added album name already exists.
 * @param name The name of the album.
 */
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

/**
 * Remove the current selection rectangle and clear the selection made by it.
 */
void GPSSearchView::slotRemoveCurrentSelection()
{
    d->existsSelection = false;
    d->mapSearchWidget->setSelectionStatus(d->existsSelection);
    d->imageAlbumModel->clearImageInfos();
    d->searchTreeView->clearSelection();

    d->mapSearchWidget->setGroupedModel(d->gpsMarkerTiler);
}

/**
 * @brief Remove the current filter.
 */
void GPSSearchView::slotRemoveCurrentFilter()
{
    QList<qlonglong> emptyIdList;
    emit signalMapSoloItems(emptyIdList, "gpssearch"); 
}

/**
 * @brief Enable or disable the album saving controls.
 */
void GPSSearchView::slotCheckNameEditGPSConditions()
{
    if (d->mapSearchWidget->getSelectionStatus())
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
}

/**
 * @brief Slot which gets called when the user selected items on the map
 * @param gpsList List of GPSInfos of selected items
 */
void GPSSearchView::slotMapSelectedItems(const GPSInfoList& /*gpsList*/)
{
}

/**
 * @brief Slot which gets called when the user makes items 'solo' on the map
 * @param gpsList List of GPSInfos which are 'solo'
 */
void GPSSearchView::slotMapSoloItems(const QList<qlonglong>& idList)
{
    emit(signalMapSoloItems(idList, "gpssearch"));
}

void GPSSearchView::slotSortOptionTriggered(QAction* /*action*/)
{
    int newSortKey = SortYoungestFirst;
    if (d->sortActionYoungestFirst->isChecked())
        newSortKey = SortYoungestFirst;
    else if (d->sortActionOldestFirst->isChecked())
        newSortKey = SortOldestFirst;
    else
        newSortKey = SortRating;

    d->mapSearchWidget->setSortKey(newSortKey);
}

}  // namespace Digikam
