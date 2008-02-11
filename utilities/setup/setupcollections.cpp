/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-01
 * Description : collections setup tab
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// QT includes.

#include <QGroupBox>
#include <QLabel>
#include <QDir>
#include <QList>
#include <QFileInfo>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTreeWidget>

// KDE includes.

#include <klocale.h>
#include <klineedit.h>
#include <kpagedialog.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kurlrequester.h>

// Local includes.

#include "albumsettings.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "scancontroller.h"
#include "setupcollections.h"
#include "setupcollections.moc"

namespace Digikam
{

class CollectionListViewItem : public QTreeWidgetItem
{

public:

    CollectionListViewItem(QTreeWidget *view, const QString& name, const CollectionLocation& collection)
        : QTreeWidgetItem(view)
    {
        QPixmap type, status;
        switch (collection.type())
        {
            case CollectionLocation::TypeVolumeHardWired:
                type = SmallIcon("drive-harddisk");
                break;
            case CollectionLocation::TypeVolumeRemovable:
                type = SmallIcon("drive-removable-media");
                break;
            case CollectionLocation::TypeNetwork:
                type = SmallIcon("folder-remote");
                break;
        }

        switch (collection.status())
        {
            case CollectionLocation::LocationNull:
                status = SmallIcon("flag-black");
                break;
            case CollectionLocation::LocationAvailable:
                status = SmallIcon("flag-green");
                break;
            case CollectionLocation::LocationHidden:
                status = SmallIcon("flag-yellow");
                break;
            case CollectionLocation::LocationUnavailable:
                type = SmallIcon("flag-red");
                break;
            case CollectionLocation::LocationDeleted:
                status = SmallIcon("edit-delete");
                break;
        }

        setIcon(1, type);
        setIcon(2, status);

        setName(name);
        setLocation(collection);
        setPathEditable(false);
    }

    CollectionListViewItem(QTreeWidget *view, const QString& name, const QString& path)
        : QTreeWidgetItem(view)
    {
        setIcon(1, QPixmap());
        setIcon(2, QPixmap());

        setName(name);
        setPath(path);
        setPathEditable(true);
    }

    ~CollectionListViewItem(){}

    void setPathEditable(bool b) { m_pathIsEditable = b; }
    bool pathIsEditable() { return m_pathIsEditable; }

    void setName(const QString& name)
    {
        m_name = name;
        setText(0, m_name);
    }

    QString name() { return m_name; }

    void setPath(const QString& path)
    {
        m_path = path;
        setText(3, m_path);
    }

    QString path() const { return m_path; }

    void setLocation(const CollectionLocation &location)
    {
        setPath(location.albumRootPath());
    }

private:

    bool    m_pathIsEditable;

    QString m_name;
    QString m_path;
};

class SetupCollectionsPriv
{
public:

    SetupCollectionsPriv()
    {
        nameLabel        = 0;
        nameEdit         = 0;
        pathLabel        = 0;
        pathEdit         = 0;
        mainDialog       = 0;
        listView         = 0;
        newButton        = 0;
        addButton        = 0;
        removeButton     = 0;
        replaceButton    = 0;
        databasePathEdit = 0;
        rootsPathChanged = false;
    }

    bool                       rootsPathChanged;

    QPushButton               *newButton;
    QPushButton               *addButton;
    QPushButton               *removeButton;
    QPushButton               *replaceButton;

    QLabel                    *pathLabel;
    QLabel                    *nameLabel;

    QTreeWidget               *listView;

    KLineEdit                 *nameEdit;

    KUrlRequester             *pathEdit;
    KUrlRequester             *databasePathEdit;

    KPageDialog               *mainDialog;

    QList<CollectionLocation>  collections;
};

SetupCollections::SetupCollections(KPageDialog* dialog, QWidget* parent)
                : QWidget(parent)
{
    d = new SetupCollectionsPriv;
    d->mainDialog = dialog;

    QVBoxLayout *layout = new QVBoxLayout( this );

    // --------------------------------------------------------

    QGroupBox *albumPathBox = new QGroupBox(i18n("Roots Album Path"), this);
    QGridLayout* grid       = new QGridLayout(albumPathBox);

    QLabel *albumPathLabel = new QLabel(i18n("Here you can set the paths to the root albums with your "
                                             "images. Write access is necessary to edit your images.\n"
                                             "You can use removable media and remote "
                                             "file systems shared over NFS for example."),
                                        albumPathBox);
    albumPathLabel->setWordWrap(true);

    d->listView = new QTreeWidget(albumPathBox);
    d->listView->setRootIsDecorated(false);
    d->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setColumnCount(4);
    d->listView->setWhatsThis(i18n("<p>This shows all root album paths used by digiKam "
                                   "as collections."));

    QStringList labels;
    labels.append( i18n("Name") );
    labels.append( i18n("Type") );
    labels.append( i18n("Status") );
    labels.append( i18n("Path") );
    d->listView->setHeaderLabels(labels);

    d->newButton     = new QPushButton(albumPathBox);
    d->addButton     = new QPushButton(albumPathBox);
    d->removeButton  = new QPushButton(albumPathBox);
    d->replaceButton = new QPushButton(albumPathBox);

    d->newButton->setText( i18n( "&New" ) );
    d->newButton->setIcon(SmallIcon("folder-new"));
    d->addButton->setText( i18n( "&Add" ) );
    d->addButton->setIcon(SmallIcon("list-add"));
    d->removeButton->setText( i18n( "&Remove" ) );
    d->removeButton->setIcon(SmallIcon("list-remove"));
    d->replaceButton->setText( i18n( "&Replace" ) );
    d->replaceButton->setIcon(SmallIcon("view-refresh"));
    d->removeButton->setEnabled(false);
    d->replaceButton->setEnabled(false);

    d->nameLabel     = new QLabel(i18n("Name:"), albumPathBox);
    d->nameEdit      = new KLineEdit(albumPathBox);
    d->nameEdit->setClearButtonShown(true);

    d->pathLabel     = new QLabel(i18n("Path:"), albumPathBox);
    d->pathEdit = new KUrlRequester(albumPathBox);
    d->pathEdit->setMode(KFile::Directory | KFile::LocalOnly | KFile::ExistingOnly);

    grid->addWidget(albumPathLabel,   0, 0, 1, 3);
    grid->addWidget(d->listView,      1, 0, 5, 2);
    grid->addWidget(d->newButton,     1, 2, 1, 1);
    grid->addWidget(d->addButton,     2, 2, 1, 1);
    grid->addWidget(d->removeButton,  3, 2, 1, 1);
    grid->addWidget(d->replaceButton, 4, 2, 1, 1);
    grid->addWidget(d->nameLabel,     6, 0, 1, 1);
    grid->addWidget(d->nameEdit,      6, 1, 1, 2);
    grid->addWidget(d->pathLabel,     7, 0, 1, 1);
    grid->addWidget(d->pathEdit,      7, 1, 1, 2);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(5, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());
    grid->setAlignment(Qt::AlignTop);

    // --------------------------------------------------------

    QGroupBox *dbPathBox = new QGroupBox(i18n("Database File Path"), this);
    QVBoxLayout *vlay    = new QVBoxLayout(dbPathBox);

    QLabel *databasePathLabel = new QLabel(i18n("Here you can enter the location on your computer "
                                                "where the digiKam database file will be stored. "
                                                "It is only one DB file common to all album roots.\n "
                                                "Write access is required to be able to edit image "
                                                "properties. \nPlease note that you cannot use a "
                                                "remote file system here, such as NFS."),
                                           dbPathBox);
    databasePathLabel->setWordWrap(true);

    d->databasePathEdit = new KUrlRequester(dbPathBox);
    d->databasePathEdit->setMode(KFile::Directory | KFile::LocalOnly);

    vlay->addWidget(databasePathLabel);
    vlay->addWidget(d->databasePathEdit);
    vlay->setSpacing(0);
    vlay->setMargin(KDialog::spacingHint());

    // --------------------------------------------------------

    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
    layout->addWidget(albumPathBox);
    layout->addWidget(dbPathBox);
    layout->addStretch();

    // --------------------------------------------------------

    connect(d->listView, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
            this, SLOT(slotSelectionChanged(QTreeWidgetItem*, int)));

    connect(d->newButton, SIGNAL(clicked()),
            this, SLOT(slotNewCollectionItem()));

    connect(d->addButton, SIGNAL(clicked()),
            this, SLOT(slotAddCollectionItem()));

    connect(d->removeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveCollectionItem()));

    connect(d->replaceButton, SIGNAL(clicked()),
            this, SLOT(slotReplaceCollectionItem()));

    connect(d->nameEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotAlbumNameEdited(const QString&)));

    connect(d->pathEdit, SIGNAL(urlSelected(const KUrl &)),
            this, SLOT(slotChangeAlbumPath(const KUrl &)));

    connect(d->pathEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotAlbumPathEdited(const QString&)));

    connect(d->databasePathEdit, SIGNAL(urlSelected(const KUrl &)),
            this, SLOT(slotChangeDatabasePath(const KUrl &)));

    connect(d->databasePathEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotDatabasePathEdited(const QString&)));

    // --------------------------------------------------------

    readSettings();
    adjustSize();
    checkforAddButton();
    checkforOkButton();
}

SetupCollections::~SetupCollections()
{
    delete d;
}

void SetupCollections::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    CollectionManager* manager = CollectionManager::instance();
    if (!manager) return;

    settings->setDatabaseFilePath(d->databasePathEdit->url().path());
    settings->saveSettings();

    // Check what root path need to be added to DB.

    int i                 = 0;
    QTreeWidgetItem *item = 0;
    do
    {
        item = d->listView->topLevelItem(i);
        CollectionListViewItem* lvItem = dynamic_cast<CollectionListViewItem*>(item);
        if (lvItem)
        {
            QString path(lvItem->path());

            bool exist = false;
            for (QList<CollectionLocation>::Iterator it2 = d->collections.begin();
                it2 != d->collections.end(); ++it2)
            {
                if ((*it2).albumRootPath() == path)
                    exist = true;
            }

            if (!exist)
            {
                CollectionLocation location = manager->addLocation(KUrl(path));
                lvItem->setLocation(location);
            }
        }
        i++;
    }
    while (item);

    d->collections = manager->allLocations();

    // Check what root path need to be removed from DB.

    for (QList<CollectionLocation>::Iterator it2 = d->collections.begin();
        it2 != d->collections.end(); ++it2)
    {
        bool exist = false;
        i          = 0;
        item       = 0;
        do
        {
            item = d->listView->topLevelItem(i);
            CollectionListViewItem* lvItem = dynamic_cast<CollectionListViewItem*>(item);
            if (lvItem)
            {
                if ((*it2).albumRootPath() == lvItem->path())
                    exist = true;
            }
            i++;
        }
        while (item);

        if (!exist)
            manager->removeLocation(*it2);
    }

    if (d->rootsPathChanged)
        ScanController::instance()->completeCollectionScan();
}

void SetupCollections::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    CollectionManager* manager = CollectionManager::instance();
    if (!manager) return;

    d->databasePathEdit->setUrl(settings->getDatabaseFilePath());

    d->collections = manager->allLocations();

    int i = 0;
    for (QList<CollectionLocation>::Iterator it = d->collections.begin();
         it != d->collections.end(); ++it)
    {
        QString name = i18n("Col. %1", i++);       // TODO: handle root album name set in DB.
        new CollectionListViewItem(d->listView, name, *it);
    }
}

void SetupCollections::slotSelectionChanged(QTreeWidgetItem *item, int)
{
    CollectionListViewItem* colItem = dynamic_cast<CollectionListViewItem*>(item);
    if (!colItem)
    {
        slotNewCollectionItem();
        return;
    }

    d->removeButton->setEnabled(true);
    d->replaceButton->setEnabled(true);
    d->nameEdit->setText(colItem->name());
    d->pathEdit->setUrl(colItem->path());
    d->pathEdit->setEnabled(colItem->pathIsEditable());
    checkforAddButton();
}

void SetupCollections::slotNewCollectionItem()
{
    d->removeButton->setEnabled(false);
    d->replaceButton->setEnabled(false);
    d->pathEdit->setEnabled(true);
    d->pathEdit->clear();
    d->nameEdit->clear();
    d->nameEdit->setFocus();
    checkforOkButton();
}

void SetupCollections::slotAddCollectionItem()
{
    if (!checkForCollection(d->nameEdit->text(), d->pathEdit->url().path()))
        return;

    new CollectionListViewItem(d->listView, d->nameEdit->text(), d->pathEdit->url().path());
    d->rootsPathChanged = true;
    d->pathEdit->clear();
    d->nameEdit->clear();
    checkforOkButton();
}

void SetupCollections::slotRemoveCollectionItem()
{
    QTreeWidgetItem *item = d->listView->currentItem();
    if (!item) return;

    delete item;
    d->rootsPathChanged = true;
    checkforOkButton();
}

void SetupCollections::slotReplaceCollectionItem()
{
    CollectionListViewItem* item = dynamic_cast<CollectionListViewItem*>(d->listView->currentItem());
    if (!item) return;
    if (item->name() == d->nameEdit->text() &&
        item->path() == d->pathEdit->url().path())
        return;

    if (!checkForCollection(d->nameEdit->text(), QString()))
        return;

    item->setName(d->nameEdit->text());
    item->setPath(d->pathEdit->url().path());
    d->rootsPathChanged = true;
    d->pathEdit->clear();
    d->nameEdit->clear();
    checkforOkButton();
}

void SetupCollections::slotChangeDatabasePath(const KUrl &result)
{
    QFileInfo targetPath(result.path());

    if (!result.isEmpty() && !targetPath.isWritable())
    {
        KMessageBox::information(0, i18n("No write access for this path to store database.\n"
                                         "Warning: the caption and tag features will not work."));
    }

    checkforOkButton();
}

void SetupCollections::slotDatabasePathEdited(const QString& newPath)
{
    if (!newPath.isEmpty() && !newPath.startsWith("/"))
    {
        d->databasePathEdit->setUrl(QDir::homePath() + '/' + newPath);
    }

    checkforOkButton();
}

void SetupCollections::slotAlbumNameEdited(const QString&)
{
    checkforAddButton();
}

void SetupCollections::slotChangeAlbumPath(const KUrl &result)
{
    if (KUrl(result).equals(KUrl(QDir::homePath()), KUrl::CompareWithoutTrailingSlash))
    {
        KMessageBox::sorry(0, i18n("Sorry; cannot use home directory as album library."));
    }
    else
    {
        QFileInfo targetPath(result.path());

        if (!result.isEmpty() && !targetPath.isWritable())
        {
            KMessageBox::information(0, i18n("No write access for this root album path.\n"
                                             "Warning: image and metadata editing will not work with this path."));
        }
    }

    checkforAddButton();
}

void SetupCollections::slotAlbumPathEdited(const QString& newPath)
{
    if (newPath.isEmpty())
    {
        d->replaceButton->setEnabled(false);
    }
    else if (!newPath.startsWith("/"))
    {
        d->pathEdit->setUrl(QDir::homePath() + '/' + newPath);
    }

    checkforAddButton();
}

void SetupCollections::checkforOkButton()
{
    bool albumOk = d->listView->topLevelItemCount() ? true : false;

    bool dbOk = false;
    if (!d->databasePathEdit->url().path().isEmpty())
    {
        QDir dbDir(d->databasePathEdit->url().path());
        dbOk = dbDir.exists();
    }

    d->mainDialog->enableButtonOk(dbOk && albumOk);
}

void SetupCollections::checkforAddButton()
{
    bool nameOk = !d->nameEdit->text().isEmpty();

    bool pathOk = false;
    if (d->pathEdit->isEnabled() && !d->pathEdit->url().path().isEmpty())
    {
        QDir pathDir(d->pathEdit->url().path());
        pathOk = pathDir.exists();
    }

    d->addButton->setEnabled(nameOk && pathOk);
}

bool SetupCollections::checkForCollection(const QString& name, const QString& path)
{
    int i                 = 0;
    QTreeWidgetItem *item = 0;
    do
    {
        item = d->listView->topLevelItem(i);
        CollectionListViewItem* lvItem = dynamic_cast<CollectionListViewItem*>(item);
        if (lvItem)
        {
            if (lvItem->name() == name)
            {
                KMessageBox::information(0, i18n("A collection named \"%1\" already exist.", name));
                return false;
            }

            if (lvItem->path() == path)
            {
                KMessageBox::information(0, i18n("A collection with the path \"%1\" already exist.", path));
                return false;
            }
        }
        i++;
    }
    while (item);

    return true;
}

}  // namespace Digikam
