/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-25
 * Description : a bar used to search a string.
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#include "searchtextbar.moc"

// Qt includes

#include <QContextMenuEvent>
#include <qmenu.h>
#include <qcolor.h>
#include <qpalette.h>
#include <qstring.h>
#include <qmap.h>

// KDE includes

#include <kconfiggroup.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kdebug.h>

// Local includes

#include "albumfiltermodel.h"

namespace Digikam
{

class SearchTextBarPriv
{
public:

    SearchTextBarPriv() :
        optionAutoCompletionModeEntry("AutoCompletionMode"),
        optionCaseSensitiveEntry("CaseSensitive"),
        textQueryCompletion(false),
        hasCaseSensitive(true),
        highlightOnResult(true),
        hasResultColor(200, 255, 200),
        hasNoResultColor(255, 200, 200),
        displayRole(Qt::DisplayRole),
        uniqueIdRole(Qt::DisplayRole),
        model(0),
        filterModel(0)
    {
    }

    QString optionAutoCompletionModeEntry;
    QString optionCaseSensitiveEntry;

    bool               textQueryCompletion;
    bool               hasCaseSensitive;
    bool               highlightOnResult;

    QColor             hasResultColor;
    QColor             hasNoResultColor;

    int                displayRole;
    int                uniqueIdRole;

    QPointer<QAbstractItemModel> model;
    QPointer<AlbumFilterModel>   filterModel;

    SearchTextSettings settings;

    /**
     * This map maps model indexes to their current text representation in the
     * completion object. This is needed because if data changes in one index,
     * the old text value is not known anymore, so that it cannot be removed
     * from the completion object.
     */
    //TODO: if we want to use models that return unique strings but not integer, add support
    QMap<int, QString> idToTextMap;
};

SearchTextBar::SearchTextBar(QWidget *parent, const char* name, const QString& msg)
             : KLineEdit(parent), StateSavingObject(this),
               d(new SearchTextBarPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setClearButtonShown(true);
    setClickMessage(msg);
    setObjectName(name + QString(" Search Text Tool"));

    KCompletion *kcom = new KCompletion;
    kcom->setOrder(KCompletion::Sorted);
    kcom->setIgnoreCase(true);
    setCompletionObject(kcom, true);
    setAutoDeleteCompletionObject(true);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));

    connect(this, SIGNAL(userTextChanged(const QString&)),
            this, SLOT(slotTextChanged(const QString&)));

    loadState();

}

SearchTextBar::~SearchTextBar()
{
    saveState();
    delete d;
}

void SearchTextBar::doLoadState()
{
    KConfigGroup group = getConfigGroup();
    setCompletionMode((KGlobalSettings::Completion)group.readEntry(d->optionAutoCompletionModeEntry,
                      (int)KGlobalSettings::completionMode()));
    d->settings.caseSensitive = (Qt::CaseSensitivity)group.readEntry(d->optionCaseSensitiveEntry,
                                                                     (int)Qt::CaseInsensitive);
}

void SearchTextBar::doSaveState()
{
    KConfigGroup group = getConfigGroup();
    if (completionMode() != KGlobalSettings::completionMode())
        group.writeEntry(d->optionAutoCompletionModeEntry, (int)completionMode());
    else
        group.deleteEntry(d->optionAutoCompletionModeEntry);
    group.writeEntry(d->optionCaseSensitiveEntry, (int)d->settings.caseSensitive);
    group.sync();
}

void SearchTextBar::setTextQueryCompletion(bool b)
{
    d->textQueryCompletion = b;
}

bool SearchTextBar::hasTextQueryCompletion() const
{
    return d->textQueryCompletion;
}

void SearchTextBar::setHighlightOnResult(bool highlight)
{
    d->highlightOnResult = highlight;
}

void SearchTextBar::setModel(QPointer<QAbstractItemModel> model, int uniqueIdRole, int displayRole)
{

    kDebug() << "Got now model " << model;

    // first release old model
    if (d->model)
    {
        disconnectFromModel(d->model);
        d->idToTextMap.clear();
        completionObject()->clear();
    }

    d->model = model;
    d->displayRole = displayRole;
    d->uniqueIdRole = uniqueIdRole;

    // connect to the new model
    if (d->model)
    {
        connectToModel(d->model);

        // do an initial sync wit the new model
        sync(d->model);
    }

}

void SearchTextBar::setFilterModel(QPointer<AlbumFilterModel> filterModel)
{

    // if there already was a model, disconnect from this model
    if (d->filterModel)
    {
        disconnect(d->filterModel);
    }

    d->filterModel = filterModel;

    // connect to new model if desired
    if (d->filterModel)
    {
        connect(this, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
                d->filterModel, SLOT(setSearchTextSettings(const SearchTextSettings&)));
        connect(d->filterModel, SIGNAL(hasSearchResult(bool)),
                this, SLOT(slotSearchResult(bool)));
    }

}

void SearchTextBar::connectToModel(QAbstractItemModel *model)
{
    connect(model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(slotRowsInserted(const QModelIndex&, int, int)));
    connect(model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
            this, SLOT(slotRowsAboutToBeRemoved(const QModelIndex&, int, int)));
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

void SearchTextBar::slotRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    kDebug() << "rows of parent " << parent << " removed, start = " << start
             << ", end = " << end;
    for (int i = start; i <= end; ++i)
    {
        QModelIndex index = d->model->index(i, 0, parent);
        if (!index.isValid())
            continue;
        int id = index.data(d->uniqueIdRole).toInt();
        if (d->idToTextMap.contains(id))
        {
            QString itemName = d->idToTextMap[id];
            completionObject()->removeItem(itemName);
            d->idToTextMap.remove(id);
        }
        else
        {
            kWarning() << "idToTextMap seems to be out of sync with the model. "
                     << "There is no entry for model index " << index;
        }
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

    for (int row = topLeft.row(); row <= bottomRight.row(); ++row)
    {
        if (!d->model->hasIndex(row, topLeft.column(), topLeft.parent()))
        {
            kError() << "Got wrong change event for index with row " << row
                     << ", column " << topLeft.column()
                     << " and parent " << topLeft.parent()
                     << " in model " << d->model << ". Ignoring it.";
            continue;
        }

        QModelIndex index = d->model->index(row, topLeft.column(), topLeft.parent());
        if (!index.isValid())
            continue;

        int id = index.data(d->uniqueIdRole).toInt();
        QString itemName = index.data(d->displayRole).toString();
        if (d->idToTextMap.contains(id))
        {
            completionObject()->removeItem(d->idToTextMap.value(id));
            completionObject()->addItem(itemName);
        }
        else
        {
            kError() << "idToTextMap did not contain an entry for index "
                     << index;
        }
        d->idToTextMap[id] = itemName;
    }

}

void SearchTextBar::disconnectFromModel(QAbstractItemModel *model)
{
    disconnect(model);
}

void SearchTextBar::sync(QAbstractItemModel *model)
{

    kDebug() << "Starting sync with model " << model
             << ", rowCount for parent: " << model->rowCount();

    completionObject()->clear();
    d->idToTextMap.clear();

    for (int i = 0; i < model->rowCount(); ++i)
    {
        const QModelIndex index = model->index(i, 0);
        sync(model, index);
    }

}

void SearchTextBar::sync(QAbstractItemModel *model, const QModelIndex &index)
{

    QString itemName = index.data(d->displayRole).toString();
    kDebug() << "sync adding item '" << itemName << "' for index " << index;
    completionObject()->addItem(itemName);
    d->idToTextMap.insert(index.data(d->uniqueIdRole).toInt(), itemName);

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

    // only highlight if text is not empty or highlighting is disabled.
    if (userText().isEmpty() || !d->highlightOnResult)
    {
        setPalette(QPalette());
        return;
    }

    QPalette pal = palette();
    pal.setColor(QPalette::Active, QPalette::Base,
                 match ? d->hasResultColor :
                 d->hasNoResultColor);
    pal.setColor(QPalette::Active, QPalette::Text, Qt::black);
    setPalette(pal);

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
