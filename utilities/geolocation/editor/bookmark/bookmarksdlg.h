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

#ifndef BOOKMARKS_DLG_H
#define BOOKMARKS_DLG_H

// Qt includes

#include <QObject>
#include <QAbstractItemModel>
#include <QUndoCommand>
#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QSpacerItem>
#include <QTreeView>
#include <QLabel>
#include <QComboBox>

// Local includes

#include "searchtextbar.h"
#include "bookmarksmngr.h"

namespace Digikam
{

class AddBookmarkDialog : public QDialog
{
    Q_OBJECT

public:

    explicit AddBookmarkDialog(const QString& url,
                               const QString& title,
                               QWidget* const parent = 0,
                               BookmarksManager* const mngr = 0);

private Q_SLOTS:

    void accept();

private:

    QString                m_url;
    BookmarksManager*      m_bookmarksManager;
    AddBookmarkProxyModel* m_proxyModel;

    QVBoxLayout*           verticalLayout;
    QLabel*                label;
    QLineEdit*             name;
    QComboBox*             location;
    QSpacerItem*           verticalSpacer;
    QDialogButtonBox*      buttonBox;
};

// --------------------------------------------------------------------

class BookmarksDialog : public QDialog
{
    Q_OBJECT

public:

    explicit BookmarksDialog(QWidget* const parent = 0, BookmarksManager* const mngr = 0);
    ~BookmarksDialog();

Q_SIGNALS:

    void signalOpenUrl(const QUrl&);

private Q_SLOTS:

    void slotCustomContextMenuRequested(const QPoint&);
    void slotOpen();
    void slotNewFolder();
    void slotRemoveOne();

private:

    void expandNodes(BookmarkNode* const node);
    bool saveExpandedNodes(const QModelIndex& parent);

private:

    BookmarksManager* m_bookmarksManager;
    BookmarksModel*   m_bookmarksModel;
    TreeProxyModel*   m_proxyModel;
    SearchTextBar*    m_search;
    QTreeView*        m_tree;
};

} // namespace Digikam

#endif // BOOKMARKS_DLG_H
