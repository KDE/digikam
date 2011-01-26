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

#include <QList>

// KDE includes

#include <kaction.h>
#include <kapplication.h>
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
#include "digikamview.h"
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
    }

    QMultiMap<int, KAction*>  tagsActionMap;
    QList<KActionCollection*> actionCollectionList;
};

// -------------------------------------------------------------------------------------------------

TagsActionMngr::TagsActionMngr(QWidget* parent)
    : QObject(parent), d(new TagsActionMngrPrivate)
{
    if (!m_defaultManager)
    {
        m_defaultManager = this;
    }

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

void TagsActionMngr::registerActionCollections()
{
    d->actionCollectionList.append(DigikamApp::instance()->actionCollection());
    d->actionCollectionList.append(ImageWindow::imageWindow()->actionCollection());
    d->actionCollectionList.append(LightTableWindow::lightTableWindow()->actionCollection());
    createActions();
}

QList<KActionCollection*> TagsActionMngr::actionCollections() const
{
    return d->actionCollectionList;
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

    // Create all Rating star action shorcuts.
    KAction* action = 0;

    foreach(KActionCollection* ac, d->actionCollectionList)
    {
        action = ac->addAction("ratenostar");
        action->setText(i18n("Assign Rating \"No Stars\""));
        action->setShortcut(KShortcut(Qt::CTRL+Qt::Key_0));
        action->setShortcutConfigurable(false);
        action->setData(0);
        connect(action, SIGNAL(triggered()), this, SLOT(slotAssignRatingFromShortcut()));

        action = ac->addAction("rateonestar");
        action->setText(i18n("Assign Rating \"One Star\""));
        action->setShortcut(KShortcut(Qt::CTRL+Qt::Key_1));
        action->setShortcutConfigurable(false);
        action->setData(1);
        connect(action, SIGNAL(triggered()), this, SLOT(slotAssignRatingFromShortcut()));

        action = ac->addAction("ratetwostar");
        action->setText(i18n("Assign Rating \"Two Stars\""));
        action->setShortcut(KShortcut(Qt::CTRL+Qt::Key_2));
        action->setShortcutConfigurable(false);
        action->setData(2);
        connect(action, SIGNAL(triggered()), this, SLOT(slotAssignRatingFromShortcut()));

        action = ac->addAction("ratethreestar");
        action->setText(i18n("Assign Rating \"Three Stars\""));
        action->setShortcut(KShortcut(Qt::CTRL+Qt::Key_3));
        action->setShortcutConfigurable(false);
        action->setData(3);
        connect(action, SIGNAL(triggered()), this, SLOT(slotAssignRatingFromShortcut()));

        action = ac->addAction("ratefourstar");
        action->setText(i18n("Assign Rating \"Four Stars\""));
        action->setShortcut(KShortcut(Qt::CTRL+Qt::Key_4));
        action->setShortcutConfigurable(false);
        action->setData(4);
        connect(action, SIGNAL(triggered()), this, SLOT(slotAssignRatingFromShortcut()));

        action = ac->addAction("ratefivestar");
        action->setText(i18n("Assign Rating \"Five Stars\""));
        action->setShortcut(KShortcut(Qt::CTRL+Qt::Key_5));
        action->setShortcutConfigurable(false);
        action->setData(5);
        connect(action, SIGNAL(triggered()), this, SLOT(slotAssignRatingFromShortcut()));
    }
}

bool TagsActionMngr::createTagActionShortcut(int tagId)
{
    if (!tagId) return false;

    TagInfo tinfo = DatabaseAccess().db()->getTagInfo(tagId);
    if (tinfo.isNull()) return false;

    TagProperties tprop(tinfo.id);
    createTagActionShortcut(tinfo, tprop);

    return true;
}

void TagsActionMngr::createTagActionShortcut(const TagInfo& tinfo, const TagProperties& tprop)
{
    KShortcut ks(tprop.value(TagPropertyName::tagKeyboardShortcut()));
    KIcon     icon(tinfo.icon);

    kDebug() << "Create Shortcut " << ks.toString()
             << " to Tag " << tinfo.name << " (" << tinfo.id << ")";

    foreach(KActionCollection* ac, d->actionCollectionList)
    {
        KAction* action = ac->addAction(QString("tagshortcut-%1").arg(tinfo.id));
        action->setText(i18n("Assign Tag \"%1\"", tinfo.name));
        action->setShortcut(ks);
        action->setShortcutConfigurable(false);
        action->setIcon(icon);
        action->setData(tinfo.id);

        connect(action, SIGNAL(triggered()),
                this, SLOT(slotAssignTagsFromShortcut()));

        d->tagsActionMap.insert(tinfo.id, action);
    }
}

void TagsActionMngr::slotUpdateTagShortcut(int tagId, const QKeySequence& ks)
{
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
    int count = d->tagsActionMap.count(tagId);
    if (count)
    {
        foreach(KAction* act, d->tagsActionMap.values(tagId))
        {
            if (act)
            {
                KActionCollection* ac = dynamic_cast<KActionCollection*>(act->parent());
                if (ac)
                {
                    // NOTE: Action is deleted by KActionCollection
                    ac->takeAction(act);
                }
            }
        }

        for (int i =0 ; i < count ; ++i)
            delete d->tagsActionMap.take(tagId);

        kDebug() << "Delete Shortcut assigned to tag " << tagId;
    }
}

void TagsActionMngr::slotAssignTagsFromShortcut()
{
    KAction* action = dynamic_cast<KAction*>(sender());
    if (!action) return;

    int tagId = action->data().toInt();
    kDebug() << "Fired Tag Shortcut " << tagId;

    QWidget* w      = kapp->activeWindow();
    DigikamApp* dkw = dynamic_cast<DigikamApp*>(w);
    if (dkw)
    {
        kDebug() << "Handling by DigikamApp";
        dkw->view()->toggleTag(tagId);
        return;
    }

    ImageWindow* imw = dynamic_cast<ImageWindow*>(w);
    if (imw)
    {
        kDebug() << "Handling by ImageWindow";
        imw->toggleTag(tagId);
        return;
    }

    LightTableWindow* ltw = dynamic_cast<LightTableWindow*>(w);
    if (ltw)
    {
        kDebug() << "Handling by LightTableWindow";
        ltw->toggleTag(tagId);
        return;
    }
}

void TagsActionMngr::slotAssignRatingFromShortcut()
{
    KAction* action = dynamic_cast<KAction*>(sender());
    if (!action) return;

    int rate = action->data().toInt();
    kDebug() << "Fired Rating Shortcut " << rate;

    QWidget* w      = kapp->activeWindow();
    DigikamApp* dkw = dynamic_cast<DigikamApp*>(w);
    if (dkw)
    {
        kDebug() << "Handling by DigikamApp";
        dkw->view()->slotAssignRating(rate);
        return;
    }

    ImageWindow* imw = dynamic_cast<ImageWindow*>(w);
    if (imw)
    {
        kDebug() << "Handling by ImageWindow";
        imw->slotAssignRating(rate);
        return;
    }

    LightTableWindow* ltw = dynamic_cast<LightTableWindow*>(w);
    if (ltw)
    {
        kDebug() << "Handling by LightTableWindow";
        ltw->slotAssignRating(rate);
        return;
    }
}

} // namespace Digikam
