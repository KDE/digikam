/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-30
 * Description : GPS search sidebar tab contents.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2009 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C)      2014 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "gpssearchview.h"

// Qt includes

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QSplitter>
#include <QToolButton>
#include <QTimer>
#include <QMenu>
#include <QActionGroup>
#include <QAction>
#include <QApplication>
#include <QStyle>
#include <QLineEdit>
#include <QInputDialog>
#include <QIcon>

// Local includes

#include "digikam_debug.h"
#include "editablesearchtreeview.h"
#include "imageinfojob.h"
#include "coredbsearchxml.h"
#include "gpsmarkertiler.h"
#include "gpsimageinfosorter.h"

namespace Digikam
{

class GPSSearchView::Private
{

public:

    Private() :
        saveBtn(0),
        nameEdit(0),
        imageInfoJob(),
        searchGPSBar(0),
        searchTreeView(0),
        splitter(0),
        mapSearchWidget(0),
        gpsMarkerTiler(0),
        imageAlbumModel(0),
        imageFilterModel(0),
        selectionModel(0),
        searchModel(0),
        sortOrderOptionsHelper(0)
    {
    }

    static const QString        configSplitterStateEntry;
    QToolButton*                saveBtn;
    QLineEdit*                  nameEdit;
    ImageInfoJob                imageInfoJob;
    SearchTextBar*              searchGPSBar;
    EditableSearchTreeView*     searchTreeView;
    QSplitter*                  splitter;
    MapWidget*        mapSearchWidget;
    GPSMarkerTiler*             gpsMarkerTiler;
    ImageAlbumModel*            imageAlbumModel;
    ImageFilterModel*           imageFilterModel;
    QItemSelectionModel*        selectionModel;
    SearchModel*                searchModel;
    GPSImageInfoSorter*         sortOrderOptionsHelper;
    QString                     nonGeonlocatedItemsXml;
};

const QString GPSSearchView::Private::configSplitterStateEntry(QLatin1String("SplitterState"));

/**
 * @brief Constructor
 * @param parent Parent object.
 * @param searchModel The model that stores the searches.
 * @param imageFilterModel The image model used by displaying the selected images on map.
 * @param itemSelectionModel The selection model corresponding to the imageFilterModel.
 */
GPSSearchView::GPSSearchView(QWidget* const parent,
                             SearchModel* const searchModel,
                             SearchModificationHelper* const searchModificationHelper,
                             ImageFilterModel* const imageFilterModel,
                             QItemSelectionModel* const itemSelectionModel)
    : QWidget(parent),
      StateSavingObject(this),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    /// @todo Really?
    setAcceptDrops(true);

    d->imageAlbumModel        = qobject_cast<ImageAlbumModel*>(imageFilterModel->sourceModel());
    d->selectionModel         = itemSelectionModel;
    d->imageFilterModel       = imageFilterModel;
    d->searchModel            = searchModel;

    // ---------------------------------------------------------------

    QVBoxLayout* const vlay   = new QVBoxLayout(this);

    QFrame* const mapPanel    = new QFrame(this);
    mapPanel->setMinimumWidth(256);
    mapPanel->setMinimumHeight(256);
    QVBoxLayout* const vlay2  = new QVBoxLayout(mapPanel);
    d->mapSearchWidget        = new MapWidget(mapPanel);
    d->mapSearchWidget->setBackend(QLatin1String("marble"));
    d->mapSearchWidget->setShowThumbnails(true);

    d->gpsMarkerTiler         = new GPSMarkerTiler(this, d->imageFilterModel, d->selectionModel);
    d->mapSearchWidget->setGroupedModel(d->gpsMarkerTiler);

    mapPanel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    mapPanel->setLineWidth(style()->pixelMetric(QStyle::PM_DefaultFrameWidth));

    d->sortOrderOptionsHelper = new GPSImageInfoSorter(this);
    d->sortOrderOptionsHelper->addToMapWidget(d->mapSearchWidget);

    vlay2->addWidget(d->mapSearchWidget);
    vlay2->setContentsMargins(QMargins());
    vlay2->setSpacing(0);

    // ---------------------------------------------------------------

    DHBox* const hbox = new DHBox(this);
    hbox->setContentsMargins(QMargins());
    hbox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    d->nameEdit       = new QLineEdit(hbox);
    d->nameEdit->setClearButtonEnabled(true);
    d->nameEdit->setWhatsThis(i18n("Enter the name of the current map search to save in the "
                                   "\"Map Searches\" view."));

    d->saveBtn        = new QToolButton(hbox);
    d->saveBtn->setIcon(QIcon::fromTheme(QLatin1String("document-save")));
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
    d->searchGPSBar   = new SearchTextBar(this, QLatin1String("GPSSearchViewSearchGPSBar"));
    d->searchGPSBar->setModel(d->searchTreeView->filteredModel(), AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->searchGPSBar->setFilterModel(d->searchTreeView->albumFilterModel());

    // ---------------------------------------------------------------

    d->splitter                = new QSplitter(Qt::Vertical, this);

    QFrame* const frameTop     = new QFrame(d->splitter);
    QVBoxLayout* const vlayTop = new QVBoxLayout(frameTop);
    vlayTop->addWidget(mapPanel);
    vlayTop->addWidget(d->mapSearchWidget->getControlWidget());

    d->mapSearchWidget->setAvailableMouseModes(MouseModePan                     |
                                               MouseModeRegionSelection         |
                                               MouseModeZoomIntoGroup           |
                                               MouseModeRegionSelectionFromIcon |
                                               MouseModeFilter                  |
                                               MouseModeSelectThumbnail);

    d->mapSearchWidget->setVisibleMouseModes(MouseModePan           |
                                             MouseModeZoomIntoGroup |
                                             MouseModeFilter        |
                                             MouseModeSelectThumbnail);

    // construct a second row of control actions below the control widget
    /// @todo Should we still replace the icons of the actions with text as discussed during the sprint?
    QWidget* const secondActionRow            = new QWidget();
    QHBoxLayout* const secondActionRowHBox    = new QHBoxLayout();
    secondActionRowHBox->setContentsMargins(QMargins());
    secondActionRow->setLayout(secondActionRowHBox);

    QLabel* const secondActionRowLabel        = new QLabel(i18n("Search by area:"));
    secondActionRowHBox->addWidget(secondActionRowLabel);

    QToolButton* const tbRegionSelection      = new QToolButton(secondActionRow);
    tbRegionSelection->setDefaultAction(d->mapSearchWidget->getControlAction(QLatin1String("mousemode-regionselectionmode")));
    secondActionRowHBox->addWidget(tbRegionSelection);

    QToolButton* const tbRegionFromIcon       = new QToolButton(secondActionRow);
    tbRegionFromIcon->setDefaultAction(d->mapSearchWidget->getControlAction(QLatin1String("mousemode-regionselectionfromiconmode")));
    secondActionRowHBox->addWidget(tbRegionFromIcon);

    QToolButton* const tbClearRegionSelection = new QToolButton(secondActionRow);
    tbClearRegionSelection->setDefaultAction(d->mapSearchWidget->getControlAction(QLatin1String("mousemode-removecurrentregionselection")));
    secondActionRowHBox->addWidget(tbClearRegionSelection);

    secondActionRowHBox->addStretch(10);
    vlayTop->addWidget(secondActionRow);

    // end of the second action row

    // Show Non Geolocated Items row

    QWidget* const nonGeolocatedActionRow = new QWidget();
    QVBoxLayout* const thirdActionRowVBox = new QVBoxLayout();
    thirdActionRowVBox->setContentsMargins(QMargins());
    nonGeolocatedActionRow->setLayout(thirdActionRowVBox);

    QPushButton* const nonGeolocatedBtn   = new QPushButton(nonGeolocatedActionRow);
    nonGeolocatedBtn->setText(i18n("Show Non-Geolocated Items"));
    nonGeolocatedBtn->setIcon(QIcon::fromTheme(QLatin1String("emblem-unmounted")));
    thirdActionRowVBox->addWidget(nonGeolocatedBtn);

    thirdActionRowVBox->addStretch(10);
    vlayTop->addWidget(nonGeolocatedActionRow);

    // end of the third action row

    vlayTop->addWidget(hbox);
    vlayTop->setStretchFactor(mapPanel, 10);
    vlayTop->setContentsMargins(QMargins());
    vlayTop->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    QFrame* const frameBottom     = new QFrame(d->splitter);
    QVBoxLayout* const vlayBottom = new QVBoxLayout(frameBottom);
    vlayBottom->addWidget(d->searchTreeView);
    vlayBottom->addWidget(d->searchGPSBar);
    vlayBottom->setContentsMargins(QMargins());
    vlayBottom->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    d->splitter->addWidget(frameTop);
    d->splitter->addWidget(frameBottom);

    // ---------------------------------------------------------------

    vlay->addWidget(d->splitter);

    // ---------------------------------------------------------------

    connect(d->searchTreeView, SIGNAL(currentAlbumChanged(Album*)),
            this, SLOT(slotAlbumSelected(Album*)));

    connect(d->saveBtn, SIGNAL(clicked()),
            this, SLOT(slotSaveGPSSAlbum()));

    connect(d->nameEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotCheckNameEditGPSConditions()));

    connect(d->nameEdit, SIGNAL(returnPressed()),
            d->saveBtn, SLOT(animateClick()));

    connect(d->mapSearchWidget, SIGNAL(signalRegionSelectionChanged()),
            this, SLOT(slotRegionSelectionChanged()));

    connect(d->gpsMarkerTiler, SIGNAL(signalModelFilteredImages(QList<qlonglong>)),
            this, SLOT(slotMapSoloItems(QList<qlonglong>)));

    connect(d->mapSearchWidget, SIGNAL(signalRemoveCurrentFilter()),
            this, SLOT(slotRemoveCurrentFilter()));

    connect(nonGeolocatedBtn, SIGNAL(clicked()),
            d->mapSearchWidget->getControlAction(QLatin1String("mousemode-removecurrentregionselection")), SIGNAL(triggered()));

    connect(nonGeolocatedBtn, SIGNAL(clicked()),
            this, SLOT(showNonGeolocatedItems()));

    // ---------------------------------------------------------------

    slotCheckNameEditGPSConditions();
}

GPSSearchView::~GPSSearchView()
{
    delete d;
}

void GPSSearchView::setConfigGroup(const KConfigGroup& group)
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

    d->sortOrderOptionsHelper->setSortOptions(GPSImageInfoSorter::SortOptions(group.readEntry(entryName(QLatin1String("Sort Order")), int(d->sortOrderOptionsHelper->getSortOptions()))));

    const KConfigGroup groupMapWidget = KConfigGroup(&group, entryName(QLatin1String("GPSSearch Map Widget")));

    d->mapSearchWidget->readSettingsFromGroup(&groupMapWidget);

    d->searchTreeView->loadState();

    AlbumManager::instance()->setCurrentAlbums(QList<Album*>());

    d->searchTreeView->clearSelection();
}

void GPSSearchView::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    group.writeEntry(entryName(d->configSplitterStateEntry), d->splitter->saveState().toBase64());
    group.writeEntry(entryName(QLatin1String("Sort Order")), int(d->sortOrderOptionsHelper->getSortOptions()));

    KConfigGroup groupMapWidget = KConfigGroup(&group, entryName(QLatin1String("GPSSearch Map Widget")));
    d->mapSearchWidget->saveSettingsToGroup(&groupMapWidget);
    d->searchTreeView->saveState();

    group.sync();
}

/**
 * @brief Sets the widget active or inactive.
 *
 * Called when the GPSSearch tab becomes the current/not current tab.
 *
 * @param state When true, the widget is enabled.
 */
void GPSSearchView::setActive(bool state)
{
    if (!state)
    {
        // make sure we reset the custom filters set by the map:
        emit(signalMapSoloItems(QList<qlonglong>(), QLatin1String("gpssearch")));
        d->mapSearchWidget->setActive(false);
    }
    else
    {
        d->mapSearchWidget->setActive(true);

        if (d->searchTreeView->currentAlbum())
        {
            AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << d->searchTreeView->currentAlbum());
        }

        slotClearImages();
    }
}

void GPSSearchView::changeAlbumFromHistory(SAlbum* const album)
{
    d->searchTreeView->setCurrentAlbums(QList<Album*>() << album);
}

/**
 * This slot saves the current album.
 */
void GPSSearchView::slotSaveGPSSAlbum()
{
    QString name = d->nameEdit->text();

    if (!checkName(name))
    {
        return;
    }

    createNewGPSSearchAlbum(name);
}

/**
 * This slot is called when a new selection is made. It creates a new Search Album.
 */
void GPSSearchView::slotRegionSelectionChanged()
{
    const GeoCoordinates::Pair newRegionSelection = d->mapSearchWidget->getRegionSelection();
    const bool haveRegionSelection                          = newRegionSelection.first.hasCoordinates();

    if (haveRegionSelection)
    {
        slotCheckNameEditGPSConditions();
        createNewGPSSearchAlbum(SAlbum::getTemporaryTitle(DatabaseSearch::MapSearch));
    }
    else
    {
        // reset the search rectangle of the temporary album:
        createNewGPSSearchAlbum(SAlbum::getTemporaryTitle(DatabaseSearch::MapSearch));
        d->gpsMarkerTiler->removeCurrentRegionSelection();
        d->searchTreeView->clearSelection();
        slotClearImages();
    }

    // also remove any filters which may have been there
    slotRemoveCurrentFilter();

    slotRefreshMap();
}

/**
 * @brief This function creates a new Search Album.
 * @param name The name of the new album.
 */
void GPSSearchView::createNewGPSSearchAlbum(const QString& name)
{
    //AlbumManager::instance()->setCurrentAlbums(QList<Album*>());

    // We query the database here

    const GeoCoordinates::Pair coordinates = d->mapSearchWidget->getRegionSelection();
    const bool haveCoordinates                       = coordinates.first.hasCoordinates();

    if (haveCoordinates)
    {
        d->gpsMarkerTiler->setRegionSelection(coordinates);
    }

    // NOTE: coordinates as lon1, lat1, lon2, lat2 (or West, North, East, South)
    // as left/top, right/bottom rectangle.
    QList<qreal> coordinatesList = QList<qreal>() <<
                                   coordinates.first.lon() << coordinates.first.lat() <<
                                   coordinates.second.lon() << coordinates.second.lat();

    if (!haveCoordinates)
    {
        /// @todo We need to create a search album with invalid coordinates
        coordinatesList.clear();
        coordinatesList << -200 << -200 << -200 << -200;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "West, North, East, South: " << coordinatesList;

    SearchXmlWriter writer;
    writer.writeGroup();
    writer.writeField(QLatin1String("position"), SearchXml::Inside);
    writer.writeAttribute(QLatin1String("type"), QLatin1String("rectangle"));
    writer.writeValue(coordinatesList);
    writer.finishField();
    writer.finishGroup();

    SAlbum* const salbum = AlbumManager::instance()->createSAlbum(name, DatabaseSearch::MapSearch, writer.xml());
    AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << salbum);
    d->imageInfoJob.allItemsFromAlbum(salbum);
    d->searchTreeView->setCurrentAlbums(QList<Album*>() << salbum);
    d->imageAlbumModel->openAlbum(QList<Album*>() << salbum);
}

/**
 * @brief An album is selected in the saved searches list.
 * @param a This album will be selected.
 */
void GPSSearchView::slotAlbumSelected(Album* a)
{
    /// @todo This re-sets the region selection unwantedly...

    SAlbum* const salbum = dynamic_cast<SAlbum*>(a);

    if (!salbum)
    {
        return;
    }

    SearchXmlReader reader(salbum->query());
    reader.readToFirstField();
    QStringRef type = reader.attributes().value(QLatin1String("type"));

    if (type == QLatin1String("rectangle"))
    {
        const QList<double> list = reader.valueToDoubleList();

        const GeoCoordinates::Pair coordinates(
            GeoCoordinates(list.at(1), list.at(0)),
            GeoCoordinates(list.at(3), list.at(2))
        );

        /// @todo Currently, invalid coordinates are stored as -200:
        if (list.at(1) != -200)
        {
            d->mapSearchWidget->setRegionSelection(coordinates);
            d->gpsMarkerTiler->setRegionSelection(coordinates);
        }
        else
        {
            d->mapSearchWidget->clearRegionSelection();
            d->gpsMarkerTiler->removeCurrentRegionSelection();
        }

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
        QString label = i18n("Search name already exists.\n"
                             "Please enter a new name:");
        bool ok;
        QString newTitle = QInputDialog::getText(this,
                                                 i18n("Name exists"),
                                                 label,
                                                 QLineEdit::Normal,
                                                 name,
                                                 &ok);

        if (!ok)
        {
            return false;
        }

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
        const SAlbum* const album = (SAlbum*)(*it);

        if (album->title() == name)
        {
            return false;
        }
    }

    return true;
}

/**
 * @brief Remove the current filter.
 */
void GPSSearchView::slotRemoveCurrentFilter()
{
    d->gpsMarkerTiler->setPositiveFilterIsActive(false);
    const QList<qlonglong> emptyIdList;
    emit signalMapSoloItems(emptyIdList, QLatin1String("gpssearch"));
    slotRefreshMap();
    d->mapSearchWidget->slotUpdateActionsEnabled();
}

/**
 * @brief Enable or disable the album saving controls.
 */
void GPSSearchView::slotCheckNameEditGPSConditions()
{
    if (d->mapSearchWidget->getRegionSelection().first.hasCoordinates())
    {
        d->nameEdit->setEnabled(true);

        if (!d->nameEdit->text().isEmpty())
        {
            d->saveBtn->setEnabled(true);
        }
    }
    else
    {
        d->nameEdit->setEnabled(false);
        d->saveBtn->setEnabled(false);
    }
}

/**
 * @brief Slot which gets called when the user makes items 'solo' on the map
 * @param gpsList List of GPSInfos which are 'solo'
 */
void GPSSearchView::slotMapSoloItems(const QList<qlonglong>& idList)
{
    emit(signalMapSoloItems(idList, QLatin1String("gpssearch")));
    d->mapSearchWidget->slotUpdateActionsEnabled();
}

void GPSSearchView::showNonGeolocatedItems()
{
    if(d->nonGeonlocatedItemsXml.isEmpty())
    {
        SearchXmlWriter writer;
        writer.setFieldOperator((SearchXml::standardFieldOperator()));
        writer.writeGroup();
        writer.writeField(QLatin1String("nogps"), SearchXml::Equal);
        writer.finishField();
        writer.finishGroup();
        writer.finish();
        d->nonGeonlocatedItemsXml = writer.xml();
    }

    QString title = SAlbum::getTemporaryTitle(DatabaseSearch::MapSearch);
    SAlbum* album = AlbumManager::instance()->findSAlbum(title);

    int id;

    if (album)
    {
        id = album->id();
        CoreDbAccess().db()->updateSearch(id,DatabaseSearch::AdvancedSearch,
                                            SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch), d->nonGeonlocatedItemsXml);
    }
    else
    {
        id = CoreDbAccess().db()->addSearch(DatabaseSearch::AdvancedSearch,
                                              SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch), d->nonGeonlocatedItemsXml);
    }

    album = new SAlbum(i18n("Non Geo-located Items"), id);

    if (album)
    {
        AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << album);
    }
}


void GPSSearchView::slotRefreshMap()
{
    d->mapSearchWidget->refreshMap();
}

void GPSSearchView::slotClearImages()
{
    if (d->mapSearchWidget->getActiveState())
    {
        d->imageAlbumModel->clearImageInfos();
    }
}

} // namespace Digikam
