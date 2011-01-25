/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-01-24
 * Description : Tags Action Manager
 *
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "tagsactionmngr.moc"

// Qt includes

#include <QSortFilterProxyModel>

// KDE includes

#include <kaction.h>
#include <kapplication.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kicon.h>
#include <kdebug.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "databaseaccess.h"
#include "databaseconstants.h"
#include "digikamapp.h"
#include "imagewindow.h"
#include "lighttablewindow.h"

namespace Digikam
{

TagsActionMngr* TagsActionMngr::m_defaultManager = 0;

TagsActionMngr* TagsActionMngr::defaultManager()
{
    return m_defaultManager;
}

class TagsActionMngr::TagsActionMngrPrivate
{
public:

    TagsActionMngrPrivate()
    {
        actionCollection = 0;
        view             = 0;
    }

    QMap<int, KAction*> tagsActionMap;
    QWidget*            view;
    KActionCollection*  actionCollection;
};

// -------------------------------------------------------------------------------------------------

TagsActionMngr::TagsActionMngr(QWidget* parent, KActionCollection* actionCollection)
    : QObject(parent), d(new TagsActionMngrPrivate)
{
    if (!m_defaultManager)
    {
        m_defaultManager = this;
    }

    d->actionCollection = actionCollection;
    d->view             = parent;

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));
}

TagsActionMngr::~TagsActionMngr()
{
    delete d;

    if (m_defaultManager == this)
    {
        m_defaultManager = 0;
    }
}

KActionCollection* TagsActionMngr::actionCollection() const
{
    return d->actionCollection;
}

void TagsActionMngr::createActions()
{
    TagInfo::List tList = DatabaseAccess().db()->scanTags();

    for (TagInfo::List::const_iterator it = tList.constBegin(); it != tList.constEnd(); ++it)
    {
        TagProperties tprop((*it).id);

        if (tprop.hasProperty(TagPropertyName::tagKeyboardShortcut()))
        {
            createTagActionShortcut(*it, tprop);
        }
    }
}

bool TagsActionMngr::createTagActionShortcut(int tagId)
{
    kDebug() << tagId;
    if (!tagId) return false;

    TagInfo tinfo = DatabaseAccess().db()->getTagInfo(tagId);
    if (tinfo.isNull()) return false;

    TagProperties tprop(tinfo.id);
    createTagActionShortcut(tinfo, tprop);

    return true;
}

void TagsActionMngr::createTagActionShortcut(const TagInfo& tinfo, const TagProperties& tprop)
{
    KAction* action = d->actionCollection->addAction(QString("tagshortcut-%1").arg(tinfo.id));
    action->setText(i18n("Assign Tag \"%1\"", tinfo.name));
    action->setShortcut(KShortcut(tprop.value(TagPropertyName::tagKeyboardShortcut())));
    action->setShortcutConfigurable(false);
    action->setIcon(KIcon(tinfo.icon));
    action->setData(tinfo.id);

    connect(action, SIGNAL(triggered()),
            this, SLOT(slotAssignTagsFromShortcut()));

    d->tagsActionMap[tinfo.id] = action;

    kDebug() << "Create Shortcut " << action->shortcut().toString()
             << " to Tag " << tinfo.name << " (" << tinfo.id << ")";
}

void TagsActionMngr::slotUpdateTagShortcut(int tagId, const QKeySequence& ks)
{
    kDebug() << tagId;
    if (!tagId) return;

    tagRemoved(tagId);

    TagProperties tprop(tagId);
    tprop.setProperty(TagPropertyName::tagKeyboardShortcut(), ks.toString());

    createTagActionShortcut(tagId);
}

void TagsActionMngr::slotAlbumDeleted(Album* album)
{
    TAlbum* talbum = dynamic_cast<TAlbum*>(album);
    if (!talbum) return;

    tagRemoved(talbum->id());
}

void TagsActionMngr::tagRemoved(int tagId)
{
    KAction* action = d->tagsActionMap[tagId];
    if (action)
    {
        // NOTE: Action is deleted by KActionCollection
        d->actionCollection->removeAction(action);
        delete d->tagsActionMap.take(tagId);
    }
}

void TagsActionMngr::slotAssignTagsFromShortcut()
{
    KAction* action = dynamic_cast<KAction*>(sender());
    if (!action) return;

    int tagId = action->data().toInt();
    kDebug() << "Fired Tag Shortcut " << tagId;

    QWidget* w      = kapp->activeWindow();
    DigikamApp* lw1 = dynamic_cast<DigikamApp*>(w);
    if (lw1)
    {
        kDebug() << "Handling by DigikamApp";
        return;
    }

    ImageWindow* lw2 = dynamic_cast<ImageWindow*>(w);
    if (lw2)
    {
        kDebug() << "Handling by ImageWindow";
        return;
    }

    LightTableWindow* lw3 = dynamic_cast<LightTableWindow*>(w);
    if (lw3)
    {
        kDebug() << "Handling by LightTableWindow";
        return;
    }

    emit signalAssignTagsFromShortcut(tagId);
}

} // namespace Digikam
