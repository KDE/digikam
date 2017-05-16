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
#include <QAction>
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

// Local includes

#include "bookmarksmngr.h"
#include "bookmarknode.h"

namespace Digikam
{

AddBookmarkDialog::AddBookmarkDialog(const QString& url,
                                     const QString& title,
                                     QWidget* const parent,
                                     BookmarksManager* const mngr)
    : QDialog(parent),
      m_url(url),
      m_bookmarksManager(mngr)
{
    setWindowFlags(Qt::Sheet);
    setWindowTitle(tr2i18n("Add Bookmark", 0));
    setObjectName(QStringLiteral("AddBookmarkDialog"));
    resize(300, 200);

    QLabel* const label = new QLabel(this);
    label->setText(tr2i18n("Type a name for the bookmark, and choose where to keep it.", 0));
    label->setObjectName(QStringLiteral("label"));
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(true);

    m_name = new QLineEdit(this);
    m_name->setObjectName(QStringLiteral("name"));
    m_name->setText(title);

    m_location = new QComboBox(this);
    m_location->setObjectName(QStringLiteral("location"));

    QSpacerItem* const verticalSpacer = new QSpacerItem(20, 2, QSizePolicy::Minimum,
                                                        QSizePolicy::Expanding);

    QDialogButtonBox* const buttonBox = new QDialogButtonBox(this);
    buttonBox->setObjectName(QStringLiteral("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    buttonBox->setCenterButtons(false);

    QVBoxLayout* const vbox = new QVBoxLayout(this);
    vbox->setObjectName(QStringLiteral("vbox"));
    vbox->addWidget(label);
    vbox->addWidget(m_name);
    vbox->addWidget(m_location);
    vbox->addItem(verticalSpacer);
    vbox->addWidget(buttonBox);

    QTreeView* const view       = new QTreeView(this);
    m_proxyModel                = new AddBookmarkProxyModel(this);
    BookmarksModel* const model = m_bookmarksManager->bookmarksModel();
    m_proxyModel->setSourceModel(model);
    view->setModel(m_proxyModel);
    view->expandAll();
    view->header()->setStretchLastSection(true);
    view->header()->hide();
    view->setItemsExpandable(false);
    view->setRootIsDecorated(false);
    view->setIndentation(10);
    view->show();

    BookmarkNode* const menu = m_bookmarksManager->menu();
    QModelIndex idx          = m_proxyModel->mapFromSource(model->index(menu));
    view->setCurrentIndex(idx);

    m_location->setModel(m_proxyModel);
    m_location->setView(view);
    m_location->setCurrentIndex(idx.row());

    connect(buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));

    connect(buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));
}

void AddBookmarkDialog::accept()
{
    QModelIndex index = m_location->view()->currentIndex();
    index             = m_proxyModel->mapToSource(index);

    if (!index.isValid())
        index = m_bookmarksManager->bookmarksModel()->index(0, 0);

    BookmarkNode* const parent   = m_bookmarksManager->bookmarksModel()->node(index);
    BookmarkNode* const bookmark = new BookmarkNode(BookmarkNode::Bookmark);
    bookmark->url                = m_url;
    bookmark->title              = m_name->text();
    m_bookmarksManager->addBookmark(parent, bookmark);
    QDialog::accept();
}

// ----------------------------------------------------------------

BookmarksDialog::BookmarksDialog(QWidget* const parent, BookmarksManager* const mngr)
    : QDialog(parent),
      m_bookmarksManager(mngr)
{
    setObjectName(QStringLiteral("BookmarksDialog"));
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(i18n("Edit Bookmarks"));
    resize(750, 450);

    m_search = new SearchTextBar(this, QLatin1String("DigikamBookmarksSearchBar"));
    m_search->setObjectName(QStringLiteral("search"));

    m_tree = new QTreeView(this);
    m_tree->setObjectName(QStringLiteral("tree"));
    m_tree->setUniformRowHeights(true);
    m_tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tree->setSelectionMode(QAbstractItemView::ContiguousSelection);
    m_tree->setTextElideMode(Qt::ElideMiddle);
    m_tree->setDragDropMode(QAbstractItemView::InternalMove);
    m_tree->setAlternatingRowColors(true);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);

    QPushButton* const removeButton    = new QPushButton(this);
    removeButton->setObjectName(QStringLiteral("removeButton"));
    removeButton->setText(i18n("&Remove"));

    QPushButton* const addFolderButton = new QPushButton(this);
    addFolderButton->setObjectName(QStringLiteral("addFolderButton"));
    addFolderButton->setText(i18n("Add Folder"));

    QSpacerItem* const spacerItem1     = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    QDialogButtonBox* const buttonBox  = new QDialogButtonBox(this);
    buttonBox->setObjectName(QStringLiteral("buttonBox"));
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);

    QHBoxLayout* const hbox = new QHBoxLayout();
    hbox->setObjectName(QStringLiteral("hbox"));
    hbox->addWidget(removeButton);
    hbox->addWidget(addFolderButton);
    hbox->addItem(spacerItem1);
    hbox->addWidget(buttonBox);

    QGridLayout* const grid = new QGridLayout(this);
    grid->setObjectName(QStringLiteral("grid"));
    grid->addWidget(m_search,   0, 0, 1, 2);
    grid->addWidget(m_tree,     1, 0, 1, 2);
    grid->addLayout(hbox,       2, 0, 1, 2);

    m_bookmarksModel = m_bookmarksManager->bookmarksModel();
    m_proxyModel     = new TreeProxyModel(this);
    m_proxyModel->setSourceModel(m_bookmarksModel);
    m_tree->setModel(m_proxyModel);
    m_tree->setExpanded(m_proxyModel->index(0, 0), true);
    QFontMetrics fm(font());
    int header       = fm.width(QLatin1Char('m')) * 40;
    m_tree->header()->resizeSection(0, header);
    m_tree->header()->setStretchLastSection(true);

    connect(buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));

    connect(m_search, SIGNAL(textChanged(QString)),
            m_proxyModel, SLOT(setFilterFixedString(QString)));

    connect(m_proxyModel, SIGNAL(signalFilterAccepts(bool)),
            m_search, SLOT(slotSearchResult(bool)));

    connect(removeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveOne()));

    connect(m_tree, SIGNAL(activated(QModelIndex)),
            this, SLOT(slotOpen()));

    connect(m_tree, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotCustomContextMenuRequested(QPoint)));

    connect(addFolderButton, SIGNAL(clicked()),
            this, SLOT(slotNewFolder()));

    expandNodes(m_bookmarksManager->bookmarks());
}

BookmarksDialog::~BookmarksDialog()
{
    if (saveExpandedNodes(m_tree->rootIndex()))
        m_bookmarksManager->changeExpanded();
}

bool BookmarksDialog::saveExpandedNodes(const QModelIndex& parent)
{
    bool changed = false;

    for (int i = 0 ; i < m_proxyModel->rowCount(parent) ; i++)
    {
        QModelIndex child             = m_proxyModel->index(i, 0, parent);
        QModelIndex sourceIndex       = m_proxyModel->mapToSource(child);
        BookmarkNode* const childNode = m_bookmarksModel->node(sourceIndex);
        bool wasExpanded              = childNode->expanded;

        if (m_tree->isExpanded(child))
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
            QModelIndex idx = m_bookmarksModel->index(childNode);
            idx             = m_proxyModel->mapFromSource(idx);
            m_tree->setExpanded(idx, true);
            expandNodes(childNode);
        }
    }
}

void BookmarksDialog::slotCustomContextMenuRequested(const QPoint& pos)
{
    QMenu menu;
    QModelIndex index = m_tree->indexAt(pos);
    index             = index.sibling(index.row(), 0);

    if (index.isValid() && !m_tree->model()->hasChildren(index))
    {
        menu.addAction(i18n("Open"), this, SLOT(slotOpen()));
        menu.addSeparator();
    }

    menu.addAction(i18n("Delete"), this, SLOT(slotRemoveOne()));
    menu.exec(QCursor::pos());
}

void BookmarksDialog::slotOpen()
{
    QModelIndex index = m_tree->currentIndex();

    if (!index.parent().isValid())
        return;

    emit signalOpenUrl(index.sibling(index.row(), 1).data(BookmarksModel::UrlRole).toUrl());
}

void BookmarksDialog::slotNewFolder()
{
    QModelIndex currentIndex = m_tree->currentIndex();
    QModelIndex idx          = currentIndex;

    if (idx.isValid() && !idx.model()->hasChildren(idx))
        idx = idx.parent();

    if (!idx.isValid())
        idx = m_tree->rootIndex();

    idx                        = m_proxyModel->mapToSource(idx);
    BookmarkNode* const parent = m_bookmarksManager->bookmarksModel()->node(idx);
    BookmarkNode* const node   = new BookmarkNode(BookmarkNode::Folder);
    node->title                = i18n("New Folder");
    m_bookmarksManager->addBookmark(parent, node, currentIndex.row() + 1);
    m_bookmarksManager->save();
}

void BookmarksDialog::slotRemoveOne()
{
    QModelIndex index = m_tree->currentIndex();

    if (index.isValid())
    {
        index                    = m_proxyModel->mapToSource(index);
        BookmarkNode* const node = m_bookmarksManager->bookmarksModel()->node(index);
        m_bookmarksManager->removeBookmark(node);
        m_bookmarksManager->save();
    }
}

} // namespace Digikam
