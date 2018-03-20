/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : tag filter view for the right sidebar
 *
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "tagfilterview.h"

// Qt includes

#include <QAction>

// KDE includes

#include <kselectaction.h>

// Local includes

#include "digikam_debug.h"
#include "albummodel.h"
#include "contextmenuhelper.h"

namespace Digikam
{

class TagFilterView::Private
{
public:

    Private() :
        onRestoreTagFiltersAction(0),
        offRestoreTagFiltersAction(0),
        ignoreTagAction(0),
        includeTagAction(0),
        excludeTagAction(0),
        restoreTagFiltersAction(0),
        tagFilterModeAction(0),
        tagFilterModel(0)
    {
    }

    QAction*       onRestoreTagFiltersAction;
    QAction*       offRestoreTagFiltersAction;
    QAction*       ignoreTagAction;
    QAction*       includeTagAction;
    QAction*       excludeTagAction;

    KSelectAction* restoreTagFiltersAction;
    KSelectAction* tagFilterModeAction;

    TagModel*      tagFilterModel;
};

TagFilterView::TagFilterView(QWidget* const parent, TagModel* const tagFilterModel)
    : TagCheckView(parent, tagFilterModel), d(new Private)
{
    d->tagFilterModel             = tagFilterModel;

    d->restoreTagFiltersAction    = new KSelectAction(i18n("Restore Tag Filters"), this);
    d->onRestoreTagFiltersAction  = d->restoreTagFiltersAction->addAction(i18n("On"));
    d->offRestoreTagFiltersAction = d->restoreTagFiltersAction->addAction(i18n("Off"));

    d->tagFilterModeAction        = new KSelectAction(i18n("Tag Filter Mode"), this);
    d->ignoreTagAction            = d->tagFilterModeAction->addAction(i18n("Ignore This Tag"));
    d->includeTagAction           = d->tagFilterModeAction->addAction(QIcon::fromTheme(QLatin1String("list-add")),    i18n("Must Have This Tag"));
    d->excludeTagAction           = d->tagFilterModeAction->addAction(QIcon::fromTheme(QLatin1String("list-remove")), i18n("Must Not Have This Tag"));
}

TagFilterView::~TagFilterView()
{
    delete d;
}

void TagFilterView::addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album)
{
    TagCheckView::addCustomContextMenuActions(cmh, album);

    // restoring
    cmh.addAction(d->restoreTagFiltersAction);

    Qt::CheckState state = d->tagFilterModel->checkState(album);

    switch (state)
    {
        case Qt::Unchecked:
            d->tagFilterModeAction->setCurrentAction(d->ignoreTagAction);
            break;
        case Qt::PartiallyChecked:
            d->tagFilterModeAction->setCurrentAction(d->excludeTagAction);
            break;
        case Qt::Checked:
            d->tagFilterModeAction->setCurrentAction(d->includeTagAction);
            break;
    }

    cmh.addAction(d->tagFilterModeAction);

    d->onRestoreTagFiltersAction->setChecked(isRestoreCheckState());
    d->offRestoreTagFiltersAction->setChecked(!isRestoreCheckState());
}

void TagFilterView::handleCustomContextMenuAction(QAction* action, AlbumPointer<Album> album)
{
    TagCheckView::handleCustomContextMenuAction(action, album);

    if (!action)
    {
        return;
    }

    if (action == d->onRestoreTagFiltersAction)        // Restore TagFilters ON.
    {
        setRestoreCheckState(true);
    }
    else if (action == d->offRestoreTagFiltersAction)  // Restore TagFilters OFF.
    {
        setRestoreCheckState(false);
    }
    else if (action == d->ignoreTagAction)
    {
        albumModel()->setCheckState(album, Qt::Unchecked);
    }
    else if (action == d->includeTagAction)
    {
        albumModel()->setCheckState(album, Qt::Checked);
    }
    else if (action == d->excludeTagAction)
    {
        albumModel()->setCheckState(album, Qt::PartiallyChecked);
    }
}

} // namespace Digikam
