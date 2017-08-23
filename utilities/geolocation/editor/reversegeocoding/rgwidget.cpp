/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-12
 * Description : A widget to apply Reverse Geocoding
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
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

#include "rgwidget.h"

// Qt includes

#include <QCheckBox>
#include <QContextMenuEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QMenu>
#include <QUrl>
#include <QInputDialog>
#include <QAction>
#include <QComboBox>
#include <QApplication>
#include <QMessageBox>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// local includes

#include "geoifacetypes.h"
#include "dlayoutbox.h"
#include "gpsundocommand.h"
#include "geolocationedit.h"
#include "gpsimagemodel.h"
#include "gpsimageitem.h"
#include "backend-geonames-rg.h"
#include "backend-osm-rg.h"
#include "backend-geonamesUS-rg.h"
#include "parsetagstring.h"
#include "rgtagmodel.h"
#include "simpletreemodel.h"
#include "dmessagebox.h"
#include "dexpanderbox.h"
#include "iptcproperties.h"

#ifdef GPSSYNC_MODELTEST
#   include <modeltest.h>
#endif // GPSSYNC_MODELTEST

namespace Digikam
{

/**
 * @class RGWidget
 *
 * @brief The RGWidget class represents the main widget for reverse geocoding.
 */

class RGWidget::Private
{
public:

    Private()
        : currentlyAskingCancelQuestion(false),
          hideOptions(true),
          UIEnabled(true),
          label(0),
          imageModel(0),
          selectionModel(0),
          buttonRGSelected(0),
          undoCommand(0),
          serviceComboBox(0),
          languageEdit(0),
          currentBackend(0),
          requestedRGCount(0),
          receivedRGCount(0),
          buttonHideOptions(0),
          iptc(0),
          xmpLoc(0),
          xmpKey(0),
          UGridContainer(0),
          LGridContainer(0),
          serviceLabel(0),
          languageLabel(0),
          separator(0),
          tagModel(0),
          tagTreeView(0),
          tagSelectionModel(0),
          actionAddCountry(0),
          actionAddState(0),
          actionAddStateDistrict(0),
          actionAddCounty(0),
          actionAddCity(0),
          actionAddCityDistrict(0),
          actionAddSuburb(0),
          actionAddTown(0),
          actionAddVillage(0),
          actionAddHamlet(0),
          actionAddStreet(0),
          actionAddHouseNumber(0),
          actionAddPlace(0),
          actionAddLAU2(0),
          actionAddLAU1(0),
          actionAddCustomizedSpacer(0),
          actionRemoveTag(0),
          actionRemoveAllSpacers(0),
          actionAddAllAddressElementsToTag(0)
    {
    }

    bool                 currentlyAskingCancelQuestion;
    bool                 hideOptions;
    bool                 UIEnabled;
    QLabel*              label;
    GPSImageModel*       imageModel;
    QItemSelectionModel* selectionModel;
    QPushButton*         buttonRGSelected;

    GPSUndoCommand*      undoCommand;
    QModelIndex          currentTagTreeIndex;

    QComboBox*           serviceComboBox;
    QComboBox*           languageEdit;
    QList<RGInfo>        photoList;
    QList<RGBackend*>    backendRGList;
    RGBackend*           currentBackend;
    int                  requestedRGCount;
    int                  receivedRGCount;
    QPushButton*         buttonHideOptions;
    QCheckBox*           iptc;
    QCheckBox*           xmpLoc;
    QCheckBox*           xmpKey;
    QWidget*             UGridContainer;
    QWidget*             LGridContainer;
    QLabel*              serviceLabel;
    QLabel*              languageLabel;
    DLineWidget*         separator;

    RGTagModel*          tagModel;
    QTreeView*           tagTreeView;

    QItemSelectionModel* tagSelectionModel;
    QAction*             actionAddCountry;
    QAction*             actionAddState;
    QAction*             actionAddStateDistrict;
    QAction*             actionAddCounty;
    QAction*             actionAddCity;
    QAction*             actionAddCityDistrict;
    QAction*             actionAddSuburb;
    QAction*             actionAddTown;
    QAction*             actionAddVillage;
    QAction*             actionAddHamlet;
    QAction*             actionAddStreet;
    QAction*             actionAddHouseNumber;
    QAction*             actionAddPlace;
    QAction*             actionAddLAU2;
    QAction*             actionAddLAU1;
    QAction*             actionAddCustomizedSpacer;
    QAction*             actionRemoveTag;
    QAction*             actionRemoveAllSpacers;
    QAction*             actionAddAllAddressElementsToTag;
};

/**
 * Constructor
 * @param imageModel image model
 * @param selectionModel image selection model
 * @param parent The parent object
 */
RGWidget::RGWidget(GPSImageModel* const imageModel, QItemSelectionModel* const selectionModel,
                   QAbstractItemModel* externTagModel, QWidget* const parent)
    : QWidget(parent),
      d(new Private())
{
    d->imageModel     = imageModel;
    d->selectionModel = selectionModel;

    // we need to have a main layout and add KVBox to it or derive from KVBox
    // - or is there an easier way to use KVBox?
    QVBoxLayout* const vBoxLayout = new QVBoxLayout(this);
    d->UGridContainer             = new QWidget(this);

    vBoxLayout->addWidget(d->UGridContainer);
    d->tagTreeView = new QTreeView(this);
    d->tagTreeView->setHeaderHidden(true);
    vBoxLayout->addWidget(d->tagTreeView);

    Q_ASSERT(d->tagTreeView!=0);

    if (!externTagModel)
        externTagModel = new SimpleTreeModel(1, this);

    d->tagModel = new RGTagModel(externTagModel, this);
    d->tagTreeView->setModel(d->tagModel);

#ifdef GPSSYNC_MODELTEST
    new ModelTest(externTagModel, d->tagTreeView);
    new ModelTest(d->tagModel, d->tagTreeView);
#endif // GPSSYNC_MODELTEST

    d->tagSelectionModel         = new QItemSelectionModel(d->tagModel);
    d->tagTreeView->setSelectionModel(d->tagSelectionModel);

    d->actionAddCountry          = new QAction(i18n("Add country tag"), this);
    d->actionAddCountry->setData(QString::fromLatin1("{Country}"));
    d->actionAddState            = new QAction(i18n("Add state tag"), this);
    d->actionAddState->setData(QString::fromLatin1("{State}"));
    d->actionAddStateDistrict    = new QAction(i18n("Add state district tag"), this);
    d->actionAddStateDistrict->setData(QString::fromLatin1("{State district}"));
    d->actionAddCounty           = new QAction(i18n("Add county tag"), this);
    d->actionAddCounty->setData(QString::fromLatin1("{County}"));
    d->actionAddCity             = new QAction(i18n("Add city tag"), this);
    d->actionAddCity->setData(QString::fromLatin1("{City}"));
    d->actionAddCityDistrict     = new QAction(i18n("Add city district tag"), this);
    d->actionAddCityDistrict->setData(QString::fromLatin1("{City district}"));
    d->actionAddSuburb           = new QAction(i18n("Add suburb tag"), this);
    d->actionAddSuburb->setData(QString::fromLatin1("{Suburb}"));
    d->actionAddTown             = new QAction(i18n("Add town tag"), this);
    d->actionAddTown->setData(QString::fromLatin1("{Town}"));
    d->actionAddVillage          = new QAction(i18n("Add village tag"), this);
    d->actionAddVillage->setData(QString::fromLatin1("{Village}"));
    d->actionAddHamlet           = new QAction(i18n("Add hamlet tag"), this);
    d->actionAddHamlet->setData(QString::fromLatin1("{Hamlet}"));
    d->actionAddStreet           = new QAction(i18n("Add street"), this);
    d->actionAddStreet->setData(QString::fromLatin1("{Street}"));
    d->actionAddHouseNumber      = new QAction(i18n("Add house number tag"), this);
    d->actionAddHouseNumber->setData(QString::fromLatin1("{House number}"));
    d->actionAddPlace            = new QAction(i18n("Add place"), this);
    d->actionAddPlace->setData(QString::fromLatin1("{Place}"));
    d->actionAddLAU2             = new QAction(i18n("Add Local Administrative Area 2"), this);
    d->actionAddLAU2->setData(QString::fromLatin1("{LAU2}"));
    d->actionAddLAU1             = new QAction(i18n("Add Local Administrative Area 1"), this);
    d->actionAddLAU1->setData(QString::fromLatin1("{LAU1}"));
    d->actionAddCustomizedSpacer = new QAction(i18n("Add new tag"), this);
    d->actionRemoveTag           = new QAction(i18n("Remove selected tag"), this);
    d->actionRemoveAllSpacers    = new QAction(i18n("Remove all control tags below this tag"), this);
    d->actionRemoveAllSpacers->setData(QString::fromLatin1("Remove all spacers"));

    d->actionAddAllAddressElementsToTag = new QAction(i18n("Add all address elements"), this);
    QGridLayout* const gridLayout       = new QGridLayout(d->UGridContainer);

    d->languageLabel = new QLabel(i18n("Select language:"), d->UGridContainer);
    d->languageEdit  = new QComboBox(d->UGridContainer);

    IPTCProperties::CountryCodeMap map = IPTCProperties::countryCodeMap();

    for (IPTCProperties::CountryCodeMap::Iterator it = map.begin(); it != map.end(); ++it)
    {

        d->languageEdit->addItem(QString::fromUtf8("%1 - %2").arg(it.key()).arg(it.value()), it.key().toLower());
    }

    d->serviceLabel    = new QLabel(i18n("Select service:"), d->UGridContainer);
    d->serviceComboBox = new QComboBox(d->UGridContainer);

    d->serviceComboBox->addItem(i18n("Open Street Map"));
    d->serviceComboBox->addItem(i18n("Geonames.org place name (non-US)"));
    d->serviceComboBox->addItem(i18n("Geonames.org full address (US only)"));

    int row = 0;
    gridLayout->addWidget(d->serviceLabel,row,0,1,2);
    row++;
    gridLayout->addWidget(d->serviceComboBox,row,0,1,2);
    row++;
    gridLayout->addWidget(d->languageLabel,row,0,1,1);
    gridLayout->addWidget(d->languageEdit,row,1,1,1);

    d->UGridContainer->setLayout(gridLayout);

    d->separator         = new DLineWidget(Qt::Horizontal, this);
    vBoxLayout->addWidget(d->separator);

    d->buttonHideOptions = new QPushButton(i18n("Less options"), this);
    vBoxLayout->addWidget(d->buttonHideOptions);

    d->LGridContainer              = new QWidget(this);
    vBoxLayout->addWidget(d->LGridContainer);
    QGridLayout* const LGridLayout = new QGridLayout(d->LGridContainer);

    d->xmpLoc = new QCheckBox( i18n("Write tags to XMP"), d->LGridContainer);
    row       = 0;
    LGridLayout->addWidget(d->xmpLoc,row,0,1,3);

    d->LGridContainer->setLayout(LGridLayout);

    d->buttonRGSelected = new QPushButton(i18n("Apply reverse geocoding"), this);
    vBoxLayout->addWidget(d->buttonRGSelected);

    d->backendRGList.append(new BackendOsmRG(this));
    d->backendRGList.append(new BackendGeonamesRG(this));
    d->backendRGList.append(new BackendGeonamesUSRG(this));

    d->tagTreeView->installEventFilter(this);

    updateUIState();

    connect(d->buttonRGSelected, SIGNAL(clicked()),
            this, SLOT(slotButtonRGSelected()));

    connect(d->buttonHideOptions, SIGNAL(clicked()),
            this, SLOT(slotHideOptions()));

    connect(d->selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(updateUIState()));

    connect(d->actionAddCountry, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddState, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddStateDistrict, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddCounty, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddCity, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddCityDistrict, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddSuburb, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddTown, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddVillage, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddHamlet, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddHouseNumber, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddStreet, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddPlace, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddLAU2, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddLAU1, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddCustomizedSpacer, SIGNAL(triggered(bool)),
            this, SLOT(slotAddCustomizedSpacer()));

    connect(d->actionAddAllAddressElementsToTag, SIGNAL(triggered(bool)),
            this, SLOT(slotAddAllAddressElementsToTag()));

    connect(d->imageModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotRegenerateNewTags()));

    connect(d->actionRemoveTag, SIGNAL(triggered(bool)),
            this, SLOT(slotRemoveTag()));

    connect(d->actionRemoveAllSpacers, SIGNAL(triggered(bool)),
            this, SLOT(slotRemoveAllSpacers()));

    for (int i = 0; i < d->backendRGList.count(); ++i)
    {
        connect(d->backendRGList[i], SIGNAL(signalRGReady(QList<RGInfo>&)),
                this, SLOT(slotRGReady(QList<RGInfo>&)));
    }

    int currentServiceIndex = d->serviceComboBox->currentIndex();
    d->currentBackend       = d->backendRGList[currentServiceIndex];
}

/**
 * Destructor
 */
RGWidget::~RGWidget()
{
    delete d;
}

/**
 * Enables or disables the containing widgets.
 */
void RGWidget::updateUIState()
{
    const bool haveSelection = d->selectionModel->hasSelection();

    d->buttonRGSelected->setEnabled(d->UIEnabled && haveSelection);
    d->serviceLabel->setEnabled(d->UIEnabled);
    d->serviceComboBox->setEnabled(d->UIEnabled);
    d->languageLabel->setEnabled(d->UIEnabled);
    d->languageEdit->setEnabled(d->UIEnabled);
    d->buttonHideOptions->setEnabled(d->UIEnabled);
    d->xmpLoc->setEnabled(d->UIEnabled);
}

/**
 * This slot triggeres when the button that start the reverse geocoding process is pressed.
 */
void RGWidget::slotButtonRGSelected()
{
    // get the selected images:
    const QModelIndexList selectedItems = d->selectionModel->selectedRows();
    int currentServiceIndex             = d->serviceComboBox->currentIndex();
    d->currentBackend                   = d->backendRGList[currentServiceIndex];
    d->undoCommand                      = new GPSUndoCommand();
    d->undoCommand->setText(i18n("Image tags are changed."));

    QList<RGInfo> photoList;
    QString wantedLanguage                 = d->languageEdit->itemData(d->languageEdit->currentIndex()).toString();
    QList<QList<TagData> > returnedSpacers = d->tagModel->getSpacers();

    for ( int i = 0; i < selectedItems.count(); ++i)
    {
        const QPersistentModelIndex itemIndex = selectedItems.at(i);
        GPSImageItem* const selectedItem      = d->imageModel->itemFromIndex(itemIndex);
        const GPSDataContainer gpsData        = selectedItem->gpsData();

         if (!gpsData.hasCoordinates())
            continue;

        const qreal latitude  = gpsData.getCoordinates().lat();
        const qreal longitude = gpsData.getCoordinates().lon();

        RGInfo photoObj;
        photoObj.id           = itemIndex;
        photoObj.coordinates  = GeoCoordinates(latitude, longitude);

        photoList << photoObj;

        selectedItem->writeTagsToXmp(d->xmpLoc->isChecked());
    }

    if (!photoList.isEmpty())
    {
        d->receivedRGCount  = 0;
        d->requestedRGCount = photoList.count();

        emit(signalSetUIEnabled(false, this, QString::fromUtf8(SLOT(slotRGCanceled()))));
        emit(signalProgressSetup(d->requestedRGCount, i18n("Retrieving RG info -")));

        d->currentBackend->callRGBackend(photoList, wantedLanguage);
    }
}

/**
 * Hide or shows the extra options.
 */
void RGWidget::slotHideOptions()
{
    if (d->hideOptions)
    {
        d->LGridContainer->hide();
        d->hideOptions = false;
        d->buttonHideOptions->setText(i18n("More options"));
    }
    else
    {
        d->LGridContainer->show();
        d->hideOptions = true;
        d->buttonHideOptions->setText(i18n("Less options"));
    }
}

/**
 * The data has returned from backend and now it's processed here.
 * @param returnedRGList Contains the data returned by backend.
 */
void RGWidget::slotRGReady(QList<RGInfo>& returnedRGList)
{
    const QString errorString = d->currentBackend->getErrorMessage();

    if (!errorString.isEmpty())
    {
        /// @todo This collides with the message box displayed if the user aborts the RG process
        QMessageBox::critical(this, qApp->applicationName(), errorString);

        d->receivedRGCount+=returnedRGList.count();
        emit(signalSetUIEnabled(true));
        return;
    }

    QString address;

    for (int i = 0; i < returnedRGList.count(); ++i)
    {
        QPersistentModelIndex currentImageIndex = returnedRGList[i].id;

        if (!returnedRGList[i].rgData.empty())
        {
            QString addressElementsWantedFormat;

            if (d->currentBackend->backendName() == QString::fromLatin1("Geonames"))
            {
                addressElementsWantedFormat.append(QString::fromLatin1("/{Country}/{Place}"));
            }
            else if (d->currentBackend->backendName() == QString::fromLatin1("GeonamesUS"))
            {
                addressElementsWantedFormat.append(QString::fromLatin1("/{LAU2}/{LAU1}/{City}"));
            }
            else
            {
                addressElementsWantedFormat.append(QString::fromLatin1("/{Country}/{State}/{State district}/{County}/{City}/{City district}/{Suburb}/{Town}/{Village}/{Hamlet}/{Street}/{House number}"));
            }

            QStringList combinedResult = makeTagString(returnedRGList[i], addressElementsWantedFormat, d->currentBackend->backendName());
            QString addressFormat      = combinedResult[0];
            QString addressElements    = combinedResult[1];

            //removes first "/" from tag addresses
            addressFormat.remove(0,1);
            addressElements.remove(0,1);
            addressElementsWantedFormat.remove(0,1);

            const QStringList listAddressElementsWantedFormat = addressElementsWantedFormat.split(QLatin1Char('/'));
            const QStringList listAddressElements             = addressElements.split(QLatin1Char('/'));
            const QStringList listAddressFormat               = addressFormat.split(QLatin1Char('/'));
            QStringList elements, resultedData;

            for (int i = 0; i < listAddressElementsWantedFormat.count(); ++i)
            {
                QString currentAddressFormat = listAddressElementsWantedFormat.at(i);
                int currentIndexFormat       = listAddressFormat.indexOf(currentAddressFormat,0);

                if (currentIndexFormat != -1)
                {
                    elements<<currentAddressFormat;
                    resultedData<<listAddressElements.at(currentIndexFormat);
                }
            }

            QList<QList<TagData> > returnedTags = d->tagModel->addNewData(elements, resultedData);
            GPSImageItem* const currentItem     = d->imageModel->itemFromIndex(currentImageIndex);

            GPSUndoCommand::UndoInfo undoInfo(currentImageIndex);
            undoInfo.readOldDataFromItem(currentItem);

            currentItem->setTagList(returnedTags);

            undoInfo.readNewDataFromItem(currentItem);
            d->undoCommand->addUndoInfo(undoInfo);
        }
    }

    d->receivedRGCount += returnedRGList.count();

    if (d->receivedRGCount >= d->requestedRGCount)
    {
        if (d->currentlyAskingCancelQuestion)
        {
            // if the user is currently answering the cancel question, do nothing, only report progress
            emit(signalProgressChanged(d->receivedRGCount));
        }
        else
        {
            emit(signalUndoCommand(d->undoCommand));
            d->undoCommand = 0;

            emit(signalSetUIEnabled(true));
        }
    }
    else
    {
        emit(signalProgressChanged(d->receivedRGCount));
    }
}

/**
 * Sets whether the containing widgets are enabled or disabled.
 * @param state If true, the controls are enabled.
 */
void RGWidget::setUIEnabled(const bool state)
{
    d->UIEnabled = state;
    updateUIState();
}

/**
 * Here are filtered the events.
 */
bool RGWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == d->tagTreeView)
    {
        if ((event->type() == QEvent::ContextMenu) && d->UIEnabled)
        {
            QMenu* const menu             = new QMenu(d->tagTreeView);
            const int currentServiceIndex = d->serviceComboBox->currentIndex();
            d->currentBackend             = d->backendRGList[currentServiceIndex];
            QString backendName           = d->currentBackend->backendName();
            QContextMenuEvent* const e    = static_cast<QContextMenuEvent*>(event);
            d->currentTagTreeIndex        = d->tagTreeView->indexAt(e->pos());
            const Type tagType            = d->tagModel->getTagType(d->currentTagTreeIndex);

            if ( backendName == QString::fromLatin1("OSM"))
            {
                menu->addAction(d->actionAddAllAddressElementsToTag);
                menu->addSeparator();
                menu->addAction(d->actionAddCountry);
                menu->addAction(d->actionAddState);
                menu->addAction(d->actionAddStateDistrict);
                menu->addAction(d->actionAddCounty);
                menu->addAction(d->actionAddCity);
                menu->addAction(d->actionAddCityDistrict);
                menu->addAction(d->actionAddSuburb);
                menu->addAction(d->actionAddTown);
                menu->addAction(d->actionAddVillage);
                menu->addAction(d->actionAddHamlet);
                menu->addAction(d->actionAddStreet);
                menu->addAction(d->actionAddHouseNumber);
            }
            else if ( backendName == QString::fromLatin1("Geonames"))
            {
                menu->addAction(d->actionAddAllAddressElementsToTag);
                menu->addAction(d->actionAddCountry);
                menu->addAction(d->actionAddPlace);
            }
            else if ( backendName == QString::fromLatin1("GeonamesUS"))
            {
                menu->addAction(d->actionAddAllAddressElementsToTag);
                menu->addAction(d->actionAddLAU2);
                menu->addAction(d->actionAddLAU1);
                menu->addAction(d->actionAddCity);
            }

            menu->addSeparator();
            menu->addAction(d->actionAddCustomizedSpacer);
            menu->addSeparator();

            if (tagType == TypeSpacer)
            {
                menu->addAction(d->actionRemoveTag);
            }

            menu->addAction(d->actionRemoveAllSpacers);
            menu->exec(e->globalPos());
            delete menu;
        }
    }

    return QObject::eventFilter(watched, event);
}

/**
 * Saves the settings of widgets contained in reverse geocoding widget.
 * @param group Here are stored the settings.
 */
void RGWidget::saveSettingsToGroup(KConfigGroup* const group)
{
    group->writeEntry("RG Backend",   d->serviceComboBox->currentIndex());
    group->writeEntry("Language",     d->languageEdit->currentIndex());
    group->writeEntry("Hide options", d->hideOptions);
    group->writeEntry("XMP location", d->xmpLoc->isChecked());

    QList<QList<TagData> > currentSpacerList = d->tagModel->getSpacers();
    const int spacerCount                    = currentSpacerList.count();
    group->writeEntry("Spacers count", spacerCount);

    for (int i = 0; i < currentSpacerList.count(); ++i)
    {
        QString spacerName;
        spacerName.append(QString::fromLatin1("Spacerlistname %1").arg(i));
        QString spacerType;
        spacerType.append(QString::fromLatin1("Spacerlisttype %1").arg(i));

        QStringList spacerTagNames;
        QStringList spacerTypes;

        for (int j = 0; j < currentSpacerList[i].count(); ++j)
        {
            spacerTagNames.append(currentSpacerList[i].at(j).tagName);

            if (currentSpacerList[i].at(j).tagType == TypeSpacer)
            {
                spacerTypes.append(QString::fromLatin1("Spacer"));
            }
            else if (currentSpacerList[i].at(j).tagType == TypeNewChild)
            {
                spacerTypes.append(QString::fromLatin1("NewChild"));
            }
            else
            {
                spacerTypes.append(QString::fromLatin1("OldChild"));
            }
        }

        group->writeEntry(spacerName, spacerTagNames);
        group->writeEntry(spacerType, spacerTypes);
    }
}

/**
 * Restores the settings of widgets contained in reverse geocoding widget.
 * @param group Here are stored the settings.
 */
void RGWidget::readSettingsFromGroup(const KConfigGroup* const group)
{
    const int spacerCount = group->readEntry("Spacers count", 0);
    QList<QList<TagData> > spacersList;

    for (int i = 0; i < spacerCount; ++i)
    {
        QStringList spacerTagNames = group->readEntry(QString::fromLatin1("Spacerlistname %1").arg(i), QStringList());
        QStringList spacerTypes    = group->readEntry(QString::fromLatin1("Spacerlisttype %1").arg(i), QStringList());
        QList<TagData> currentSpacerAddress;

        for (int j = 0; j < spacerTagNames.count(); ++j)
        {
            TagData currentTagData;
            currentTagData.tagName = spacerTagNames.at(j);
            QString currentTagType = spacerTypes.at(j);

            if (currentTagType == QString::fromLatin1("Spacer"))
                currentTagData.tagType = TypeSpacer;
            else if (currentTagType == QString::fromLatin1("NewChild"))
                currentTagData.tagType = TypeNewChild;
            else if (currentTagType == QString::fromLatin1("OldChild"))
                currentTagData.tagType = TypeChild;

            currentSpacerAddress.append(currentTagData);
        }

        spacersList.append(currentSpacerAddress);
    }

    //this make sure that all external tags are added to tag tree view before spacers are re-added
    d->tagModel->addAllExternalTagsToTreeView();
    d->tagModel->readdNewTags(spacersList);

    d->serviceComboBox->setCurrentIndex(group->readEntry("RG Backend", 0));
    d->languageEdit->setCurrentIndex(group->readEntry("Language", 0));

    d->hideOptions = !(group->readEntry("Hide options", false));
    slotHideOptions();

    d->xmpLoc->setChecked(group->readEntry("XMP location", false));
}

/**
 * Adds a tag to tag tree.
 */
void RGWidget::slotAddSingleSpacer()
{
    //    const QModelIndex baseIndex = d->tagSelectionModel->currentIndex();
    QModelIndex baseIndex;

    if (!d->currentTagTreeIndex.isValid())
        baseIndex = d->currentTagTreeIndex;
    else
        baseIndex = d->tagSelectionModel->currentIndex();

    QAction* const senderAction = qobject_cast<QAction*>(sender());
    QString currentSpacerName   = senderAction->data().toString();

    d->tagModel->addSpacerTag(baseIndex, currentSpacerName);
}

/**
 * Adds a new tag to the tag tree.
 */
void RGWidget::slotAddCustomizedSpacer()
{
    QModelIndex baseIndex;

    if (!d->currentTagTreeIndex.isValid())
    {
        baseIndex = d->currentTagTreeIndex;
    }
    else
    {
        baseIndex = d->tagSelectionModel->currentIndex();
    }

    bool ok            = false;
    QString textString = QInputDialog::getText(this, i18nc("@title:window", "Add new tag:"),
                                               i18n("Select a name for the new tag:"),
                                               QLineEdit::Normal, QString(), &ok);

    if ( ok && !textString.isEmpty() )
    {
        d->tagModel->addSpacerTag(baseIndex, textString);
    }
}

/**
 * Removes a tag from tag tree.
 * Note: If the tag is an external, it is no more deleted.
 */
void RGWidget::slotRemoveTag()
{
    const QModelIndex baseIndex = d->tagSelectionModel->currentIndex();
    d->tagModel->deleteTag(baseIndex);
}

/**
 * Removes all spacers.
 */
void RGWidget::slotRemoveAllSpacers()
{
    QString whatShouldRemove = QString::fromLatin1("Spacers");
    QModelIndex baseIndex;

    if (!d->currentTagTreeIndex.isValid())
    {
        baseIndex = d->currentTagTreeIndex;
    }
    else
    {
        baseIndex = d->tagSelectionModel->currentIndex();
    }

    d->tagModel->deleteAllSpacersOrNewTags(baseIndex, TypeSpacer);
}

/**
 * Re-adds all deleted tags based on Undo/Redo widget.
 */
void RGWidget::slotReaddNewTags()
{
    for (int row = 0; row < d->imageModel->rowCount(); ++row)
    {
        GPSImageItem* const currentItem     = d->imageModel->itemFromIndex(d->imageModel->index(row, 0));
        QList<QList<TagData> > tagAddresses = currentItem->getTagList();

        if (!tagAddresses.isEmpty())
        {
            d->tagModel->readdNewTags(tagAddresses);
        }
    }
}

/**
 * Deletes and re-adds all new added tags.
 */
void RGWidget::slotRegenerateNewTags()
{
    QModelIndex baseIndex = QModelIndex();
    d->tagModel->deleteAllSpacersOrNewTags(baseIndex, TypeNewChild);

    slotReaddNewTags();
}

/**
 * Adds all address elements below the selected tag. The address ellements are order by area size.
 * For example: country > state > state district > city ...
 */
void RGWidget::slotAddAllAddressElementsToTag()
{
    QModelIndex baseIndex;

    if (!d->currentTagTreeIndex.isValid())
    {
        baseIndex = d->currentTagTreeIndex;
    }
    else
    {
        baseIndex = d->tagSelectionModel->currentIndex();
    }

    QStringList spacerList;

    if (d->currentBackend->backendName() == QString::fromLatin1("OSM"))
    {
        /// @todo Why are these wrapped in QString?
        spacerList.append(QString::fromLatin1("{Country}"));
        spacerList.append(QString::fromLatin1("{State}"));
        spacerList.append(QString::fromLatin1("{State district}"));
        spacerList.append(QString::fromLatin1("{County}"));
        spacerList.append(QString::fromLatin1("{City}"));
        spacerList.append(QString::fromLatin1("{City district}"));
        spacerList.append(QString::fromLatin1("{Suburb}"));
        spacerList.append(QString::fromLatin1("{Town}"));
        spacerList.append(QString::fromLatin1("{Village}"));
        spacerList.append(QString::fromLatin1("{Hamlet}"));
        spacerList.append(QString::fromLatin1("{Street}"));
        spacerList.append(QString::fromLatin1("{House number}"));
    }
    else if (d->currentBackend->backendName() == QString::fromLatin1("Geonames"))
    {
        spacerList.append(QString::fromLatin1("{Country}"));
        spacerList.append(QString::fromLatin1("{Place}"));
    }
    else
    {
        spacerList.append(QString::fromLatin1("{LAU1}"));
        spacerList.append(QString::fromLatin1("{LAU2}"));
        spacerList.append(QString::fromLatin1("{City}"));
    }

    d->tagModel->addAllSpacersToTag(baseIndex, spacerList,0);
}

void RGWidget::slotRGCanceled()
{
    if (!d->undoCommand)
    {
        // the undo command object is not available, therefore
        // RG has probably been finished already
        return;
    }

    if (d->receivedRGCount>0)
    {
        // Before we abort, ask the user whether he wants to discard
        // the information obtained so far.

        // ATTENTION: While we ask the question, the RG backend continues running
        //            and sends information about new images to this widget.
        //            This means that RG might finish while we ask the question!!!
        d->currentlyAskingCancelQuestion = true;

        const QString question = i18n("%1 out of %2 images have been reverse geocoded. Would you like to keep the tags which were already obtained or discard them?",
                                      d->receivedRGCount, d->requestedRGCount);

        const int result = DMessageBox::showYesNo(QMessageBox::Warning,
                                                  this,
                                                  i18n("Abort reverse geocoding?"),
                                                  question
                                                 );

        d->currentlyAskingCancelQuestion = false;

        if (result == QMessageBox::Cancel)
        {
            // continue

            // did RG finish while we asked the question?
            if (d->receivedRGCount==d->requestedRGCount)
            {
                // the undo data was delayed, now send it
                if (d->undoCommand)
                {
                    emit(signalUndoCommand(d->undoCommand));
                    d->undoCommand = 0;
                }

                // unlock the UI
                emit(signalSetUIEnabled(true));
            }

            return;
        }

        if (result == QMessageBox::No)
        {
            // discard the tags
            d->undoCommand->undo();
        }

        if (result == QMessageBox::Yes)
        {
            if (d->undoCommand)
            {
                emit(signalUndoCommand(d->undoCommand));
                d->undoCommand = 0;
            }
        }
    }

    // clean up the RG request:
    d->currentBackend->cancelRequests();

    if (d->undoCommand)
    {
        delete d->undoCommand;
        d->undoCommand = 0;
    }

    emit(signalSetUIEnabled(true));
}

} // namespace Digikam
