/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-23
 * Description : a tab to display image editing history
 *
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "imagepropertieshistorytab.h"

// Qt includes

#include <QGridLayout>
#include <QTreeView>
#include <QMenu>
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "imagefiltershistorymodel.h"
#include "imagefiltershistorytreeitem.h"
#include "imagefiltershistoryitemdelegate.h"

namespace Digikam
{


RemoveFilterAction::RemoveFilterAction(const QString& label, const QModelIndex& index, QObject* const parent)
    : QAction(label, parent)
{
    m_index = index;
}

// -------------------------------------------------------------------------------------------------------

class ImagePropertiesHistoryTab::Private
{
public:

    Private()
    {
        view        = 0;
        model       = 0;
        layout      = 0;
        delegate    = 0;
        headerLabel = 0;
    }

    QTreeView*                       view;
    ImageFiltersHistoryModel*        model;
    QGridLayout*                     layout;
    ImageFiltersHistoryItemDelegate* delegate;
    QLabel*                          headerLabel;
};

ImagePropertiesHistoryTab::ImagePropertiesHistoryTab(QWidget* const parent)
    : QWidget(parent), d(new Private)
{
    d->layout      = new QGridLayout(this);
    d->view        = new QTreeView(this);
    d->delegate    = new ImageFiltersHistoryItemDelegate(this);
    d->model       = new ImageFiltersHistoryModel(0, QUrl());
    d->headerLabel = new QLabel(this);

    d->headerLabel->setText(i18n("Used filters"));

    d->layout->addWidget(d->headerLabel);
    d->layout->addWidget(d->view);

    d->view->setItemDelegate(d->delegate);
    d->view->setRootIsDecorated(false);
    d->view->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(d->view, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showCustomContextMenu(QPoint)));
}

ImagePropertiesHistoryTab::~ImagePropertiesHistoryTab()
{
    delete d->model;
    delete d->view;
    delete d->delegate;
    delete d;
}

void ImagePropertiesHistoryTab::setCurrentURL(const QUrl& url)
{
    d->model->setUrl(url);
    d->view->setModel(d->model);
    d->view->update();
}

void ImagePropertiesHistoryTab::showCustomContextMenu(const QPoint& position)
{
    QList<QAction*> actions;

    if (d->view->indexAt(position).isValid())
    {
        QModelIndex index = d->view->indexAt(position);

        QString s(i18n("Remove filter"));
        RemoveFilterAction* removeFilterAction = new RemoveFilterAction(s, index, 0);
        removeFilterAction->setDisabled(true);

        if (!index.model()->parent(index).isValid())
        {
            actions.append(removeFilterAction);

            connect(removeFilterAction, SIGNAL(triggered()),
                    removeFilterAction, SLOT(triggerSlot()));

            connect(removeFilterAction, SIGNAL(actionTriggered(QModelIndex)),
                    d->model, SLOT(removeEntry(QModelIndex)));
        }
    }

    if (actions.count() > 0)
    {
        QMenu::exec(actions, d->view->mapToGlobal(position));
    }
}

void ImagePropertiesHistoryTab::setModelData(const QList<DImageHistory::Entry>& entries)
{
    d->model->setupModelData(entries);
}

void ImagePropertiesHistoryTab::disableEntry(bool disable)
{
    d->model->disableEntry(d->model->index(d->model->rowCount(),0), disable);
}

} // namespace Digikam

