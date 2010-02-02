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

#include "tagcheckview.moc"

// Qt includes
#include <qaction.h>

// KDE includes
#include <kdebug.h>
#include <kmenu.h>
#include <kselectaction.h>

// Local includes
#include "contextmenuhelper.h"

namespace Digikam
{

class TagCheckViewPriv
{
public:
    TagCheckViewPriv() :
        configToggleAutoTagsEntry("Toggle Auto Tags"),
        toggleAutoTags(TagCheckView::NoToggleAuto)
    {
    }

    const QString configToggleAutoTagsEntry;

    TagCheckView::ToggleAutoTags       toggleAutoTags;

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
};

TagCheckView::TagCheckView(QWidget *parent, TagModel *tagModel) :
                TagFolderView(parent, tagModel),
                d(new TagCheckViewPriv)
{

    setSelectAlbumOnClick(false);
    setExpandOnSingleClick(false);
    setSelectOnContextMenu(false);

    setShowFindDuplicateAction(false);

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

    connect(albumModel(), SIGNAL(checkStateChanged(Album*, Qt::CheckState)),
            this, SLOT(slotCheckStateChange(Album*, Qt::CheckState)));

}

TagCheckView::~TagCheckView()
{
    delete d;
}

void TagCheckView::slotResetCheckState()
{
    albumModel()->resetAllCheckedAlbums();
}

void TagCheckView::slotCheckStateChange(Album *album, Qt::CheckState state)
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

    emit checkedTagsChanged(getCheckedTags());

}

void TagCheckView::doLoadState()
{
    TagFolderView::doLoadState();

    KConfigGroup group = getConfigGroup();
    d->toggleAutoTags  = (ToggleAutoTags)
                         (group.readEntry(entryName(d->configToggleAutoTagsEntry), (int)NoToggleAuto));
}

void TagCheckView::doSaveState()
{
    TagFolderView::doSaveState();

    KConfigGroup group = getConfigGroup();
    group.writeEntry(entryName(d->configToggleAutoTagsEntry), (int)(d->toggleAutoTags));
    group.sync();

}

QList<TAlbum*> TagCheckView::getCheckedTags() const
{
    QList<TAlbum*> tags;
    foreach(Album *album, albumModel()->checkedAlbums())
    {
        TAlbum *tag = dynamic_cast<TAlbum*> (album);
        if (tag)
        {
            tags << tag;
        }
    }
    return tags;
}

TagCheckView::ToggleAutoTags TagCheckView::getToggleAutoTags() const
{
    return d->toggleAutoTags;
}

void TagCheckView::setToggleAutoTags(TagCheckView::ToggleAutoTags toggle)
{
    d->toggleAutoTags = toggle;
}

void TagCheckView::addCustomContextMenuActions(ContextMenuHelper &cmh, Album *album)
{
    TagFolderView::addCustomContextMenuActions(cmh, album);

    TAlbum *tag = dynamic_cast<TAlbum*> (album);
    if (!tag)
    {
        return;
    }

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

    d->toggleNoneAction->setChecked(d->toggleAutoTags == TagCheckView::NoToggleAuto);
    d->toggleChildrenAction->setChecked(d->toggleAutoTags == TagCheckView::Children);
    d->toggleParentsAction->setChecked(d->toggleAutoTags == TagCheckView::Parents);
    d->toggleBothAction->setChecked(d->toggleAutoTags == TagCheckView::ChildrenAndParents);

}

void TagCheckView::handleCustomContextMenuAction(QAction *action, Album *album)
{
    TagFolderView::handleCustomContextMenuAction(action, album);

    TAlbum *tag = dynamic_cast<TAlbum*> (album);

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
    d->toggleAutoTags = toggleRestore;

}

}
