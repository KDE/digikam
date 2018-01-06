/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : camera setup tab.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupcamera.h"

// Qt includes

#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QPixmap>
#include <QCheckBox>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>
#include <QListWidget>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QStandardPaths>
#include <QIcon>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "albumselectwidget.h"
#include "cameralist.h"
#include "cameraselection.h"
#include "cameratype.h"
#include "digikam_config.h"
#include "gpcamera.h"
#include "filtercombo.h"
#include "importfilters.h"
#include "dfontselect.h"
#include "importsettings.h"
#include "fullscreensettings.h"
#include "dxmlguiwindow.h"
#include "dactivelabel.h"

namespace Digikam
{

class SetupCameraItem : public QTreeWidgetItem
{

public:

    SetupCameraItem(QTreeWidget* const parent, CameraType* const ctype)
        : QTreeWidgetItem(parent), m_ctype(0)
    {
        setCameraType(ctype);
    };

    ~SetupCameraItem()
    {
        delete m_ctype;
    };

    void setCameraType(CameraType* const ctype)
    {
        delete m_ctype;

        m_ctype = new CameraType(*ctype);

        if (m_ctype)
        {
            setText(0, m_ctype->title());
            setText(1, m_ctype->model());
            setText(2, m_ctype->port());
            setText(3, m_ctype->path());
        }
    };

    CameraType* cameraType() const
    {
        return m_ctype;
    };

private:

    CameraType* m_ctype;
};

// -------------------------------------------------------------------

class CameraAutoDetectThread::Private
{
public:

    Private()
    {
        result = 0;
    }

    int     result;

    QString model;
    QString port;
};

CameraAutoDetectThread::CameraAutoDetectThread(QObject* const parent)
    : DBusyThread(parent), d(new Private)
{
    d->result = -1;
}

CameraAutoDetectThread::~CameraAutoDetectThread()
{
    delete d;
}

void CameraAutoDetectThread::run()
{
    d->result = GPCamera::autoDetect(d->model, d->port);
    emit signalComplete();
}

int CameraAutoDetectThread::result() const
{
    return(d->result);
}

QString CameraAutoDetectThread::model() const
{
    return(d->model);
}

QString CameraAutoDetectThread::port() const
{
    return(d->port);
}

// -------------------------------------------------------------------

class SetupCamera::Private
{
public:

    Private() :
        addButton(0),
        removeButton(0),
        editButton(0),
        autoDetectButton(0),
        importAddButton(0),
        importRemoveButton(0),
        importEditButton(0),
        storeDiffButton(0),
        overwriteButton(0),
        skipFileButton(0),
        conflictButtonGroup(0),
        useFileMetadata(0),
        turnHighQualityThumbs(0),
        useDefaultTargetAlbum(0),
        iconShowNameBox(0),
        iconShowSizeBox(0),
        iconShowDateBox(0),
        iconShowResolutionBox(0),
        iconShowTagsBox(0),
        iconShowOverlaysBox(0),
        iconShowRatingBox(0),
        iconShowFormatBox(0),
        iconShowCoordinatesBox(0),
        previewLoadFullImageSize(0),
        previewItemsWhileDownload(0),
        previewShowIcons(0),
        leftClickActionComboBox(0),
        iconViewFontSelect(0),
        target1AlbumSelector(0),
        listView(0),
        importListView(0),
        tab(0),
        ignoreNamesEdit(0),
        ignoreExtensionsEdit(0),
        fullScreenSettings(0)
    {
    }

    static const QString configGroupName;
    static const QString configUseFileMetadata;
    static const QString configTrunHighQualityThumbs;
    static const QString configUseDefaultTargetAlbum;
    static const QString configDefaultTargetAlbumId;
    static const QString configFileSaveConflictRule;
    static const QString importFiltersConfigGroupName;

    QPushButton*         addButton;
    QPushButton*         removeButton;
    QPushButton*         editButton;
    QPushButton*         autoDetectButton;
    QPushButton*         importAddButton;
    QPushButton*         importRemoveButton;
    QPushButton*         importEditButton;

    QRadioButton*        storeDiffButton;
    QRadioButton*        overwriteButton;
    QRadioButton*        skipFileButton;

    QButtonGroup*        conflictButtonGroup;

    QCheckBox*           useFileMetadata;
    QCheckBox*           turnHighQualityThumbs;
    QCheckBox*           useDefaultTargetAlbum;

    QCheckBox*           iconShowNameBox;
    QCheckBox*           iconShowSizeBox;
    QCheckBox*           iconShowDateBox;
    QCheckBox*           iconShowResolutionBox;
    QCheckBox*           iconShowTagsBox;
    QCheckBox*           iconShowOverlaysBox;
    QCheckBox*           iconShowRatingBox;
    QCheckBox*           iconShowFormatBox;
    QCheckBox*           iconShowCoordinatesBox;
    QCheckBox*           previewLoadFullImageSize;
    QCheckBox*           previewItemsWhileDownload;
    QCheckBox*           previewShowIcons;

    QComboBox*           leftClickActionComboBox;

    DFontSelect*         iconViewFontSelect;


    AlbumSelectWidget*   target1AlbumSelector;

    QTreeWidget*         listView;
    QListWidget*         importListView;

    QTabWidget*          tab;

    QLineEdit*           ignoreNamesEdit;
    QLineEdit*           ignoreExtensionsEdit;

    FilterList           filters;

    FullScreenSettings*  fullScreenSettings;
};

const QString SetupCamera::Private::configGroupName(QLatin1String("Camera Settings"));
const QString SetupCamera::Private::configUseFileMetadata(QLatin1String("UseFileMetadata"));
const QString SetupCamera::Private::configTrunHighQualityThumbs(QLatin1String("TurnHighQualityThumbs"));
const QString SetupCamera::Private::configUseDefaultTargetAlbum(QLatin1String("UseDefaultTargetAlbum"));
const QString SetupCamera::Private::configDefaultTargetAlbumId(QLatin1String("DefaultTargetAlbumId"));
const QString SetupCamera::Private::configFileSaveConflictRule(QLatin1String("FileSaveConflictRule"));
const QString SetupCamera::Private::importFiltersConfigGroupName(QLatin1String("Import Filters"));

SetupCamera::SetupCamera(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    d->tab               = new QTabWidget(viewport());
    setWidget(d->tab);
    setWidgetResizable(true);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QWidget* const panel = new QWidget(d->tab);

    QGridLayout* const grid = new QGridLayout(panel);
    d->listView             = new QTreeWidget(panel);
    d->listView->sortItems(0, Qt::AscendingOrder);
    d->listView->setColumnCount(4);
    d->listView->setRootIsDecorated(false);
    d->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setWhatsThis(i18n("Here you can see the digital camera list used by digiKam "
                                   "via the Gphoto interface."));

    QStringList labels;
    labels.append(i18n("Title"));
    labels.append(i18n("Model"));
    labels.append(i18n("Port"));
    labels.append(i18n("Path"));
    d->listView->setHeaderLabels(labels);
    d->listView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    d->listView->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    d->listView->header()->setSectionResizeMode(2, QHeaderView::Stretch);
    d->listView->header()->setSectionResizeMode(3, QHeaderView::Stretch);

    // -------------------------------------------------------------

    d->addButton        = new QPushButton(panel);
    d->removeButton     = new QPushButton(panel);
    d->editButton       = new QPushButton(panel);
    d->autoDetectButton = new QPushButton(panel);

    d->addButton->setText(i18n("&Add..."));
    d->addButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    d->removeButton->setText(i18n("&Remove"));
    d->removeButton->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
    d->editButton->setText(i18n("&Edit..."));
    d->editButton->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    d->autoDetectButton->setText(i18n("Auto-&Detect"));
    d->autoDetectButton->setIcon(QIcon::fromTheme(QLatin1String("edit-find")));
    d->removeButton->setEnabled(false);
    d->editButton->setEnabled(false);

    // -------------------------------------------------------------

    QSpacerItem* const spacer           = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
    DActiveLabel* const gphotoLogoLabel = new DActiveLabel(QUrl(QLatin1String("http://www.gphoto.org")),
                                                           QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/logo-gphoto.png")),
                                                           panel);
    gphotoLogoLabel->setToolTip(i18n("Visit Gphoto project website"));

#ifndef HAVE_GPHOTO2
    // If digiKam is compiled without Gphoto2 support, we hide widgets relevant.
    d->autoDetectButton->hide();
    gphotoLogoLabel->hide();
#endif /* HAVE_GPHOTO2 */

    // -------------------------------------------------------------

    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);
    grid->setAlignment(Qt::AlignTop);
    grid->addWidget(d->listView,         0, 0, 6, 1);
    grid->addWidget(d->addButton,        0, 1, 1, 1);
    grid->addWidget(d->removeButton,     1, 1, 1, 1);
    grid->addWidget(d->editButton,       2, 1, 1, 1);
    grid->addWidget(d->autoDetectButton, 3, 1, 1, 1);
    grid->addItem(spacer,                4, 1, 1, 1);
    grid->addWidget(gphotoLogoLabel,     5, 1, 1, 1);

    d->tab->insertTab(0, panel, i18n("Devices"));

    // -------------------------------------------------------------

    QWidget* const panel2     = new QWidget(d->tab);

    QVBoxLayout* const layout = new QVBoxLayout(panel2);
    d->useFileMetadata        = new QCheckBox(i18n("Use file metadata (makes connection slower)"), panel2);
    d->turnHighQualityThumbs  = new QCheckBox(i18n("Turn on high quality thumbnail loading (slower loading)"), panel2);
    d->useDefaultTargetAlbum  = new QCheckBox(i18n("Use a default target album to download from camera"), panel2);
    d->target1AlbumSelector   = new AlbumSelectWidget(panel2);

    QGroupBox* const conflictBox = new QGroupBox(panel2);
    QLabel* const conflictIcon   = new QLabel(conflictBox);
    conflictIcon->setPixmap(QIcon::fromTheme(QLatin1String("document-save-as")).pixmap(32));
    QLabel* const conflictLabel  = new QLabel(i18n("If target file exists when downloaded from camera"), conflictBox);
    QGridLayout* const boxLayout = new QGridLayout(conflictBox);
    d->conflictButtonGroup       = new QButtonGroup(conflictBox);
    d->storeDiffButton           = new QRadioButton(i18n("Store as a different name"), conflictBox);
    d->overwriteButton           = new QRadioButton(i18n("Overwrite automatically"),   conflictBox);
    d->skipFileButton            = new QRadioButton(i18n("Skip automatically"),        conflictBox);
    d->conflictButtonGroup->addButton(d->overwriteButton, OVERWRITE);
    d->conflictButtonGroup->addButton(d->storeDiffButton, DIFFNAME);
    d->conflictButtonGroup->addButton(d->skipFileButton,  SKIPFILE);
    d->conflictButtonGroup->setExclusive(true);
    d->storeDiffButton->setChecked(true);

    boxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    boxLayout->setSpacing(spacing);
    boxLayout->addWidget(conflictIcon,       0, 0);
    boxLayout->addWidget(conflictLabel,      0, 1);
    boxLayout->addWidget(d->storeDiffButton, 1, 0, 1, 3);
    boxLayout->addWidget(d->overwriteButton, 2, 0, 1, 3);
    boxLayout->addWidget(d->skipFileButton,  3, 0, 1, 3);
    boxLayout->setColumnStretch(2, 1);
    conflictBox->setLayout(boxLayout);

    d->tab->insertTab(1, panel2, i18n("Behavior"));

    layout->setContentsMargins(spacing, spacing, spacing, spacing);
    layout->setSpacing(spacing);
    layout->addWidget(d->useFileMetadata);
    layout->addWidget(d->turnHighQualityThumbs);
    layout->addWidget(d->useDefaultTargetAlbum);
    layout->addWidget(d->target1AlbumSelector);
    layout->addWidget(conflictBox);
    layout->addStretch();

    // -------------------------------------------------------------

    QWidget* const panel3         = new QWidget(d->tab);

    QGridLayout* const importGrid = new QGridLayout(panel3);
    d->importListView             = new QListWidget(panel3);
    d->importListView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->importListView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->importListView->setWhatsThis(i18n("Here you can see filters that can be used to filter "
                                         "files in import dialog."));

    d->importAddButton         = new QPushButton(panel3);
    d->importRemoveButton      = new QPushButton(panel3);
    d->importEditButton        = new QPushButton(panel3);
    QSpacerItem* const spacer2 = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

    QGroupBox* const groupBox         = new QGroupBox(panel3);
    QVBoxLayout* const verticalLayout = new QVBoxLayout(groupBox);
    QLabel* const label               = new QLabel(groupBox);
    verticalLayout->addWidget(label);
    d->ignoreNamesEdit                = new QLineEdit(groupBox);
    verticalLayout->addWidget(d->ignoreNamesEdit);
    QLabel* const label2              = new QLabel(groupBox);
    verticalLayout->addWidget(label2);
    d->ignoreExtensionsEdit           = new QLineEdit(groupBox);
    verticalLayout->addWidget(d->ignoreExtensionsEdit);

    groupBox->setTitle(i18n("Always ignore"));
    label->setText(i18n("Ignored file names:"));
    label2->setText(i18n("Ignored file extensions:"));
    d->importAddButton->setText(i18n("&Add..."));
    d->importAddButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    d->importRemoveButton->setText(i18n("&Remove"));
    d->importRemoveButton->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
    d->importEditButton->setText(i18n("&Edit..."));
    d->importEditButton->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    d->importRemoveButton->setEnabled(false);
    d->importEditButton->setEnabled(false);

    importGrid->setContentsMargins(spacing, spacing, spacing, spacing);
    importGrid->setSpacing(spacing);
    importGrid->setAlignment(Qt::AlignTop);
    importGrid->addWidget(d->importListView,     0, 0, 4, 1);
    importGrid->addWidget(groupBox,              5, 0, 1, 1);
    importGrid->addWidget(d->importAddButton,    0, 1, 1, 1);
    importGrid->addWidget(d->importRemoveButton, 1, 1, 1, 1);
    importGrid->addWidget(d->importEditButton,   2, 1, 1, 1);
    importGrid->addItem(spacer2,                 3, 1, 1, 1);

    d->tab->insertTab(2, panel3, i18n("Import Filters"));

    // -- Import Icon View ----------------------------------------------------------

    QWidget* const panel4      = new QWidget(d->tab);

    QVBoxLayout* const layout2 = new QVBoxLayout(panel4);

    QGroupBox* const iconViewGroup = new QGroupBox(i18n("Icon-View Options"), panel4);
    QGridLayout* const grid2       = new QGridLayout(iconViewGroup);

    d->iconShowNameBox       = new QCheckBox(i18n("Show file&name"), iconViewGroup);
    d->iconShowNameBox->setWhatsThis(i18n("Set this option to show the filename below the image thumbnail."));

    d->iconShowSizeBox       = new QCheckBox(i18n("Show file si&ze"), iconViewGroup);
    d->iconShowSizeBox->setWhatsThis(i18n("Set this option to show the file size below the image thumbnail."));

    d->iconShowDateBox    = new QCheckBox(i18n("Show camera creation &date"), iconViewGroup);
    d->iconShowDateBox->setWhatsThis(i18n("Set this option to show the camera creation date "
                                             "below the image thumbnail."));

/*
    d->iconShowResolutionBox = new QCheckBox(i18n("Show ima&ge dimensions"), iconViewGroup);
    d->iconShowResolutionBox->setWhatsThis(i18n("Set this option to show the image size in pixels "
                                                "below the image thumbnail."));
*/

    d->iconShowFormatBox      = new QCheckBox(i18n("Show image Format"), iconViewGroup);
    d->iconShowFormatBox->setWhatsThis(i18n("Set this option to show image format over image thumbnail."));

    d->iconShowCoordinatesBox = new QCheckBox(i18n("Show Geolocation Indicator"), iconViewGroup);
    d->iconShowCoordinatesBox->setWhatsThis(i18n("Set this option to indicate if image has geolocation information."));

    d->iconShowTagsBox        = new QCheckBox(i18n("Show digiKam &tags"), iconViewGroup);
    d->iconShowTagsBox->setWhatsThis(i18n("Set this option to show the digiKam tags "
                                          "below the image thumbnail."));

    d->iconShowRatingBox      = new QCheckBox(i18n("Show item &rating"), iconViewGroup);
    d->iconShowRatingBox->setWhatsThis(i18n("Set this option to show the item rating "
                                            "below the image thumbnail."));

    d->iconShowOverlaysBox    = new QCheckBox(i18n("Show rotation overlay buttons"), iconViewGroup);
    d->iconShowOverlaysBox->setWhatsThis(i18n("Set this option to show overlay buttons on "
                                              "the image thumbnail for image rotation."));

    QLabel* const leftClickLabel = new QLabel(i18n("Thumbnail click action:"), iconViewGroup);
    d->leftClickActionComboBox   = new QComboBox(iconViewGroup);
    d->leftClickActionComboBox->addItem(i18n("Show embedded preview"), ImportSettings::ShowPreview);
    d->leftClickActionComboBox->addItem(i18n("Start image editor"), ImportSettings::StartEditor);
    d->leftClickActionComboBox->setToolTip(i18n("Choose what should happen when you click on a thumbnail."));

    d->iconViewFontSelect = new DFontSelect(i18n("Icon View font:"), panel);
    d->iconViewFontSelect->setToolTip(i18n("Select here the font used to display text in Icon Views."));

    grid2->addWidget(d->iconShowNameBox,          0, 0, 1, 1);
    grid2->addWidget(d->iconShowSizeBox,          1, 0, 1, 1);
    grid2->addWidget(d->iconShowDateBox,          2, 0, 1, 1);
    grid2->addWidget(d->iconShowFormatBox,        3, 0, 1, 1);
//  grid2->addWidget(d->iconShowResolutionBox,    4, 0, 1, 1);              TODO

    grid2->addWidget(d->iconShowTagsBox,          0, 1, 1, 1);
    grid2->addWidget(d->iconShowRatingBox,        1, 1, 1, 1);
    grid2->addWidget(d->iconShowOverlaysBox,      2, 1, 1, 1);
    grid2->addWidget(d->iconShowCoordinatesBox,   3, 1, 1, 1);
    grid2->addWidget(leftClickLabel,              5, 0, 1, 1);
    grid2->addWidget(d->leftClickActionComboBox,  5, 1, 1, 1);
    grid2->addWidget(d->iconViewFontSelect,       6, 0, 1, 2);
    grid2->setContentsMargins(spacing, spacing, spacing, spacing);
    grid2->setSpacing(spacing);

    // --------------------------------------------------------

    QGroupBox* const interfaceOptionsGroup = new QGroupBox(i18n("Preview Options"), panel4);
    QGridLayout* const grid3               = new QGridLayout(interfaceOptionsGroup);

    d->previewLoadFullImageSize  = new QCheckBox(i18n("Embedded preview loads full-sized images"), interfaceOptionsGroup);
    d->previewLoadFullImageSize->setWhatsThis(i18n("<p>Set this option to load images at their full size "
                                                   "for preview, rather than at a reduced size. As this option "
                                                   "will make it take longer to load images, only use it if you have "
                                                   "a fast computer.</p>"
                                                   "<p><b>Note:</b> for Raw images, a half size version of the Raw data "
                                                   "is used instead of the embedded JPEG preview.</p>"));

    d->previewItemsWhileDownload = new QCheckBox(i18n("Preview each item while downloading it"), interfaceOptionsGroup);
    d->previewItemsWhileDownload->setWhatsThis(i18n("<p>Set this option to preview each item while downloading.</p>"));

    d->previewShowIcons          = new QCheckBox(i18n("Show icons and text over preview"), interfaceOptionsGroup);
    d->previewShowIcons->setWhatsThis(i18n("Uncheck this if you don't want to see icons and text in the image preview."));

    grid3->setContentsMargins(spacing, spacing, spacing, spacing);
    grid3->setSpacing(spacing);
    grid3->addWidget(d->previewLoadFullImageSize,  0, 0, 1, 2);
    grid3->addWidget(d->previewItemsWhileDownload, 1, 0, 1, 2);
    grid3->addWidget(d->previewShowIcons,          2, 0, 1, 2);

    // --------------------------------------------------------

    d->fullScreenSettings = new FullScreenSettings(FS_IMPORTUI, panel4);

    layout2->setContentsMargins(QMargins());
    layout2->setSpacing(spacing);
    layout2->addWidget(iconViewGroup);
    layout2->addWidget(interfaceOptionsGroup);
    layout2->addWidget(d->fullScreenSettings);
    layout2->addStretch();

    d->tab->insertTab(3, panel4, i18n("Import Window"));

    // -------------------------------------------------------------

    adjustSize();

    // -------------------------------------------------------------

    connect(d->listView, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->addButton, SIGNAL(clicked()),
            this, SLOT(slotAddCamera()));

    connect(d->removeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveCamera()));

    connect(d->editButton, SIGNAL(clicked()),
            this, SLOT(slotEditCamera()));

    connect(d->autoDetectButton, SIGNAL(clicked()),
            this, SLOT(slotAutoDetectCamera()));

    connect(d->useDefaultTargetAlbum, SIGNAL(toggled(bool)),
            d->target1AlbumSelector, SLOT(setEnabled(bool)));

    connect(d->previewItemsWhileDownload, SIGNAL(clicked()),
            this, SLOT(slotPreviewItemsClicked()));

    connect(d->previewLoadFullImageSize, SIGNAL(clicked()),
            this, SLOT(slotPreviewFullImageSizeClicked()));

    // -------------------------------------------------------------

    connect(d->importListView, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotImportSelectionChanged()));

    connect(d->importAddButton, SIGNAL(clicked()),
            this, SLOT(slotAddFilter()));

    connect(d->importRemoveButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveFilter()));

    connect(d->importEditButton, SIGNAL(clicked()),
            this, SLOT(slotEditFilter()));

    // -------------------------------------------------------------

    connect(d->useFileMetadata, SIGNAL(toggled(bool)),
            this, SIGNAL(signalUseFileMetadataChanged(bool)));

    // -------------------------------------------------------------

    readSettings();
}

SetupCamera::~SetupCamera()
{
    delete d;
}

bool SetupCamera::useFileMetadata()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    return (group.readEntry(d->configUseFileMetadata, false));
}

void SetupCamera::readSettings()
{
    // Populate cameras --------------------------------------

    CameraList* const clist = CameraList::defaultList();

    if (clist)
    {
        QList<CameraType*>* const cl = clist->cameraList();

        foreach(CameraType* const ctype, *cl)
        {
            new SetupCameraItem(d->listView, ctype);
        }
    }

    // -------------------------------------------------------

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->useFileMetadata->setChecked(useFileMetadata());
    d->turnHighQualityThumbs->setChecked(group.readEntry(d->configTrunHighQualityThumbs, false));
    d->useDefaultTargetAlbum->setChecked(group.readEntry(d->configUseDefaultTargetAlbum, false));
    PAlbum* const album = AlbumManager::instance()->findPAlbum(group.readEntry(d->configDefaultTargetAlbumId, 0));
    d->target1AlbumSelector->setCurrentAlbum(album);
    d->target1AlbumSelector->setEnabled(d->useDefaultTargetAlbum->isChecked());
    d->conflictButtonGroup->button(group.readEntry(d->configFileSaveConflictRule, (int)DIFFNAME))->setChecked(true);

    d->fullScreenSettings->readSettings(group);

    // -------------------------------------------------------

    KConfigGroup importGroup = config->group(d->importFiltersConfigGroupName);

    for (int i = 0; true; ++i)
    {
        QString filter = importGroup.readEntry(QString::fromUtf8("Filter%1").arg(i), QString());

        if (filter.isEmpty())
        {
            break;
        }

        Filter* const f = new Filter;
        f->fromString(filter);
        d->filters.append(f);
    }

    FilterComboBox::defaultFilters(&d->filters);

    foreach(Filter* const f, d->filters)
    {
        new QListWidgetItem(f->name, d->importListView);
    }

    d->importListView->sortItems();

    d->ignoreNamesEdit->setText(importGroup.readEntry(QLatin1String("IgnoreNames"), FilterComboBox::defaultIgnoreNames));
    d->ignoreExtensionsEdit->setText(importGroup.readEntry(QLatin1String("IgnoreExtensions"), FilterComboBox::defaultIgnoreExtensions));

    ImportSettings* const settings = ImportSettings::instance();

    if (!settings)
    {
        return;
    }

    d->iconShowNameBox->setChecked(settings->getIconShowName());
    d->iconShowTagsBox->setChecked(settings->getIconShowTags());
    d->iconShowSizeBox->setChecked(settings->getIconShowSize());
    d->iconShowDateBox->setChecked(settings->getIconShowDate());
    //TODO: d->iconShowResolutionBox->setChecked(settings->getIconShowResolution());
    d->iconShowOverlaysBox->setChecked(settings->getIconShowOverlays());
    d->iconShowRatingBox->setChecked(settings->getIconShowRating());
    d->iconShowFormatBox->setChecked(settings->getIconShowImageFormat());
    d->iconShowCoordinatesBox->setChecked(settings->getIconShowCoordinates());
    d->iconViewFontSelect->setFont(settings->getIconViewFont());

    d->leftClickActionComboBox->setCurrentIndex((int)settings->getItemLeftClickAction());

    d->previewLoadFullImageSize->setChecked(settings->getPreviewLoadFullImageSize());
    d->previewItemsWhileDownload->setChecked(settings->getPreviewItemsWhileDownload());
    d->previewShowIcons->setChecked(settings->getPreviewShowIcons());
}

void SetupCamera::applySettings()
{
    // Save camera devices -----------------------------------

    CameraList* const clist = CameraList::defaultList();

    if (clist)
    {
        clist->clear();

        QTreeWidgetItemIterator it(d->listView);

        while (*it)
        {
            SetupCameraItem* const item = dynamic_cast<SetupCameraItem*>(*it);

            if (item)
            {
                CameraType* const ctype = item->cameraType();

                if (ctype)
                {
                    clist->insert(new CameraType(*ctype));
                }
            }

            ++it;
        }

        clist->save();
    }

    // -------------------------------------------------------

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configUseFileMetadata,  d->useFileMetadata->isChecked());
    group.writeEntry(d->configTrunHighQualityThumbs, d->turnHighQualityThumbs->isChecked());
    group.writeEntry(d->configUseDefaultTargetAlbum, d->useDefaultTargetAlbum->isChecked());
    PAlbum* const album = d->target1AlbumSelector->currentAlbum();
    group.writeEntry(d->configDefaultTargetAlbumId, album ? album->id() : 0);
    group.writeEntry(d->configFileSaveConflictRule, d->conflictButtonGroup->checkedId());

    d->fullScreenSettings->saveSettings(group);

    group.sync();

    // -------------------------------------------------------

    KConfigGroup importGroup = config->group(d->importFiltersConfigGroupName);

    importGroup.deleteGroup();

    for (int i = 0; i < d->filters.count(); ++i)
    {
        importGroup.writeEntry(QString::fromUtf8("Filter%1").arg(i), d->filters[i]->toString());
    }

    importGroup.writeEntry(QLatin1String("IgnoreNames"), d->ignoreNamesEdit->text());
    importGroup.writeEntry(QLatin1String("IgnoreExtensions"), d->ignoreExtensionsEdit->text());
    importGroup.sync();

    ImportSettings* const settings = ImportSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setIconShowName(d->iconShowNameBox->isChecked());
    settings->setIconShowTags(d->iconShowTagsBox->isChecked());
    settings->setIconShowSize(d->iconShowSizeBox->isChecked());
    settings->setIconShowDate(d->iconShowDateBox->isChecked());
    //TODO: settings->setIconShowResolution(d->iconShowResolutionBox->isChecked());
    settings->setIconShowOverlays(d->iconShowOverlaysBox->isChecked());
    settings->setIconShowRating(d->iconShowRatingBox->isChecked());
    settings->setIconShowImageFormat(d->iconShowFormatBox->isChecked());
    settings->setIconShowCoordinates(d->iconShowCoordinatesBox->isChecked());
    settings->setIconViewFont(d->iconViewFontSelect->font());

    settings->setItemLeftClickAction((ImportSettings::ItemLeftClickAction)
                                     d->leftClickActionComboBox->currentIndex());

    settings->setPreviewLoadFullImageSize(d->previewLoadFullImageSize->isChecked());
    settings->setPreviewItemsWhileDownload(d->previewItemsWhileDownload->isChecked());
    settings->setPreviewShowIcons(d->previewShowIcons->isChecked());
    settings->saveSettings();
}

bool SetupCamera::checkSettings()
{
    if (d->useDefaultTargetAlbum->isChecked() && !d->target1AlbumSelector->currentAlbum())
    {
        d->tab->setCurrentIndex(1);
        QMessageBox::information(this, qApp->applicationName(), i18n("No default target album have been selected to process download "
                                                                     "from camera device. Please select one."));
        return false;
    }

    return true;
}

void SetupCamera::slotSelectionChanged()
{
    QTreeWidgetItem* const item = d->listView->currentItem();

    if (!item)
    {
        d->removeButton->setEnabled(false);
        d->editButton->setEnabled(false);
        return;
    }

    d->removeButton->setEnabled(true);
    d->editButton->setEnabled(true);
}

void SetupCamera::slotAddCamera()
{
    CameraSelection* const select = new CameraSelection;

    connect(select, SIGNAL(signalOkClicked(QString, QString, QString, QString)),
            this,   SLOT(slotAddedCamera(QString, QString, QString, QString)));

    select->show();
}

void SetupCamera::slotAddedCamera(const QString& title, const QString& model,
                                  const QString& port,  const QString& path)
{
    CameraType ctype(title, model, port, path, 1);
    new SetupCameraItem(d->listView, &ctype);
}

void SetupCamera::slotRemoveCamera()
{
    SetupCameraItem* const item = dynamic_cast<SetupCameraItem*>(d->listView->currentItem());

    delete item;
}

void SetupCamera::slotEditCamera()
{
    SetupCameraItem* const item = dynamic_cast<SetupCameraItem*>(d->listView->currentItem());

    if (!item)
    {
        return;
    }

    CameraType* const ctype = item->cameraType();

    if (!ctype)
    {
        return;
    }

    CameraSelection* const select = new CameraSelection;
    select->setCamera(ctype->title(), ctype->model(), ctype->port(), ctype->path());

    connect(select, SIGNAL(signalOkClicked(QString,QString,QString,QString)),
            this,   SLOT(slotEditedCamera(QString,QString,QString,QString)));

    select->show();
}

void SetupCamera::slotEditedCamera(const QString& title, const QString& model,
                                   const QString& port, const QString& path)
{
    SetupCameraItem* const item = dynamic_cast<SetupCameraItem*>(d->listView->currentItem());

    if (!item)
    {
        return;
    }

    CameraType ctype(title, model, port, path, 1);
    item->setCameraType(&ctype);
}

void SetupCamera::slotAutoDetectCamera()
{
    DBusyDlg* const dlg                  = new DBusyDlg(i18n("Device detection under progress, please wait..."), this);
    CameraAutoDetectThread* const thread = new CameraAutoDetectThread(this);
    dlg->setBusyThread(thread);
    dlg->exec();

    QString model = thread->model();
    QString port  = thread->port();
    int ret       = thread->result();

    if (ret != 0)
    {
        QMessageBox::critical(this, qApp->applicationName(), i18n("Failed to auto-detect camera.\n"
                                                                  "Please check if your camera is turned on "
                                                                  "and retry or try setting it manually."));
        return;
    }

    // NOTE: See note in digikam/digikam/cameralist.cpp
    if (port.startsWith(QLatin1String("usb:")))
    {
        port = QLatin1String("usb:");
    }

    if (!d->listView->findItems(model, Qt::MatchExactly, 1).isEmpty())
    {
        QMessageBox::information(this, qApp->applicationName(), i18n("Camera '%1' (%2) is already in list.", model, port));
    }
    else
    {
        QMessageBox::information(this, qApp->applicationName(), i18n("Found camera '%1' (%2) and added it to the list.", model, port));
        slotAddedCamera(model, model, port, QLatin1String("/"));
    }
}

void SetupCamera::slotImportSelectionChanged()
{
    QListWidgetItem* const item = d->importListView->currentItem();

    d->importRemoveButton->setEnabled(item);
    d->importEditButton->setEnabled(item);
}

void SetupCamera::slotAddFilter()
{
    Filter filter;
    filter.name = i18n("Untitled");
    ImportFilters dlg(this);
    dlg.setData(filter);

    if (dlg.exec() == QDialog::Accepted)
    {
        Filter* const f = new Filter;
        dlg.getData(f);
        d->filters.append(f);
        new QListWidgetItem(f->name, d->importListView);
    }

    slotImportSelectionChanged();
}

void SetupCamera::slotRemoveFilter()
{
    QListWidgetItem* const item = d->importListView->currentItem();
    int current                 = d->importListView->currentRow();

    for (int i = 0 ; i < d->filters.count() ; i++)
    {
        if (d->filters.at(i)->name == item->text())
        {
            delete d->filters.takeAt(i);
            delete d->importListView->takeItem(current);
            slotImportSelectionChanged();
            break;
        }
    }
}

void SetupCamera::slotEditFilter()
{
    QListWidgetItem* const item = d->importListView->currentItem();

    for (int i = 0 ; i < d->filters.count() ; i++)
    {
        if (d->filters.at(i)->name == item->text())
        {
            Filter filter = *d->filters.at(i);
            ImportFilters dlg(this);
            dlg.setData(filter);

            if (dlg.exec() == QDialog::Accepted)
            {
                Filter* const f = d->filters.at(i);
                dlg.getData(f);
                item->setText(f->name);
            }

            break;
        }
    }
}

void SetupCamera::slotPreviewItemsClicked()
{
    if (d->previewItemsWhileDownload->isChecked() && d->previewLoadFullImageSize->isChecked())
    {
        QMessageBox::information(this, qApp->applicationName(), i18n("In order to enable this feature, the full-sized preview will be disabled."));

        d->previewLoadFullImageSize->setChecked(false);
    }
}

void SetupCamera::slotPreviewFullImageSizeClicked()
{
    if (d->previewItemsWhileDownload->isChecked() && d->previewLoadFullImageSize)
    {
        QMessageBox::information(this, qApp->applicationName(), i18n("If the full-sized preview is enabled it will affect the speed of previewing each item while download."));
        d->previewItemsWhileDownload->setChecked(false);
    }
}

}  // namespace Digikam
