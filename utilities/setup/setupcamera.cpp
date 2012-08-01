/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : camera setup tab.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupcamera.moc"

// Qt includes

#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QPixmap>
#include <QCheckBox>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>
#include <QListWidget>

// KDE includes

#include <kapplication.h>
#include <kcursor.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktoolinvocation.h>
#include <ktabwidget.h>
#include <kurllabel.h>
#include <kconfig.h>

// Local includes

#include "albumselectwidget.h"
#include "cameralist.h"
#include "cameraselection.h"
#include "cameratype.h"
#include "config-digikam.h"
#include "gpcamera.h"
#include "filtercombo.h"
#include "importfilters.h"
#include "dfontselect.h"
#include "importsettings.h"

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

class CameraAutoDetectThread::CameraAutoDetectThreadPriv
{
public:

    CameraAutoDetectThreadPriv()
    {
        result = 0;
    }

    int     result;

    QString model;
    QString port;
};

CameraAutoDetectThread::CameraAutoDetectThread(QObject* const parent)
    : DBusyThread(parent), d(new CameraAutoDetectThreadPriv)
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

class SetupCamera::SetupCameraPriv
{
public:

    SetupCameraPriv() :
        addButton(0),
        removeButton(0),
        editButton(0),
        autoDetectButton(0),
        importAddButton(0),
        importRemoveButton(0),
        importEditButton(0),
        useDateFromMetadata(0),
        turnHighQualityThumbs(0),
        useDefaultTargetAlbum(0),
        iconShowNameBox(0),
        iconShowSizeBox(0),
        iconShowModDateBox(0),
        iconShowResolutionBox(0),
        //TODO: iconShowTagsBox(0),
        //TODO: iconShowOverlaysBox(0),
        //TODO: iconShowRatingBox(0),
        iconShowFormatBox(0),
        previewLoadFullImageSize(0),
        previewShowIcons(0),
        leftClickActionComboBox(0),
        iconViewFontSelect(0),
        target1AlbumSelector(0),
        listView(0),
        importListView(0),
        tab(0),
        ignoreNamesEdit(0),
        ignoreExtensionsEdit(0)
    {
    }

    static const QString configGroupName;
    static const QString configUseMetadataDateEntry;
    static const QString configTrunHighQualityThumbs;
    static const QString configUseDefaultTargetAlbum;
    static const QString configDefaultTargetAlbumId;
    static const QString importFiltersConfigGroupName;

    QPushButton*         addButton;
    QPushButton*         removeButton;
    QPushButton*         editButton;
    QPushButton*         autoDetectButton;
    QPushButton*         importAddButton;
    QPushButton*         importRemoveButton;
    QPushButton*         importEditButton;

    QCheckBox*           useDateFromMetadata;
    QCheckBox*           turnHighQualityThumbs;
    QCheckBox*           useDefaultTargetAlbum;

    QCheckBox*           iconShowNameBox;
    QCheckBox*           iconShowSizeBox;
    QCheckBox*           iconShowModDateBox;
    QCheckBox*           iconShowResolutionBox;
    //TODO: QCheckBox*           iconShowTagsBox;
    //TODO: QCheckBox*           iconShowOverlaysBox;
    //TODO: QCheckBox*           iconShowRatingBox;
    QCheckBox*           iconShowFormatBox;
    QCheckBox*           previewLoadFullImageSize;
    QCheckBox*           previewShowIcons;

    KComboBox*           leftClickActionComboBox;

    DFontSelect*         iconViewFontSelect;


    AlbumSelectWidget*   target1AlbumSelector;

    QTreeWidget*         listView;
    QListWidget*         importListView;

    KTabWidget*          tab;

    QLineEdit*           ignoreNamesEdit;
    QLineEdit*           ignoreExtensionsEdit;

    FilterList           filters;
};

const QString SetupCamera::SetupCameraPriv::configGroupName("Camera Settings");
const QString SetupCamera::SetupCameraPriv::configUseMetadataDateEntry("UseThemeBackgroundColor");
const QString SetupCamera::SetupCameraPriv::configTrunHighQualityThumbs("TurnHighQualityThumbs");
const QString SetupCamera::SetupCameraPriv::configUseDefaultTargetAlbum("UseDefaultTargetAlbum");
const QString SetupCamera::SetupCameraPriv::configDefaultTargetAlbumId("DefaultTargetAlbumId");
const QString SetupCamera::SetupCameraPriv::importFiltersConfigGroupName("Import Filters");

SetupCamera::SetupCamera(QWidget* const parent)
    : QScrollArea(parent), d(new SetupCameraPriv)
{
    d->tab            = new KTabWidget(viewport());
    setWidget(d->tab);
    setWidgetResizable(true);

    QWidget* panel    = new QWidget(d->tab);
    panel->setAutoFillBackground(false);

    QGridLayout* grid = new QGridLayout(panel);
    d->listView       = new QTreeWidget(panel);
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
    d->listView->header()->setResizeMode(0, QHeaderView::ResizeToContents);
    d->listView->header()->setResizeMode(1, QHeaderView::Stretch);
    d->listView->header()->setResizeMode(2, QHeaderView::Stretch);
    d->listView->header()->setResizeMode(3, QHeaderView::Stretch);

    // -------------------------------------------------------------

    d->addButton        = new QPushButton(panel);
    d->removeButton     = new QPushButton(panel);
    d->editButton       = new QPushButton(panel);
    d->autoDetectButton = new QPushButton(panel);

    d->addButton->setText(i18n("&Add..."));
    d->addButton->setIcon(SmallIcon("list-add"));
    d->removeButton->setText(i18n("&Remove"));
    d->removeButton->setIcon(SmallIcon("list-remove"));
    d->editButton->setText(i18n("&Edit..."));
    d->editButton->setIcon(SmallIcon("configure"));
    d->autoDetectButton->setText(i18n("Auto-&Detect"));
    d->autoDetectButton->setIcon(SmallIcon("system-search"));
    d->removeButton->setEnabled(false);
    d->editButton->setEnabled(false);

    // -------------------------------------------------------------

    QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

    KUrlLabel* gphotoLogoLabel = new KUrlLabel(panel);
    gphotoLogoLabel->setText(QString());
    gphotoLogoLabel->setUrl("http://www.gphoto.org");
    gphotoLogoLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-gphoto.png")));
    gphotoLogoLabel->setToolTip(i18n("Visit Gphoto project website"));

#ifndef HAVE_GPHOTO2
    // If digiKam is compiled without Gphoto2 support, we hide widgets relevant.
    d->autoDetectButton->hide();
    gphotoLogoLabel->hide();
#endif /* HAVE_GPHOTO2 */

    // -------------------------------------------------------------

    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());
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

    QWidget* panel2          = new QWidget(d->tab);
    panel2->setAutoFillBackground(false);

    QVBoxLayout* layout      = new QVBoxLayout(panel2);
    d->useDateFromMetadata   = new QCheckBox(i18n("Use date from metadata to sort items instead file-system date (makes connection slower)"), panel2);
    d->turnHighQualityThumbs = new QCheckBox(i18n("Turn on high quality thumbnail loading (slower loading)"), panel2);
    d->useDefaultTargetAlbum = new QCheckBox(i18n("Use a default target album to download from camera"), panel2);
    d->target1AlbumSelector  = new AlbumSelectWidget(panel2);

    d->tab->insertTab(1, panel2, i18n("Behavior"));

    layout->setMargin(KDialog::spacingHint());
    layout->setSpacing(KDialog::spacingHint());
    layout->addWidget(d->useDateFromMetadata);
    layout->addWidget(d->turnHighQualityThumbs);
    layout->addWidget(d->useDefaultTargetAlbum);
    layout->addWidget(d->target1AlbumSelector);
    layout->addStretch();

    // -------------------------------------------------------------

    QWidget* panel3 = new QWidget(d->tab);
    panel3->setAutoFillBackground(false);

    QGridLayout* importGrid = new QGridLayout(panel3);
    d->importListView       = new QListWidget(panel3);
    d->importListView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->importListView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->importListView->setWhatsThis(i18n("Here you can see filters that can be used to filter "
                                         "files in import dialog."));

    d->importAddButton    = new QPushButton(panel3);
    d->importRemoveButton = new QPushButton(panel3);
    d->importEditButton   = new QPushButton(panel3);
    QSpacerItem* spacer2  = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

    QGroupBox* groupBox         = new QGroupBox(panel3);
    QVBoxLayout* verticalLayout = new QVBoxLayout(groupBox);
    QLabel* label               = new QLabel(groupBox);
    verticalLayout->addWidget(label);
    d->ignoreNamesEdit          = new QLineEdit(groupBox);
    verticalLayout->addWidget(d->ignoreNamesEdit);
    QLabel* label2              = new QLabel(groupBox);
    verticalLayout->addWidget(label2);
    d->ignoreExtensionsEdit     = new QLineEdit(groupBox);
    verticalLayout->addWidget(d->ignoreExtensionsEdit);

    groupBox->setTitle(i18n("Always ignore"));
    label->setText(i18n("Ignored file names:"));
    label2->setText(i18n("Ignored file extensions:"));
    d->importAddButton->setText(i18n("&Add..."));
    d->importAddButton->setIcon(SmallIcon("list-add"));
    d->importRemoveButton->setText(i18n("&Remove"));
    d->importRemoveButton->setIcon(SmallIcon("list-remove"));
    d->importEditButton->setText(i18n("&Edit..."));
    d->importEditButton->setIcon(SmallIcon("configure"));
    d->importRemoveButton->setEnabled(false);
    d->importEditButton->setEnabled(false);

    importGrid->setMargin(KDialog::spacingHint());
    importGrid->setSpacing(KDialog::spacingHint());
    importGrid->setAlignment(Qt::AlignTop);
    importGrid->addWidget(d->importListView,     0, 0, 4, 1);
    importGrid->addWidget(groupBox,              5, 0, 1, 1);
    importGrid->addWidget(d->importAddButton,    0, 1, 1, 1);
    importGrid->addWidget(d->importRemoveButton, 1, 1, 1, 1);
    importGrid->addWidget(d->importEditButton,   2, 1, 1, 1);
    importGrid->addItem(spacer2,                 3, 1, 1, 1);

    d->tab->insertTab(2, panel3, i18n("Import Filters"));

    // -- Import Icon View ----------------------------------------------------------

    QWidget* panel4 = new QWidget(d->tab);
    panel4->setAutoFillBackground(false);

    QVBoxLayout* layout2 = new QVBoxLayout(panel4);

    QGroupBox* iconViewGroup = new QGroupBox(i18n("Icon-View Options"), panel4);
    QGridLayout* grid2       = new QGridLayout(iconViewGroup);

    d->iconShowNameBox       = new QCheckBox(i18n("Show file&name"), iconViewGroup);
    d->iconShowNameBox->setWhatsThis(i18n("Set this option to show the filename below the image thumbnail."));

    d->iconShowSizeBox       = new QCheckBox(i18n("Show file si&ze"), iconViewGroup);
    d->iconShowSizeBox->setWhatsThis(i18n("Set this option to show the file size below the image thumbnail."));

    d->iconShowModDateBox    = new QCheckBox(i18n("Show file &modification date"), iconViewGroup);
    d->iconShowModDateBox->setWhatsThis(i18n("Set this option to show the file modification date "
                                             "below the image thumbnail."));

    //d->iconShowResolutionBox = new QCheckBox(i18n("Show ima&ge dimensions"), iconViewGroup);
    //d->iconShowResolutionBox->setWhatsThis(i18n("Set this option to show the image size in pixels "
                                                //"below the image thumbnail."));

    d->iconShowFormatBox     = new QCheckBox(i18n("Show image Format"), iconViewGroup);
    d->iconShowFormatBox->setWhatsThis(i18n("Set this option to show image format over image thumbnail."));

    //TODO: d->iconShowTagsBox       = new QCheckBox(i18n("Show digiKam &tags"), iconViewGroup);
    //TODO: d->iconShowTagsBox->setWhatsThis(i18n("Set this option to show the digiKam tags "
                                          //"below the image thumbnail."));

    //TODO: d->iconShowRatingBox     = new QCheckBox(i18n("Show digiKam &rating"), iconViewGroup);
    //TODO: d->iconShowRatingBox->setWhatsThis(i18n("Set this option to show the digiKam rating "
                                            //"below the image thumbnail."));

    //TODO: d->iconShowOverlaysBox   = new QCheckBox(i18n("Show rotation overlay buttons"), iconViewGroup);
    //TODO: d->iconShowOverlaysBox->setWhatsThis(i18n("Set this option to show overlay buttons on "
                                              //"the image thumbnail for image rotation."));

    QLabel* leftClickLabel     = new QLabel(i18n("Thumbnail click action:"), iconViewGroup);
    d->leftClickActionComboBox = new KComboBox(iconViewGroup);
    d->leftClickActionComboBox->addItem(i18n("Show embedded preview"), ImportSettings::ShowPreview);
    d->leftClickActionComboBox->addItem(i18n("Start image editor"), ImportSettings::StartEditor);
    d->leftClickActionComboBox->setToolTip(i18n("Choose what should happen when you click on a thumbnail."));

    d->iconViewFontSelect = new DFontSelect(i18n("Icon View font:"), panel);
    d->iconViewFontSelect->setToolTip(i18n("Select here the font used to display text in Icon Views."));

    grid2->addWidget(d->iconShowNameBox,          0, 0, 1, 1);
    grid2->addWidget(d->iconShowSizeBox,          1, 0, 1, 1);
    grid2->addWidget(d->iconShowModDateBox,       3, 0, 1, 1);
    //TODO: grid2->addWidget(d->iconShowResolutionBox,    4, 0, 1, 1);
    grid2->addWidget(d->iconShowFormatBox,        4, 0, 1, 1);

    //TODO: grid2->addWidget(d->iconShowTagsBox,          2, 1, 1, 1);
    //TODO: grid2->addWidget(d->iconShowRatingBox,        3, 1, 1, 1);
    //TODO: grid2->addWidget(d->iconShowOverlaysBox,      4, 1, 1, 1);

    grid2->addWidget(leftClickLabel,              5, 0, 1, 1);
    grid2->addWidget(d->leftClickActionComboBox,  6, 1, 1, 1);
    grid2->addWidget(d->iconViewFontSelect,       7, 0, 1, 2);
    grid2->setSpacing(KDialog::spacingHint());
    grid2->setMargin(KDialog::spacingHint());

    // --------------------------------------------------------

    QGroupBox* interfaceOptionsGroup = new QGroupBox(i18n("Preview Options"), panel);
    QGridLayout* grid3               = new QGridLayout(interfaceOptionsGroup);

    d->previewLoadFullImageSize      = new QCheckBox(i18n("Embedded preview loads full-sized images."), interfaceOptionsGroup);
    d->previewLoadFullImageSize->setWhatsThis(i18n("<p>Set this option to load images at their full size "
                                                   "for preview, rather than at a reduced size. As this option "
                                                   "will make it take longer to load images, only use it if you have "
                                                   "a fast computer.</p>"
                                                   "<p><b>Note:</b> for Raw images, a half size version of the Raw data "
                                                   "is used instead of the embedded JPEG preview.</p>"));

    d->previewShowIcons              = new QCheckBox(i18n("Show icons and text over preview"), interfaceOptionsGroup);
    d->previewShowIcons->setWhatsThis(i18n("Uncheck this if you don't want to see icons and text in the image preview."));

    grid3->setMargin(KDialog::spacingHint());
    grid3->setSpacing(KDialog::spacingHint());
    grid3->addWidget(d->previewLoadFullImageSize, 0, 0, 1, 2);
    grid3->addWidget(d->previewShowIcons,         1, 0, 1, 2);

    layout2->setMargin(0);
    layout2->setSpacing(KDialog::spacingHint());
    layout2->addWidget(iconViewGroup);
    layout2->addWidget(interfaceOptionsGroup);
    layout2->addStretch();

    d->tab->insertTab(3, panel4, i18n("Icon View"));

    // -------------------------------------------------------------

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);

    adjustSize();

    // -------------------------------------------------------------

    connect(gphotoLogoLabel, SIGNAL(leftClickedUrl(QString)),
            this, SLOT(slotProcessGphotoUrl(QString)));

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

    readSettings();
}

SetupCamera::~SetupCamera()
{
    delete d;
}

void SetupCamera::readSettings()
{
    // Populate cameras --------------------------------------

    CameraList* clist = CameraList::defaultList();

    if (clist)
    {
        QList<CameraType*>* cl = clist->cameraList();

        foreach(CameraType* ctype, *cl)
        {
            new SetupCameraItem(d->listView, ctype);
        }
    }

    // -------------------------------------------------------

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->useDateFromMetadata->setChecked(group.readEntry(d->configUseMetadataDateEntry, false));
    d->turnHighQualityThumbs->setChecked(group.readEntry(d->configTrunHighQualityThumbs, false));
    d->useDefaultTargetAlbum->setChecked(group.readEntry(d->configUseDefaultTargetAlbum, false));
    PAlbum* album = AlbumManager::instance()->findPAlbum(group.readEntry(d->configDefaultTargetAlbumId, 0));
    d->target1AlbumSelector->setCurrentAlbum(album);
    d->target1AlbumSelector->setEnabled(d->useDefaultTargetAlbum->isChecked());

    // -------------------------------------------------------

    KConfigGroup importGroup = config->group(d->importFiltersConfigGroupName);

    for (int i = 0; true; ++i)
    {
        QString filter = importGroup.readEntry(QString("Filter%1").arg(i), QString());

        if (filter.isEmpty())
        {
            break;
        }

        Filter* f = new Filter;
        f->fromString(filter);
        d->filters.append(f);
    }

    FilterComboBox::defaultFilters(&d->filters);
    foreach(Filter* f, d->filters)
    {
        new QListWidgetItem(f->name, d->importListView);
    }
    d->ignoreNamesEdit->setText(importGroup.readEntry("IgnoreNames", FilterComboBox::defaultIgnoreNames));
    d->ignoreExtensionsEdit->setText(importGroup.readEntry("IgnoreExtensions", FilterComboBox::defaultIgnoreExtensions));

    ImportSettings* settings = ImportSettings::instance();

    if (!settings)
    {
        return;
    }

    d->iconShowNameBox->setChecked(settings->getIconShowName());
    //TODO: d->iconShowTagsBox->setChecked(settings->getIconShowTags());
    d->iconShowSizeBox->setChecked(settings->getIconShowSize());
    d->iconShowModDateBox->setChecked(settings->getIconShowModDate());
    //TODO: d->iconShowResolutionBox->setChecked(settings->getIconShowResolution());
    //TODO: d->iconShowOverlaysBox->setChecked(settings->getIconShowOverlays());
    //TODO: d->iconShowRatingBox->setChecked(settings->getIconShowRating());
    d->iconShowFormatBox->setChecked(settings->getIconShowImageFormat());
    d->iconViewFontSelect->setFont(settings->getIconViewFont());

    d->leftClickActionComboBox->setCurrentIndex((int)settings->getItemLeftClickAction());

    d->previewLoadFullImageSize->setChecked(settings->getPreviewLoadFullImageSize());
    d->previewShowIcons->setChecked(settings->getPreviewShowIcons());
}

void SetupCamera::applySettings()
{
    // Save camera devices -----------------------------------

    CameraList* clist = CameraList::defaultList();

    if (clist)
    {
        clist->clear();

        QTreeWidgetItemIterator it(d->listView);

        while (*it)
        {
            SetupCameraItem* item = dynamic_cast<SetupCameraItem*>(*it);

            if (item)
            {
                CameraType* ctype = item->cameraType();

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

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configUseMetadataDateEntry, d->useDateFromMetadata->isChecked());
    group.writeEntry(d->configTrunHighQualityThumbs, d->turnHighQualityThumbs->isChecked());
    group.writeEntry(d->configUseDefaultTargetAlbum, d->useDefaultTargetAlbum->isChecked());
    PAlbum* album = d->target1AlbumSelector->currentAlbum();
    group.writeEntry(d->configDefaultTargetAlbumId, album ? album->id() : 0);
    group.sync();

    // -------------------------------------------------------

    KConfigGroup importGroup = config->group(d->importFiltersConfigGroupName);

    importGroup.deleteGroup();

    for (int i = 0; i < d->filters.count(); ++i)
    {
        importGroup.writeEntry(QString("Filter%1").arg(i), d->filters[i]->toString());
    }

    importGroup.writeEntry("IgnoreNames", d->ignoreNamesEdit->text());
    importGroup.writeEntry("IgnoreExtensions", d->ignoreExtensionsEdit->text());
    importGroup.sync();

    ImportSettings* settings = ImportSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setIconShowName(d->iconShowNameBox->isChecked());
    //TODO: settings->setIconShowTags(d->iconShowTagsBox->isChecked());
    settings->setIconShowSize(d->iconShowSizeBox->isChecked());
    settings->setIconShowModDate(d->iconShowModDateBox->isChecked());
    //TODO: settings->setIconShowResolution(d->iconShowResolutionBox->isChecked());
    //TODO: settings->setIconShowOverlays(d->iconShowOverlaysBox->isChecked());
    //TODO: settings->setIconShowRating(d->iconShowRatingBox->isChecked());
    settings->setIconShowImageFormat(d->iconShowFormatBox->isChecked());
    settings->setIconViewFont(d->iconViewFontSelect->font());

    settings->setItemLeftClickAction((ImportSettings::ItemLeftClickAction)
                                     d->leftClickActionComboBox->currentIndex());

    settings->setPreviewLoadFullImageSize(d->previewLoadFullImageSize->isChecked());
    settings->setPreviewShowIcons(d->previewShowIcons->isChecked());
    settings->saveSettings();
}

bool SetupCamera::checkSettings()
{
    if (d->useDefaultTargetAlbum->isChecked() && !d->target1AlbumSelector->currentAlbum())
    {
        d->tab->setCurrentIndex(1);
        KMessageBox::information(this, i18n("No default target album have been selected to process download "
                                            "from camera device. Please select one."));
        return false;
    }

    return true;
}

void SetupCamera::slotProcessGphotoUrl(const QString& url)
{
    KToolInvocation::self()->invokeBrowser(url);
}

void SetupCamera::slotSelectionChanged()
{
    QTreeWidgetItem* item = d->listView->currentItem();

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
    CameraSelection* select = new CameraSelection;

    connect(select, SIGNAL(signalOkClicked(const QString&, const QString&,
                                           const QString&, const QString&)),
            this,   SLOT(slotAddedCamera(const QString&, const QString&,
                                         const QString&, const QString&)));

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
    SetupCameraItem* item = dynamic_cast<SetupCameraItem*>(d->listView->currentItem());

    delete item;
}

void SetupCamera::slotEditCamera()
{
    SetupCameraItem* item = dynamic_cast<SetupCameraItem*>(d->listView->currentItem());

    if (!item)
    {
        return;
    }

    CameraType* ctype = item->cameraType();

    if (!ctype)
    {
        return;
    }

    CameraSelection* select = new CameraSelection;
    select->setCamera(ctype->title(), ctype->model(), ctype->port(), ctype->path());

    connect(select, SIGNAL(signalOkClicked(const QString&, const QString&,
                                           const QString&, const QString&)),
            this,   SLOT(slotEditedCamera(const QString&, const QString&,
                                          const QString&, const QString&)));

    select->show();
}

void SetupCamera::slotEditedCamera(const QString& title, const QString& model,
                                   const QString& port, const QString& path)
{
    SetupCameraItem* item = dynamic_cast<SetupCameraItem*>(d->listView->currentItem());

    if (!item)
    {
        return;
    }

    CameraType ctype(title, model, port, path, 1);
    item->setCameraType(&ctype);
}

void SetupCamera::slotAutoDetectCamera()
{
    DBusyDlg* dlg                  = new DBusyDlg(i18n("Device detection under progress, please wait..."), this);
    CameraAutoDetectThread* thread = new CameraAutoDetectThread(this);
    dlg->setBusyThread(thread);
    dlg->exec();

    QString model = thread->model();
    QString port  = thread->port();
    int ret       = thread->result();

    if (ret != 0)
    {
        KMessageBox::error(this, i18n("Failed to auto-detect camera.\n"
                                      "Please check if your camera is turned on "
                                      "and retry or try setting it manually."));
        return;
    }

    // NOTE: See note in digikam/digikam/cameralist.cpp
    if (port.startsWith(QLatin1String("usb:")))
    {
        port = "usb:";
    }

    if (!d->listView->findItems(model, Qt::MatchExactly, 1).isEmpty())
    {
        KMessageBox::information(this, i18n("Camera '%1' (%2) is already in list.", model, port));
    }
    else
    {
        KMessageBox::information(this, i18n("Found camera '%1' (%2) and added it to the list.", model, port));
        slotAddedCamera(model, model, port, QString("/"));
    }
}

void SetupCamera::slotImportSelectionChanged()
{
    QListWidgetItem* item = d->importListView->currentItem();

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
        Filter* f = new Filter;
        dlg.getData(f);
        d->filters.append(f);
        new QListWidgetItem(f->name, d->importListView);
    }

    slotImportSelectionChanged();
}

void SetupCamera::slotRemoveFilter()
{
    int i = d->importListView->currentRow();
    delete d->filters.takeAt(i);
    delete d->importListView->takeItem(i);
    slotImportSelectionChanged();
}

void SetupCamera::slotEditFilter()
{
    int i = d->importListView->currentRow();
    Filter filter = *d->filters.at(i);
    ImportFilters dlg(this);
    dlg.setData(filter);

    if (dlg.exec() == QDialog::Accepted)
    {
        Filter* f = d->filters.at(i);
        dlg.getData(f);
        d->importListView->currentItem()->setText(f->name);
    }
}

}  // namespace Digikam
