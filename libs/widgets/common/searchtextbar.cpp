/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-25
 * Description : a bar used to search a string.
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "searchtextbar.h"
#include "searchtextbar.moc"

// Qt includes

#include <QContextMenuEvent>
#include <QMenu>
#include <QColor>
#include <QPalette>
#include <QString>

// KDE includes

#include <kconfiggroup.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kdebug.h>

namespace Digikam
{

class SearchTextBarPriv
{
public:

    SearchTextBarPriv()
    {
        textQueryCompletion = false;
        hasCaseSensitive    = true;
        model               = 0;
    }

    bool               textQueryCompletion;
    bool               hasCaseSensitive;

    QAbstractItemModel *model;

    SearchTextSettings settings;
};

SearchTextBar::SearchTextBar(QWidget *parent, const char* name, const QString& msg)
             : KLineEdit(parent), d(new SearchTextBarPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setClearButtonShown(true);
    setClickMessage(msg);
    setObjectName(name);

    KCompletion *kcom = new KCompletion;
    kcom->setOrder(KCompletion::Sorted);
    kcom->setIgnoreCase(true);
    setCompletionObject(kcom, true);
    setAutoDeleteCompletionObject(true);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));

    connect(this, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTextChanged(const QString&)));

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(name + QString(" Search Text Tool"));
    setCompletionMode((KGlobalSettings::Completion)group.readEntry("AutoCompletionMode",
                      (int)KGlobalSettings::CompletionAuto));
    d->settings.caseSensitive = (Qt::CaseSensitivity)group.readEntry("CaseSensitive", 
                                                                     (int)Qt::CaseInsensitive);
}

SearchTextBar::~SearchTextBar()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(objectName() + QString(" Search Text Tool"));
    group.writeEntry("AutoCompletionMode", (int)completionMode());
    group.writeEntry("CaseSensitive",      (int)d->settings.caseSensitive);
    group.sync();

    delete d;
}

void SearchTextBar::setTextQueryCompletion(bool b)
{
    d->textQueryCompletion = b;
}

bool SearchTextBar::hasTextQueryCompletion() const
{
    return d->textQueryCompletion;
}

void SearchTextBar::setModel(QAbstractItemModel *model)
{

    kDebug() << "Got now model " << model;

    // first release old model
    if (d->model)
    {
        disconnectFromModel(d->model);
        completionObject()->clear();
    }

    d->model = model;

    // connect to the new model
    if (model)
    {
        connectToModel(d->model);

        // do an initial sync wit the new model
        sync(d->model);
    }

}

void SearchTextBar::connectToModel(QAbstractItemModel *model)
{
    connect(model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(slotRowsInserted(const QModelIndex&, int, int)));
    connect(model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
            this, SLOT(slotRowsRemoved(const QModelIndex&, int, int)));
    connect(model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex &)),
            this, SLOT(slotDataChanged(const QModelIndex&, const QModelIndex &)));
    connect(model, SIGNAL(modelReset()),
            this, SLOT(slotModelReset()));

}

void SearchTextBar::slotRowsInserted(const QModelIndex &parent, int start, int end)
{
    kDebug() << "rowInserted in parent " << parent << ", start = " << start
             << ", end = " << end;

    for (int i = start; i <= end; ++i)
    {
        const QModelIndex child = d->model->index(i, 0, parent);
        sync(d->model, child);
    }
}

void SearchTextBar::slotRowsRemoved(const QModelIndex &parent, int start, int end)
{
    kDebug() << "rows of parent " << parent << " removed, start = " << start
             << ", end = " << end;
    for (int i = start; i <= end; ++i)
    {
        // TODO this doesn't work, itemText contains no, or wrong album names
        // maybe that is caused by the model because the album is already
        // deleted when I want to access the name?
        QString itemText = d->model->index(i, 0, parent).data().value<QString> ();
        kDebug() << "Removing item " << itemText;
        completionObject()->removeItem(itemText);
    }
}

void SearchTextBar::slotModelReset()
{
    kDebug() << "model reset, resync";
    sync(d->model);
}

void SearchTextBar::slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);

    kDebug() << "data changed, resync";

    // TODO what if data changes? arguments don't help here because
    // it does not provide the old text value. Complete resync?
    // FIXME this is realllly a performance issue
    sync(d->model);

}

void SearchTextBar::disconnectFromModel(QAbstractItemModel *model)
{
    disconnect(model);
}

void SearchTextBar::sync(QAbstractItemModel *model)
{

    kDebug() << "Starting sync with model " << model
             << ", rowCount for parent: " << model->rowCount();

    clear();

    for (int i = 0; i < model->rowCount(); ++i)
    {
        const QModelIndex index = model->index(i, 0);
        sync(model, index);
    }

}

void SearchTextBar::sync(QAbstractItemModel *model, const QModelIndex &index)
{

    QString itemName = index.data(Qt::DisplayRole).value<QString> ();
    kDebug() << "sync adding item '" << itemName << "'";
    completionObject()->addItem(itemName);

    for (int i = 0; i < model->rowCount(index); ++i)
    {
        const QModelIndex child = model->index(i, 0, index);
        sync(model, child);
    }

}

void SearchTextBar::setCaseSensitive(bool b)
{
    d->hasCaseSensitive = b;
}

bool SearchTextBar::hasCaseSensitive() const
{
    return d->hasCaseSensitive;
}

void SearchTextBar::setSearchTextSettings(const SearchTextSettings& settings)
{
    d->settings = settings;
}

SearchTextSettings SearchTextBar::searchTextSettings() const
{
    return d->settings;
}

void SearchTextBar::slotTextChanged(const QString& text)
{
    if (text.isEmpty())
        setPalette(QPalette());

    d->settings.text = text;

    emit signalSearchTextSettings(d->settings);
}

void SearchTextBar::slotSearchResult(bool match)
{

    kDebug() << "match = " << match;

    if (text().isEmpty())
    {
        setPalette(QPalette());
        return;
    }

    QPalette pal = palette();
    pal.setColor(QPalette::Active, QPalette::Base,
                 match ? QColor(200, 255, 200) :
                 QColor(255, 200, 200));
    pal.setColor(QPalette::Active, QPalette::Text, Qt::black);
    setPalette(pal);

    // If search result match the text query, we put the text
    // in auto-completion history.
    if (d->textQueryCompletion && match)
        completionObject()->addItem(text());
}

void SearchTextBar::contextMenuEvent(QContextMenuEvent* e)
{
    QAction *cs = 0;
    QMenu *menu = createStandardContextMenu();

    if (d->hasCaseSensitive)
    {
        cs = menu->addAction(tr("Case sensitive"));
        cs->setCheckable(true);
        cs->setChecked(d->settings.caseSensitive == Qt::CaseInsensitive ? false : true);
    }

    menu->exec(e->globalPos());

    if (d->hasCaseSensitive)
        d->settings.caseSensitive = cs->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;

    delete menu;
}

}  // namespace Digikam
