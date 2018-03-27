/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-15
 * Description : menu to manage GPS bookmarks
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef BOOKMARKS_MENU_H
#define BOOKMARKS_MENU_H

// Qt includes

#include <QMenu>
#include <QList>
#include <QAction>
#include <QWidget>
#include <QModelIndex>
#include <QAbstractItemModel>

// Local includes

#include "bookmarksmngr.h"

namespace Digikam
{

/** A QMenu that is dynamically populated from a QAbstractItemModel
 */
class ModelMenu : public QMenu
{
    Q_OBJECT

public:

    explicit ModelMenu(QWidget* const parent = 0);
    ~ModelMenu();

    void setModel(QAbstractItemModel* model);
    QAbstractItemModel* model() const;

    void setMaxRows(int max);
    int maxRows() const;

    void setFirstSeparator(int offset);
    int firstSeparator() const;

    void setRootIndex(const QModelIndex& index);
    QModelIndex rootIndex() const;

    void setHoverRole(int role);
    int hoverRole() const;

    void setSeparatorRole(int role);
    int separatorRole() const;

    QAction* makeAction(const QIcon& icon, const QString& text, QObject* parent);

Q_SIGNALS:

    void activated(const QModelIndex& index);
    void hovered(const QString& text);

protected:

    // add any actions before the tree, return true if any actions are added.
    virtual bool prePopulated();

    // add any actions after the tree
    virtual void postPopulated();

    // put all of the children of parent into menu up to max
    void createMenu(const QModelIndex& parent, int max, QMenu* parentMenu = 0, QMenu* menu = 0);

private Q_SLOTS:

    void slotAboutToShow();
    void triggered(QAction* action);
    void hovered(QAction* action);

private:

    QAction* makeAction(const QModelIndex& index);

private:

    class Private;
    Private* const d;
};

// ---------------------------------------------------------------------------

/** Menu that is dynamically populated from the bookmarks
 */
class BookmarksMenu : public ModelMenu
{
    Q_OBJECT

public:

    explicit BookmarksMenu(BookmarksManager* const mngr, QWidget* const parent = 0);
    ~BookmarksMenu();

    void setInitialActions(const QList<QAction*>& actions);

Q_SIGNALS:

    void openUrl(const QUrl& url);

protected:

    bool prePopulated();

private Q_SLOTS:

    void activated(const QModelIndex& index);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // BOOKMARKS_MENU_H
