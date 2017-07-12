/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-15
 * Description : Managemenet dialogs for GPS bookmarks
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "bookmarksdlg.h"

// Qt includes

#include <QMenu>
#include <QFile>
#include <QIcon>
#include <QHeaderView>
#include <QMessageBox>
#include <QToolButton>
#include <QAction>
#include <QCloseEvent>
#include <QObject>
#include <QUndoCommand>
#include <QVariant>
#include <QApplication>
#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "bookmarksmngr.h"
#include "bookmarknode.h"
#include "dxmlguiwindow.h"
#include "imagepropertiesgpstab.h"
#include "gpsimageinfo.h"

namespace Digikam
{

class AddBookmarkDialog::Private
{
public:

    Private() :
        manager(0),
        proxyModel(0),
        location(0),
        title(0),
        desc(0)
    {
    }

    QString                url;
    BookmarksManager*      manager;
    AddBookmarkProxyModel* proxyModel;
    QComboBox*             location;
    QLineEdit*             title;
    QLineEdit*             desc;
};

AddBookmarkDialog::AddBookmarkDialog(const QString& url,
                                     const QString& title,
                                     QWidget* const parent,
                                     BookmarksManager* const mngr)
    : QDialog(parent),
      d(new Private)
{
    d->url     = url;
    d->manager = mngr;

    setWindowFlags(Qt::Sheet);
    setWindowTitle(tr2i18n("Add Bookmark", 0));
    setObjectName(QLatin1String("AddBookmarkDialog"));
    resize(350, 300);

    QLabel* const label = new QLabel(this);
    label->setText(i18n("Type a name and a comment for the bookmark, "
                        "and choose where to keep it.", 0));
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(true);

    d->title            = new QLineEdit(this);
    d->title->setPlaceholderText(i18n("Bookmark title"));
    d->title->setText(title);

    d->desc             = new QLineEdit(this);
    d->desc->setPlaceholderText(i18n("Bookmark comment"));
    d->location         = new QComboBox(this);

    QSpacerItem* const verticalSpacer = new QSpacerItem(20, 2, QSizePolicy::Minimum,
                                                        QSizePolicy::Expanding);

    QDialogButtonBox* const buttonBox = new QDialogButtonBox(this);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    buttonBox->setCenterButtons(false);

    QVBoxLayout* const vbox = new QVBoxLayout(this);
    vbox->addWidget(label);
    vbox->addWidget(d->title);
    vbox->addWidget(d->desc);
    vbox->addWidget(d->location);
    vbox->addItem(verticalSpacer);
    vbox->addWidget(buttonBox);

    QTreeView* const view       = new QTreeView(this);
    d->proxyModel               = new AddBookmarkProxyModel(this);
    BookmarksModel* const model = d->manager->bookmarksModel();
    d->proxyModel->setSourceModel(model);
    view->setModel(d->proxyModel);
    view->expandAll();
    view->header()->setStretchLastSection(true);
    view->header()->hide();
    view->setItemsExpandable(false);
    view->setRootIsDecorated(false);
    view->setIndentation(10);
    view->show();

    BookmarkNode* const menu = d->manager->bookmarks();
    QModelIndex idx          = d->proxyModel->mapFromSource(model->index(menu));
    view->setCurrentIndex(idx);

    d->location->setModel(d->proxyModel);
    d->location->setView(view);

    connect(buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));

    connect(buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));
}

AddBookmarkDialog::~AddBookmarkDialog()
{
    delete d;
}

void AddBookmarkDialog::accept()
{
    QModelIndex index = d->location->view()->currentIndex();
    index             = d->proxyModel->mapToSource(index);

    if (!index.isValid())
        index = d->manager->bookmarksModel()->index(0, 0);

    BookmarkNode* const parent   = d->manager->bookmarksModel()->node(index);
    BookmarkNode* const bookmark = new BookmarkNode(BookmarkNode::Bookmark);
    bookmark->url                = d->url;
    bookmark->title              = d->title->text();
    bookmark->desc               = d->desc->text();
    bookmark->dateAdded          = QDateTime::currentDateTime();
    d->manager->addBookmark(parent, bookmark);
    d->manager->save();
    QDialog::accept();
}

// ----------------------------------------------------------------

class BookmarksDialog::Private
{
public:

    Private() :
        manager(0),
        bookmarksModel(0),
        proxyModel(0),
        search(0),
        tree(0),
        mapView(0)
    {
    }

    BookmarksManager*      manager;
    BookmarksModel*        bookmarksModel;
    TreeProxyModel*        proxyModel;
    SearchTextBar*         search;
    QTreeView*             tree;
    ImagePropertiesGPSTab* mapView;
};

BookmarksDialog::BookmarksDialog(QWidget* const parent, BookmarksManager* const mngr)
    : QDialog(parent),
      d(new Private)
{
    d->manager = mngr;

    setObjectName(QLatin1String("GeolocationBookmarksEditDialog"));
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(i18n("Edit Geolocation Bookmarks"));
    resize(750, 450);

    d->search  = new SearchTextBar(this, QLatin1String("DigikamBookmarksGeolocationSearchBar"));
    d->search->setObjectName(QLatin1String("search"));

    d->tree    = new QTreeView(this);
    d->tree->setUniformRowHeights(true);
    d->tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    d->tree->setSelectionMode(QAbstractItemView::ContiguousSelection);
    d->tree->setTextElideMode(Qt::ElideMiddle);
    d->tree->setDragDropMode(QAbstractItemView::InternalMove);
    d->tree->setAlternatingRowColors(true);
    d->tree->setContextMenuPolicy(Qt::CustomContextMenu);

    d->mapView = new ImagePropertiesGPSTab(this);

    QPushButton* const removeButton    = new QPushButton(this);
    removeButton->setText(i18n("&Remove"));

    QPushButton* const addFolderButton = new QPushButton(this);
    addFolderButton->setText(i18n("Add Folder"));

    QSpacerItem* const spacerItem1     = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    QDialogButtonBox* const buttonBox  = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

    QHBoxLayout* const hbox = new QHBoxLayout();
    hbox->addWidget(removeButton);
    hbox->addWidget(addFolderButton);
    hbox->addItem(spacerItem1);
    hbox->addWidget(buttonBox);

    QGridLayout* const grid = new QGridLayout(this);
    grid->addWidget(d->search,  0, 0, 1, 2);
    grid->addWidget(d->tree,    1, 0, 1, 2);
    grid->addLayout(hbox,       2, 0, 1, 3);
    grid->addWidget(d->mapView, 0, 2, 2, 1);
    grid->setColumnStretch(1, 10);

    d->bookmarksModel       = d->manager->bookmarksModel();
    d->proxyModel           = new TreeProxyModel(this);
    d->proxyModel->setSourceModel(d->bookmarksModel);
    d->tree->setModel(d->proxyModel);
    d->tree->setExpanded(d->proxyModel->index(0, 0), true);
    d->tree->header()->setSectionResizeMode(QHeaderView::Stretch);

    connect(buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));

    connect(buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));

    connect(d->search, SIGNAL(textChanged(QString)),
            d->proxyModel, SLOT(setFilterFixedString(QString)));

    connect(d->proxyModel, SIGNAL(signalFilterAccepts(bool)),
            d->search, SLOT(slotSearchResult(bool)));

    connect(removeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveOne()));

    connect(d->tree, SIGNAL(clicked(QModelIndex)),
            this, SLOT(slotOpenInMap(QModelIndex)));

    connect(d->tree, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotCustomContextMenuRequested(QPoint)));

    connect(addFolderButton, SIGNAL(clicked()),
            this, SLOT(slotNewFolder()));

    readSettings();
}

BookmarksDialog::~BookmarksDialog()
{
    delete d;
}

void BookmarksDialog::accept()
{
    d->manager->save();
    QDialog::accept();
}

void BookmarksDialog::closeEvent(QCloseEvent* e)
{
    if (!e) return;

    saveSettings();
    e->accept();
}

bool BookmarksDialog::saveExpandedNodes(const QModelIndex& parent)
{
    bool changed = false;

    for (int i = 0 ; i < d->proxyModel->rowCount(parent) ; i++)
    {
        QModelIndex child             = d->proxyModel->index(i, 0, parent);
        QModelIndex sourceIndex       = d->proxyModel->mapToSource(child);
        BookmarkNode* const childNode = d->bookmarksModel->node(sourceIndex);
        bool wasExpanded              = childNode->expanded;

        if (d->tree->isExpanded(child))
        {
            childNode->expanded = true;
            changed            |= saveExpandedNodes(child);
        }
        else
        {
            childNode->expanded = false;
        }

        changed |= (wasExpanded != childNode->expanded);
    }

    return changed;
}

void BookmarksDialog::expandNodes(BookmarkNode* const node)
{
    for (int i = 0 ; i < node->children().count() ; i++)
    {
        BookmarkNode* const childNode = node->children()[i];

        if (childNode->expanded)
        {
            QModelIndex idx = d->bookmarksModel->index(childNode);
            idx             = d->proxyModel->mapFromSource(idx);
            d->tree->setExpanded(idx, true);
            expandNodes(childNode);
        }
    }
}

void BookmarksDialog::slotCustomContextMenuRequested(const QPoint& pos)
{
    QModelIndex index = d->tree->indexAt(pos);
    index             = index.sibling(index.row(), 0);

    if (index.isValid())
    {
        QMenu menu;
        menu.addAction(i18n("Remove"), this, SLOT(slotRemoveOne()));
        menu.exec(QCursor::pos());
    }
}

void BookmarksDialog::slotOpenInMap(const QModelIndex& index)
{
    if (!index.isValid())
    {
        d->mapView->setGPSInfoList(GPSImageInfo::List());
        d->mapView->setActive(false);
        return;
    }

    QModelIndexList list = d->tree->selectionModel()->selectedIndexes();
    GPSImageInfo::List ilst;

    foreach (const QModelIndex& item, list)
    {
        QUrl url                  = item.sibling(index.row(), 1)
                                        .data(BookmarksModel::UrlRole).toUrl();
        bool ok                   = false;
        GeoCoordinates coordinate = GeoCoordinates::fromGeoUrl(url.toString(), &ok);

        if (ok)
        {
            GPSImageInfo gpsInfo;
            gpsInfo.coordinates = coordinate;
            gpsInfo.dateTime    = item.sibling(index.row(), 1)
                                      .data(BookmarksModel::DateAddedRole).toDateTime();
            gpsInfo.rating      = -1;
            gpsInfo.url         = url;
            ilst << gpsInfo;
        }
    }

    d->mapView->setGPSInfoList(GPSImageInfo::List() << ilst);
    d->mapView->setActive(!ilst.empty());
}

void BookmarksDialog::slotNewFolder()
{
    QModelIndex currentIndex = d->tree->currentIndex();
    QModelIndex idx          = currentIndex;

    if (idx.isValid() && !idx.model()->hasChildren(idx))
        idx = idx.parent();

    if (!idx.isValid())
        idx = d->tree->rootIndex();

    idx                        = d->proxyModel->mapToSource(idx);
    BookmarkNode* const parent = d->manager->bookmarksModel()->node(idx);
    BookmarkNode* const node   = new BookmarkNode(BookmarkNode::Folder);
    node->title                = i18n("New Folder");
    d->manager->addBookmark(parent, node, currentIndex.row() + 1);
}

void BookmarksDialog::slotRemoveOne()
{
    QModelIndex index = d->tree->currentIndex();

    if (index.isValid())
    {
        index                    = d->proxyModel->mapToSource(index);
        BookmarkNode* const node = d->manager->bookmarksModel()->node(index);

        if (QMessageBox::question(this, qApp->applicationName(),
                                  i18nc("@info", "Do you want to remove \"%1\" "
                                        "from your Bookmarks collection?",
                                        node->title),
                                  QMessageBox::Yes | QMessageBox::No
                                 ) == QMessageBox::No)
        {
            return;
        }

        d->manager->removeBookmark(node);
    }
}

void BookmarksDialog::readSettings()
{
    expandNodes(d->manager->bookmarks());

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(objectName());
    KConfigGroup groupGPSTab  = KConfigGroup(&group, QLatin1String("GPS Properties Tab"));
    d->mapView->readSettings(groupGPSTab);

    KConfigGroup groupDialog  = KConfigGroup(&group, "Dialog");
    winId();
    DXmlGuiWindow::restoreWindowSize(windowHandle(), groupDialog);
    resize(windowHandle()->size());
}

void BookmarksDialog::saveSettings()
{
    if (saveExpandedNodes(d->tree->rootIndex()))
        d->manager->changeExpanded();

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(objectName());
    KConfigGroup groupGPSTab  = KConfigGroup(&group, QLatin1String("GPS Properties Tab"));
    d->mapView->writeSettings(groupGPSTab);

    KConfigGroup groupDialog = KConfigGroup(&group, "Dialog");
    DXmlGuiWindow::saveWindowSize(windowHandle(), groupDialog);
}

} // namespace Digikam
