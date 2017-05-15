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
#include <QBuffer>
#include <QFile>
#include <QMimeData>
#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QIcon>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QToolButton>
#include <QDebug>

// Local includes

#include "bookmarksmngr.h"
#include "bookmarknode.h"

namespace Digikam
{

AddBookmarkDialog::AddBookmarkDialog(const QString &url,
                                     const QString &title,
                                     QWidget *parent,
                                     BookmarksManager *bookmarkManager)
    : QDialog(parent),
      m_url(url),
      m_bookmarksManager(bookmarkManager)
{
    setWindowFlags(Qt::Sheet);
    setWindowTitle(tr2i18n("Add Bookmark", 0));

    setObjectName(QStringLiteral("AddBookmarkDialog"));
    resize(300, 200);

    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));

    label = new QLabel(this);
    label->setText(tr2i18n("Type a name for the bookmark, and choose where to keep it.", 0));
    label->setObjectName(QStringLiteral("label"));
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(true);

    verticalLayout->addWidget(label);

    name = new QLineEdit(this);
    name->setObjectName(QStringLiteral("name"));

    verticalLayout->addWidget(name);

    location = new QComboBox(this);
    location->setObjectName(QStringLiteral("location"));

    verticalLayout->addWidget(location);

    verticalSpacer = new QSpacerItem(20, 2, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout->addItem(verticalSpacer);

    buttonBox = new QDialogButtonBox(this);
    buttonBox->setObjectName(QStringLiteral("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    buttonBox->setCenterButtons(false);

    verticalLayout->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));

    connect(buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));

    QTreeView *view = new QTreeView(this);
    m_proxyModel = new AddBookmarkProxyModel(this);
    BookmarksModel *model = m_bookmarksManager->bookmarksModel();
    m_proxyModel->setSourceModel(model);
    view->setModel(m_proxyModel);
    view->expandAll();
    view->header()->setStretchLastSection(true);
    view->header()->hide();
    view->setItemsExpandable(false);
    view->setRootIsDecorated(false);
    view->setIndentation(10);
    location->setModel(m_proxyModel);
    view->show();
    location->setView(view);
    BookmarkNode *menu = m_bookmarksManager->menu();
    QModelIndex idx = m_proxyModel->mapFromSource(model->index(menu));
    view->setCurrentIndex(idx);
    location->setCurrentIndex(idx.row());
    name->setText(title);
}

void AddBookmarkDialog::accept()
{
    QModelIndex index = location->view()->currentIndex();
    index = m_proxyModel->mapToSource(index);

    if (!index.isValid())
        index = m_bookmarksManager->bookmarksModel()->index(0, 0);

    BookmarkNode *parent = m_bookmarksManager->bookmarksModel()->node(index);
    BookmarkNode *bookmark = new BookmarkNode(BookmarkNode::Bookmark);
    bookmark->url = m_url;
    bookmark->title = name->text();
    m_bookmarksManager->addBookmark(parent, bookmark);
    QDialog::accept();
}

// ----------------------------------------------------------------

BookmarksDialog::BookmarksDialog(QWidget *parent, BookmarksManager *manager)
    : QDialog(parent)
{
    m_bookmarksManager = manager;

    setObjectName(QStringLiteral("BookmarksDialog"));
    resize(750, 450);

    gridLayout = new QGridLayout(this);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    spacerItem = new QSpacerItem(252, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout->addItem(spacerItem, 0, 0, 1, 1);

    search = new SearchTextBar(this, QLatin1String("DigikamGeoLocationBookmarksSearchBar"));
    search->setObjectName(QStringLiteral("search"));

    gridLayout->addWidget(search, 0, 1, 1, 1);

    tree = new QTreeView(this);
    tree->setObjectName(QStringLiteral("tree"));

    gridLayout->addWidget(tree, 1, 0, 1, 2);

    hboxLayout = new QHBoxLayout();
    hboxLayout->setObjectName(QStringLiteral("hboxLayout"));
    removeButton = new QPushButton(this);
    removeButton->setObjectName(QStringLiteral("removeButton"));

    hboxLayout->addWidget(removeButton);

    addFolderButton = new QPushButton(this);
    addFolderButton->setObjectName(QStringLiteral("addFolderButton"));

    hboxLayout->addWidget(addFolderButton);

    spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem1);

    buttonBox = new QDialogButtonBox(this);
    buttonBox->setObjectName(QStringLiteral("buttonBox"));
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);

    hboxLayout->addWidget(buttonBox);

    gridLayout->addLayout(hboxLayout, 2, 0, 1, 2);

    setWindowTitle(tr2i18n("Bookmarks", 0));
    removeButton->setText(tr2i18n("&Remove", 0));
    addFolderButton->setText(tr2i18n("Add Folder", 0));

    QObject::connect(buttonBox, SIGNAL(accepted()),
                     this, SLOT(accept()));

    tree->setUniformRowHeights(true);
    tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    tree->setSelectionMode(QAbstractItemView::ContiguousSelection);
    tree->setTextElideMode(Qt::ElideMiddle);
    m_bookmarksModel = m_bookmarksManager->bookmarksModel();
    m_proxyModel = new TreeProxyModel(this);

    connect(search, SIGNAL(textChanged(QString)),
            m_proxyModel, SLOT(setFilterFixedString(QString)));

    connect(removeButton, SIGNAL(clicked()),
            this, SLOT(removeOne()));

    m_proxyModel->setSourceModel(m_bookmarksModel);
    tree->setModel(m_proxyModel);
    tree->setDragDropMode(QAbstractItemView::InternalMove);
    tree->setExpanded(m_proxyModel->index(0, 0), true);
    tree->setAlternatingRowColors(true);
    QFontMetrics fm(font());
    int header = fm.width(QLatin1Char('m')) * 40;
    tree->header()->resizeSection(0, header);
    tree->header()->setStretchLastSection(true);

    connect(tree, SIGNAL(activated(QModelIndex)),
            this, SLOT(open()));

    tree->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(tree, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(customContextMenuRequested(QPoint)));

    connect(addFolderButton, SIGNAL(clicked()),
            this, SLOT(newFolder()));

    expandNodes(m_bookmarksManager->bookmarks());
    setAttribute(Qt::WA_DeleteOnClose);
}

BookmarksDialog::~BookmarksDialog()
{
    if (saveExpandedNodes(tree->rootIndex()))
        m_bookmarksManager->changeExpanded();
}

bool BookmarksDialog::saveExpandedNodes(const QModelIndex &parent)
{
    bool changed = false;

    for (int i = 0; i < m_proxyModel->rowCount(parent); ++i)
    {
        QModelIndex child             = m_proxyModel->index(i, 0, parent);
        QModelIndex sourceIndex       = m_proxyModel->mapToSource(child);
        BookmarkNode* const childNode = m_bookmarksModel->node(sourceIndex);
        bool wasExpanded              = childNode->expanded;

        if (tree->isExpanded(child))
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

void BookmarksDialog::expandNodes(BookmarkNode *node)
{
    for (int i = 0; i < node->children().count(); ++i)
    {
        BookmarkNode* childNode = node->children()[i];

        if (childNode->expanded)
        {
            QModelIndex idx = m_bookmarksModel->index(childNode);
            idx             = m_proxyModel->mapFromSource(idx);
            tree->setExpanded(idx, true);
            expandNodes(childNode);
        }
    }
}

void BookmarksDialog::customContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    QModelIndex index = tree->indexAt(pos);
    index             = index.sibling(index.row(), 0);

    if (index.isValid() && !tree->model()->hasChildren(index))
    {
        menu.addAction(tr("Open"), this, SLOT(open()));
        menu.addSeparator();
    }

    menu.addAction(tr("Delete"), this, SLOT(removeOne()));
    menu.exec(QCursor::pos());
}

void BookmarksDialog::open()
{
    QModelIndex index = tree->currentIndex();

    if (!index.parent().isValid())
        return;

    emit openUrl(index.sibling(index.row(), 1).data(BookmarksModel::UrlRole).toUrl());
}

void BookmarksDialog::newFolder()
{
    QModelIndex currentIndex = tree->currentIndex();
    QModelIndex idx          = currentIndex;

    if (idx.isValid() && !idx.model()->hasChildren(idx))
        idx = idx.parent();

    if (!idx.isValid())
        idx = tree->rootIndex();

    idx                  = m_proxyModel->mapToSource(idx);
    BookmarkNode* parent = m_bookmarksManager->bookmarksModel()->node(idx);
    BookmarkNode* node   = new BookmarkNode(BookmarkNode::Folder);
    node->title          = tr("New Folder");
    m_bookmarksManager->addBookmark(parent, node, currentIndex.row() + 1);
}

void BookmarksDialog::removeOne()
{
    QModelIndex index = tree->currentIndex();

    if (index.isValid())
    {
        index              = m_proxyModel->mapToSource(index);
        BookmarkNode* node = m_bookmarksManager->bookmarksModel()->node(index);
        m_bookmarksManager->removeBookmark(node);
    }
}

} // namespace Digikam
