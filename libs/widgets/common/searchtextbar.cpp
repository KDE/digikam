/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-25
 * Description : a bar used to search a string.
 *
 * Copyright (C) 2007-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
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
#include <QMenu>
#include <QColor>
#include <QPalette>
#include <QString>
#include <QMap>

// KDE includes

#include <kconfiggroup.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kdebug.h>

// Local includes

#include "albumfiltermodel.h"
#include "modelcompletion.h"

namespace Digikam
{

bool operator==(const SearchTextSettings& a, const SearchTextSettings& b)
{
    return a.caseSensitive == b.caseSensitive && a.text == b.text;
}

class SearchTextBar::Private
{
public:

    Private() :
        optionAutoCompletionModeEntry("AutoCompletionMode"),
        optionCaseSensitiveEntry("CaseSensitive"),
        textQueryCompletion(false),
        hasCaseSensitive(true),
        highlightOnResult(true),
        hasResultColor(200, 255, 200),
        hasNoResultColor(255, 200, 200),
        completion(0),
        filterModel(0)
    {
    }

    QString                    optionAutoCompletionModeEntry;
    QString                    optionCaseSensitiveEntry;

    bool                       textQueryCompletion;
    bool                       hasCaseSensitive;
    bool                       highlightOnResult;

    QColor                     hasResultColor;
    QColor                     hasNoResultColor;

    ModelCompletion*           completion;

    QPointer<AlbumFilterModel> filterModel;

    SearchTextSettings         settings;
};

SearchTextBar::SearchTextBar(QWidget* const parent, const char* const name, const QString& msg)
    : KLineEdit(parent), StateSavingObject(this),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setClearButtonShown(true);
    setClickMessage(msg);
    setObjectName(name + QString(" Search Text Tool"));

    d->completion = new ModelCompletion;
    setCompletionObject(d->completion, true);
    setAutoDeleteCompletionObject(true);

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));

    connect(this, SIGNAL(userTextChanged(QString)),
            this, SLOT(slotTextChanged(QString)));

    loadState();
}

SearchTextBar::~SearchTextBar()
{
    saveState();
    delete d;
}

void SearchTextBar::doLoadState()
{
    KConfigGroup group        = getConfigGroup();
    setCompletionMode((KGlobalSettings::Completion)group.readEntry(entryName(d->optionAutoCompletionModeEntry),
                      (int)KGlobalSettings::completionMode()));
    d->settings.caseSensitive = (Qt::CaseSensitivity)group.readEntry(entryName(d->optionCaseSensitiveEntry),
                                (int)Qt::CaseInsensitive);
    setIgnoreCase(d->settings.caseSensitive == Qt::CaseInsensitive);
}

void SearchTextBar::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    if (completionMode() != KGlobalSettings::completionMode())
    {
        group.writeEntry(entryName(d->optionAutoCompletionModeEntry), (int)completionMode());
    }
    else
    {
        group.deleteEntry(entryName(d->optionAutoCompletionModeEntry));
    }

    group.writeEntry(entryName(d->optionCaseSensitiveEntry), (int)d->settings.caseSensitive);
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

    if (!highlight)
    {
        setPalette(QPalette());
    }
}

void SearchTextBar::setModel(QAbstractItemModel* model, int uniqueIdRole, int displayRole)
{
    d->completion->setModel(model, uniqueIdRole, displayRole);
}

void SearchTextBar::setModel(AbstractAlbumModel* model)
{
    d->completion->setModel(model, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
}

void SearchTextBar::setFilterModel(AlbumFilterModel* filterModel)
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
        connect(this, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
                d->filterModel, SLOT(setSearchTextSettings(SearchTextSettings)));

        connect(d->filterModel, SIGNAL(hasSearchResult(bool)),
                this, SLOT(slotSearchResult(bool)));
    }
}

SearchTextBar::HighlightState SearchTextBar::getCurrentHighlightState() const
{
    if (palette() == QPalette())
    {
        return NEUTRAL;
    }
    else if (palette().color(QPalette::Active, QPalette::Base) == d->hasResultColor)
    {
        return HAS_RESULT;
    }
    else if (palette().color(QPalette::Active, QPalette::Base) == d->hasNoResultColor)
    {
        return NO_RESULT;
    }

    kError() << "Impossible highlighting state";

    return NEUTRAL;
}

void SearchTextBar::setCaseSensitive(bool b)
{
    d->hasCaseSensitive = b;

    // reset settings if selecting case sensitivity is not allowed
    if (!b)
    {
        d->settings.caseSensitive = Qt::CaseInsensitive;
    }

    // re-emit signal with changed settings
    if (!text().isEmpty())
    {
        emit signalSearchTextSettings(d->settings);
    }
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
    {
        setPalette(QPalette());
    }

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
    QAction* cs       = 0;
    QMenu* const menu = createStandardContextMenu();

    if (d->hasCaseSensitive)
    {
        cs = menu->addAction(i18n("Case sensitive"));
        cs->setCheckable(true);
        cs->setChecked(d->settings.caseSensitive == Qt::CaseInsensitive ? false : true);
    }

    menu->exec(e->globalPos());

    if (d->hasCaseSensitive)
    {
        setIgnoreCase(!cs->isChecked());
    }

    delete menu;
}

void SearchTextBar::setIgnoreCase(bool ignore)
{
    if (hasCaseSensitive())
    {

        if (ignore)
        {
            d->settings.caseSensitive = Qt::CaseInsensitive;
        }
        else
        {
            d->settings.caseSensitive = Qt::CaseSensitive;
        }

        completionObject()->setIgnoreCase(ignore);

    }
    else
    {
        d->settings.caseSensitive = Qt::CaseInsensitive;
        completionObject()->setIgnoreCase(true);
    }

    emit signalSearchTextSettings(d->settings);
}

}  // namespace Digikam
