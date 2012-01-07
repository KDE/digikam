/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : camera setup tab.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <config-digikam.h>
#include "gpcamera.h"
#include "filtercombo.h"
#include "importfilters.h"

namespace Digikam
{

class SetupCameraItem : public QTreeWidgetItem
{

public:

    SetupCameraItem(QTreeWidget* parent, CameraType* ctype)
        : QTreeWidgetItem(parent), m_ctype(0)
    {
        setCameraType(ctype);
    };

    ~SetupCameraItem()
    {
        delete m_ctype;
    };

    void setCameraType(CameraType* ctype)
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

CameraAutoDetectThread::CameraAutoDetectThread(QObject* parent)
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
        useDefaultTargetAlbum(0),
        target1AlbumSelector(0),
        listView(0),
        tab(0)
    {
    }

    static const QString configGroupName;
    static const QString configUseMetadataDateEntry;
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
    QCheckBox*           useDefaultTargetAlbum;

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
const QString SetupCamera::SetupCameraPriv::configUseDefaultTargetAlbum("UseDefaultTargetAlbum");
const QString SetupCamera::SetupCameraPriv::configDefaultTargetAlbumId("DefaultTargetAlbumId");
const QString SetupCamera::SetupCameraPriv::importFiltersConfigGroupName("Import Filters");

SetupCamera::SetupCamera(QWidget* parent)
    : QScrollArea(parent), d(new SetupCameraPriv)
{
    d->tab = new KTabWidget(viewport());
    setWidget(d->tab);
    setWidgetResizable(true);

    QWidget* panel = new QWidget(d->tab);
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
    d->useDefaultTargetAlbum = new QCheckBox(i18n("Use a default target album to download from camera"), panel2);
    d->target1AlbumSelector  = new AlbumSelectWidget(panel2);

    d->tab->insertTab(1, panel2, i18n("Behavior"));

    layout->setMargin(KDialog::spacingHint());
    layout->setSpacing(KDialog::spacingHint());
    layout->addWidget(d->useDateFromMetadata);
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
