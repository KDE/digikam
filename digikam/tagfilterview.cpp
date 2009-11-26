/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-05
 * Description : tags filter view
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "tagfilterview.moc"

// Qt includes
#include <qaction.h>

// KDE includes
#include <kdebug.h>
#include <kmenu.h>
#include <kselectaction.h>

// Local includes
#include "contextmenuhelper.h"
#include "tagfilterview.h"

namespace Digikam
{

class TagFilterViewNewPriv
{
public:
    TagFilterViewNewPriv() :
        matchingCond(ImageFilterSettings::OrCondition)
    {
    }

    // TODO update, implement this
    TagFilterViewNew::RestoreTagFilters    restoreTagFilters;
    TagFilterViewNew::ToggleAutoTags       toggleAutoTags;

    ImageFilterSettings::MatchingCondition matchingCond;

    KMenu *selectTagsMenu;
    QAction *selectAllTagsAction;
    QAction *selectChildrenAction;
    QAction *selectParentsAction;
    KMenu *deselectTagsMenu;
    QAction *deselectAllTagsAction;
    QAction *deselectChildrenAction;
    QAction *deselectParentsAction;
    QAction *invertAction;
    KSelectAction *toggleAutoAction;
    QAction *toggleNoneAction;
    QAction *toggleChildrenAction;
    QAction *toggleParentsAction;
    QAction *toggleBothAction;
    KSelectAction *matchingCondAction;
    QAction *orBetweenAction;
    QAction *andBetweenAction;
    KSelectAction *restoreTagFiltersAction;
    QAction *onRestoreTagFiltersAction;
    QAction *offRestoreTagFiltersAction;
};

TagFilterViewNew::TagFilterViewNew(QWidget *parent, TagModel *tagModel,
                                   TagModificationHelper *tagModificationHelper) :
                TagFolderViewNew(parent, tagModel, tagModificationHelper),
                d(new TagFilterViewNewPriv)
{

    setSelectAlbumOnClick(false);
    setExpandOnSingleClick(false);
    setSelectOnContextMenu(false);

    // prepare custom menu action
    d->selectTagsMenu = new KMenu(i18nc("select tags menu", "Select"), this);
    d->selectAllTagsAction = d->selectTagsMenu->addAction(i18n("All Tags"));
    d->selectTagsMenu->addSeparator();
    d->selectChildrenAction = d->selectTagsMenu->addAction(i18n("Children"));
    d->selectParentsAction  = d->selectTagsMenu->addAction(i18n("Parents"));

    d->deselectTagsMenu = new KMenu(i18nc("deselect tags menu", "Deselect"), this);
    d->deselectAllTagsAction = d->deselectTagsMenu->addAction(i18n("All Tags"));
    d->deselectTagsMenu->addSeparator();
    d->deselectChildrenAction = d->deselectTagsMenu->addAction(i18n("Children"));
    d->deselectParentsAction  = d->deselectTagsMenu->addAction(i18n("Parents"));

    d->invertAction = new QAction(i18n("Invert Selection"), this);

    d->toggleAutoAction = new KSelectAction(i18n("Toggle Auto"), this);
    d->toggleNoneAction     = d->toggleAutoAction->addAction(i18nc("no auto toggle", "None"));
    d->toggleAutoAction->menu()->addSeparator();
    d->toggleChildrenAction = d->toggleAutoAction->addAction(i18nc("toggle child tags", "Children"));
    d->toggleParentsAction  = d->toggleAutoAction->addAction(i18nc("toggle parent tag", "Parents"));
    d->toggleBothAction     = d->toggleAutoAction->addAction(i18nc("toggle child and parent tags", "Both"));

    d->matchingCondAction = new KSelectAction(i18n("Matching Condition"), this);
    d->orBetweenAction = d->matchingCondAction->addAction(i18n("Or Between Tags"));
    d->andBetweenAction = d->matchingCondAction->addAction(i18n("And Between Tags"));

    d->restoreTagFiltersAction = new KSelectAction(i18n("Restore Tag Filters"), this);
    d->onRestoreTagFiltersAction = d->restoreTagFiltersAction->addAction(i18n("On"));
    d->offRestoreTagFiltersAction = d->restoreTagFiltersAction->addAction(i18n("Off"));

    connect(albumModel(), SIGNAL(checkStateChanged(Album*, Qt::CheckState)),
            this, SLOT(slotCheckStateChange(Album*, Qt::CheckState)));

}

TagFilterViewNew::~TagFilterViewNew()
{
    delete d;
}

void TagFilterViewNew::slotResetTagFilters()
{
    albumModel()->resetAllCheckedAlbums();
}

void TagFilterViewNew::slotCheckStateChange(Album *album, Qt::CheckState state)
{

    Q_UNUSED(album);
    Q_UNUSED(state);

    // handle custom toggle modes
    albumModel()->blockSignals(true);
    // avoid signal recursion here
    switch(d->toggleAutoTags)
    {
    case Children:
        albumModel()->setCheckStateForChildren(album, state);
        break;
    case Parents:
        albumModel()->setCheckStateForParents(album, state);
        break;
    case ChildrenAndParents:
        albumModel()->setCheckStateForChildren(album, state);
        albumModel()->setCheckStateForParents(album, state);
        break;
    default:
        break;
    }
    albumModel()->blockSignals(false);

    filterChanged();

}

void TagFilterViewNew::filterChanged()
{

    // TODO update, untagged does not work right now

    QList<int> tagIds;
    foreach(Album *album, albumModel()->checkedAlbums())
    {
        TAlbum *tag = dynamic_cast<TAlbum*> (album);
        if (tag)
        {
            tagIds << tag->id();
        }
    }

    emit tagFilterChanged(tagIds, d->matchingCond, false);

}

void TagFilterViewNew::loadViewState(KConfigGroup &group, QString prefix)
{
    TagFolderViewNew::loadViewState(group, prefix);

    // TODO update
//    KSharedConfig::Ptr config = KGlobal::config();
//    KConfigGroup group = config->group(objectName());
//    d->matchingCond = (ImageFilterSettings::MatchingCondition)
//                      (group.readEntry("Matching Condition", (int)ImageFilterSettings::OrCondition));
//
//    d->toggleAutoTags    = (ToggleAutoTags)
//                           (group.readEntry("Toggle Auto Tags", (int)NoToggleAuto));
//
//    d->restoreTagFilters = (RestoreTagFilters)
//                           (group.readEntry("Restore Tag Filters", (int)OffRestoreTagFilters));
}

void TagFilterViewNew::saveViewState(KConfigGroup &group, QString prefix)
{
    TagFolderViewNew::saveViewState(group, prefix);

    // TODO update
//    KSharedConfig::Ptr config = KGlobal::config();
//    KConfigGroup group        = config->group(objectName());
//    group.writeEntry("Matching Condition",  (int)(d->matchingCond));
//    group.writeEntry("Toggle Auto Tags",    (int)(d->toggleAutoTags));
//    group.writeEntry("Restore Tag Filters", (int)(d->restoreTagFilters));
//    saveTagFilters();
//    config->sync();

}

void TagFilterViewNew::addCustomContextMenuActions(ContextMenuHelper &cmh, TAlbum *tag)
{
    TagFolderViewNew::addCustomContextMenuActions(cmh, tag);

    cmh.addSeparator();

    // selection (checked) modification
    cmh.addSubMenu(d->selectTagsMenu);
    cmh.addSubMenu(d->deselectTagsMenu);

    d->selectChildrenAction->setEnabled(tag);
    d->selectParentsAction->setEnabled(tag);
    d->deselectChildrenAction->setEnabled(tag);
    d->deselectParentsAction->setEnabled(tag);

    cmh.addAction(d->invertAction);

    cmh.addSeparator();

    // automatic toggle

    cmh.addAction(d->toggleAutoAction);

    d->toggleNoneAction->setChecked(d->toggleAutoTags == TagFilterViewNew::NoToggleAuto);
    d->toggleChildrenAction->setChecked(d->toggleAutoTags == TagFilterViewNew::Children);
    d->toggleParentsAction->setChecked(d->toggleAutoTags == TagFilterViewNew::Parents);
    d->toggleBothAction->setChecked(d->toggleAutoTags == TagFilterViewNew::ChildrenAndParents);

    // matching condition

    cmh.addAction(d->matchingCondAction);

    d->orBetweenAction->setChecked(d->matchingCond == ImageFilterSettings::OrCondition);
    d->andBetweenAction->setChecked(d->matchingCond != ImageFilterSettings::OrCondition);

    // restoring

    cmh.addAction(d->restoreTagFiltersAction);

    d->onRestoreTagFiltersAction->setChecked(d->restoreTagFilters == OnRestoreTagFilters);
    d->offRestoreTagFiltersAction->setChecked(d->restoreTagFilters != OnRestoreTagFilters);

}

void TagFilterViewNew::handleCustomContextMenuAction(QAction *action, TAlbum *tag)
{
    TagFolderViewNew::handleCustomContextMenuAction(action, tag);

    if (!action || !tag)
    {
        return;
    }

    QModelIndex tagIndex = albumModel()->indexForAlbum(tag);
    ToggleAutoTags toggleRestore = d->toggleAutoTags;
    d->toggleAutoTags = NoToggleAuto;
    if (action == d->selectAllTagsAction)     // Select All Tags.
    {
        albumModel()->checkAllAlbums();
    }
    else if (action == d->deselectAllTagsAction)    // Deselect All Tags.
    {
        albumModel()->resetAllCheckedAlbums();
    }
    else if (action == d->invertAction)             // Invert All Tags Selection.
    {
        albumModel()->invertCheckedAlbums();
    }
    else if (action == d->selectChildrenAction)     // Select Child Tags.
    {
        albumModel()->checkAllAlbums(tagIndex);
    }
    else if (action == d->deselectChildrenAction)   // Deselect Child Tags.
    {
        albumModel()->resetCheckedAlbums(tagIndex);
    }
    else if (action == d->selectParentsAction)     // Select Parent Tags.
    {
        albumModel()->checkAllParentAlbums(tagIndex);
    }
    else if (action == d->deselectParentsAction)   // Deselect Parent Tags.
    {
        albumModel()->resetCheckedParentAlbums(tagIndex);
    }
    else if (action == d->toggleNoneAction)        // No toggle auto tags.
    {
        toggleRestore = NoToggleAuto;
    }
    else if (action == d->toggleChildrenAction)    // Toggle auto Children tags.
    {
        toggleRestore = Children;
    }
    else if (action == d->toggleParentsAction)     // Toggle auto Parents tags.
    {
        toggleRestore = Parents;
    }
    else if (action == d->toggleBothAction)        // Toggle auto Children and Parents tags.
    {
        toggleRestore = ChildrenAndParents;
    }
    else if (action == d->orBetweenAction)         // Or Between Tags.
    {
        d->matchingCond = ImageFilterSettings::OrCondition;
        filterChanged();
    }
    else if (action == d->andBetweenAction)        // And Between Tags.
    {
        d->matchingCond = ImageFilterSettings::AndCondition;
        filterChanged();
    }
    else if (action == d->onRestoreTagFiltersAction)        // Restore TagFilters ON.
    {
        d->restoreTagFilters = TagFilterViewNew::OnRestoreTagFilters;
        filterChanged();
    }
    else if (action == d->offRestoreTagFiltersAction)        // Restore TagFilters OFF.
    {
        d->restoreTagFilters = TagFilterViewNew::OffRestoreTagFilters;
        filterChanged();
    }
    else
    {
        kWarning() << "Did not handle action " << action;
    }
    d->toggleAutoTags = toggleRestore;

}

}
