/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-15
 * Description : menu to manage GPS bookmarks
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

#include "bookmarksmenu.h"

// Qt includes

#include <QAbstractItemModel>
#include <QUrl>
#include <QDebug>

Q_DECLARE_METATYPE(QModelIndex)

namespace Digikam
{

class ModelMenu::Private
{
public:

    Private() :
        maxRows(7),
        firstSeparator(-1),
        maxWidth(-1),
        hoverRole(0),
        separatorRole(0),
        model(0)
    {
    }

    int                   maxRows;
    int                   firstSeparator;
    int                   maxWidth;
    int                   hoverRole;
    int                   separatorRole;
    QAbstractItemModel*   model;
    QPersistentModelIndex root;
};

ModelMenu::ModelMenu(QWidget* const parent)
    : QMenu(parent),
      d(new Private)
{
    connect(this, SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShow()));
}

ModelMenu::~ModelMenu()
{
    delete d;
}

bool ModelMenu::prePopulated()
{
    return false;
}

void ModelMenu::postPopulated()
{
}

void ModelMenu::setModel(QAbstractItemModel* model)
{
    d->model = model;
}

QAbstractItemModel* ModelMenu::model() const
{
    return d->model;
}

void ModelMenu::setMaxRows(int max)
{
    d->maxRows = max;
}

int ModelMenu::maxRows() const
{
    return d->maxRows;
}

void ModelMenu::setFirstSeparator(int offset)
{
    d->firstSeparator = offset;
}

int ModelMenu::firstSeparator() const
{
    return d->firstSeparator;
}

void ModelMenu::setRootIndex(const QModelIndex& index)
{
    d->root = index;
}

QModelIndex ModelMenu::rootIndex() const
{
    return d->root;
}

void ModelMenu::setHoverRole(int role)
{
    d->hoverRole = role;
}

int ModelMenu::hoverRole() const
{
    return d->hoverRole;
}

void ModelMenu::setSeparatorRole(int role)
{
    d->separatorRole = role;
}

int ModelMenu::separatorRole() const
{
    return d->separatorRole;
}

void ModelMenu::slotAboutToShow()
{
    if (QMenu* const menu = qobject_cast<QMenu*>(sender()))
    {
        QVariant v = menu->menuAction()->data();

        if (v.canConvert<QModelIndex>())
        {
            QModelIndex idx = qvariant_cast<QModelIndex>(v);
            createMenu(idx, -1, menu, menu);

            disconnect(menu, SIGNAL(aboutToShow()),
                       this, SLOT(slotAboutToShow()));

            return;
        }
    }

    clear();

    if (prePopulated())
        addSeparator();

    int max = d->maxRows;

    if (max != -1)
        max += d->firstSeparator;

    createMenu(d->root, max, this, this);
    postPopulated();
}

void ModelMenu::createMenu(const QModelIndex& parent, int max, QMenu* parentMenu, QMenu* menu)
{
    if (!menu)
    {
        QString title = parent.data().toString();
        menu          = new QMenu(title, this);
        QIcon icon    = qvariant_cast<QIcon>(parent.data(Qt::DecorationRole));
        menu->setIcon(icon);
        parentMenu->addMenu(menu);
        QVariant v;
        v.setValue(parent);
        menu->menuAction()->setData(v);

        connect(menu, SIGNAL(aboutToShow()),
                this, SLOT(slotAboutToShow()));

        return;
    }

    int end = d->model->rowCount(parent);

    if (max != -1)
        end = qMin(max, end);

    connect(menu, SIGNAL(triggered(QAction*)),
            this, SLOT(triggered(QAction*)));

    connect(menu, SIGNAL(hovered(QAction*)),
            this, SLOT(hovered(QAction*)));

    for (int i = 0 ; i < end ; ++i)
    {
        QModelIndex idx = d->model->index(i, 0, parent);

        if (d->model->hasChildren(idx))
        {
            createMenu(idx, -1, menu);
        }
        else
        {
            if (d->separatorRole != 0 && idx.data(d->separatorRole).toBool())
                addSeparator();
            else
                menu->addAction(makeAction(idx));
        }

        if (menu == this && i == d->firstSeparator - 1)
            addSeparator();
    }
}

QAction* ModelMenu::makeAction(const QModelIndex& index)
{
    QIcon icon            = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    QAction* const action = makeAction(icon, index.data().toString(), this);
    QVariant v;
    v.setValue(index);
    action->setData(v);

    return action;
}

QAction* ModelMenu::makeAction(const QIcon& icon, const QString& text, QObject* parent)
{
    QFontMetrics fm(font());

    if (d->maxWidth == -1)
        d->maxWidth = fm.width(QLatin1Char('m')) * 30;

    QString smallText = fm.elidedText(text, Qt::ElideMiddle, d->maxWidth);

    return (new QAction(icon, smallText, parent));
}

void ModelMenu::triggered(QAction* action)
{
    QVariant v = action->data();

    if (v.canConvert<QModelIndex>())
    {
        QModelIndex idx = qvariant_cast<QModelIndex>(v);
        emit activated(idx);
    }
}

void ModelMenu::hovered(QAction* action)
{
    QVariant v = action->data();

    if (v.canConvert<QModelIndex>())
    {
        QModelIndex idx       = qvariant_cast<QModelIndex>(v);
        QString hoveredString = idx.data(d->hoverRole).toString();

        if (!hoveredString.isEmpty())
            emit hovered(hoveredString);
    }
}

// ------------------------------------------------------------------------------

class BookmarksMenu::Private
{
public:

    Private() :
        manager(0)
    {
    }

    BookmarksManager* manager;
    QList<QAction*>   initActions;
};

BookmarksMenu::BookmarksMenu(BookmarksManager* const mngr, QWidget* const parent)
    : ModelMenu(parent),
      d(new Private)
{
    d->manager = mngr;

    connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(activated(QModelIndex)));

    setMaxRows(-1);
    setHoverRole(BookmarksModel::UrlStringRole);
    setSeparatorRole(BookmarksModel::SeparatorRole);
}

BookmarksMenu::~BookmarksMenu()
{
    delete d;
}

void BookmarksMenu::activated(const QModelIndex& index)
{
    emit openUrl(index.data(BookmarksModel::UrlRole).toUrl());
}

bool BookmarksMenu::prePopulated()
{
    setModel(d->manager->bookmarksModel());
    setRootIndex(d->manager->bookmarksModel()->index(d->manager->bookmarks()));

    // initial actions

    foreach (QAction* const ac, d->initActions)
    {
        if (ac)
            addAction(ac);
    }

    if (!d->initActions.isEmpty())
        addSeparator();

    createMenu(rootIndex(), 0, this, this);

    return true;
}

void BookmarksMenu::setInitialActions(const QList<QAction*>& actions)
{
    d->initActions = actions;

    foreach (QAction* const ac, d->initActions)
    {
        if (ac)
            addAction(ac);
    }
}

} // namespace Digikam
