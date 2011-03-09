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
#include "picklabelwidget.h"
#include "colorlabelwidget.h"
#include "tagscache.h"

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
        : ratingShortcutPrefix("rateshortcut"),
          tagShortcutPrefix("tagshortcut"),
          pickShortcutPrefix("pickshortcut"),
          colorShortcutPrefix("colorshortcut")
    {
    }

    QMultiMap<int, KAction*>  tagsActionMap;
    QList<KActionCollection*> actionCollectionList;

    const QString             ratingShortcutPrefix;
    const QString             tagShortcutPrefix;
    const QString             pickShortcutPrefix;
    const QString             colorShortcutPrefix;
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

QString TagsActionMngr::ratingShortcutPrefix() const
{
    return d->ratingShortcutPrefix;
}

QString TagsActionMngr::tagShortcutPrefix() const
{
    return d->tagShortcutPrefix;
}

QString TagsActionMngr::pickShortcutPrefix() const
{
    return d->pickShortcutPrefix;
}

QString TagsActionMngr::colorShortcutPrefix() const
{
    return d->colorShortcutPrefix;
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
    // Create Tags shortcuts.

    TagInfo::List tList = DatabaseAccess().db()->scanTags();

    for (TagInfo::List::const_iterator it = tList.constBegin(); it != tList.constEnd(); ++it)
    {
        TagProperties tprop((*it).id);

        if (tprop.hasProperty(TagPropertyName::tagKeyboardShortcut()))
        {
            createTagActionShortcut(*it, tprop);
        }
    }

    // Create Rating shortcuts.

    foreach(KActionCollection* ac, d->actionCollectionList)
    {
        for (int i = RatingMin ; i <= RatingMax ; ++i)
        {
            createRatingActionShortcut(ac, i);
        }
    }

    // Create Color Label shortcuts.

    foreach(KActionCollection* ac, d->actionCollectionList)
    {
        for (int i = NoColorLabel ; i <= WhiteLabel ; ++i)
        {
            createColorLabelActionShortcut(ac, i);
        }
    }

    // Create Pick Label shortcuts.

    foreach(KActionCollection* ac, d->actionCollectionList)
    {
        for (int i = NoPickLabel ; i <= AcceptedLabel ; ++i)
        {
            createPickLabelActionShortcut(ac, i);
        }
    }
}

bool TagsActionMngr::createRatingActionShortcut(KActionCollection* ac, int rating)
{
    if (ac)
    {
        KAction* action = ac->addAction(QString("%1-%2").arg(d->ratingShortcutPrefix).arg(rating));
        action->setText(i18n("Assign Rating \"%1 Star\"", rating));
        action->setShortcut(KShortcut(QString("CTRL+%1").arg(rating)));
        action->setShortcutConfigurable(false);
        action->forgetGlobalShortcut();
        action->setData(rating);

        connect(action, SIGNAL(triggered()),
                this, SLOT(slotAssignFromShortcut()));

        return true;
    }

    return false;
}

bool TagsActionMngr::createPickLabelActionShortcut(KActionCollection* ac, int pickId)
{
    if (ac)
    {
        KAction* action = ac->addAction(QString("%1-%2").arg(d->pickShortcutPrefix).arg(pickId));
        action->setText(i18n("Assign Pick Label \"%1\"", PickLabelWidget::labelPickName((PickLabel)pickId)));
        action->setShortcut(KShortcut(QString("ALT+%1").arg(pickId)));
        action->setShortcutConfigurable(false);
        action->forgetGlobalShortcut();
        action->setData(pickId);

        connect(action, SIGNAL(triggered()),
                this, SLOT(slotAssignFromShortcut()));

        return true;
    }

    return false;
}

bool TagsActionMngr::createColorLabelActionShortcut(KActionCollection* ac, int colorId)
{
    if (ac)
    {
        KAction* action = ac->addAction(QString("%1-%2").arg(d->colorShortcutPrefix).arg(colorId));
        action->setText(i18n("Assign Color Label \"%1\"", ColorLabelWidget::labelColorName((ColorLabel)colorId)));
        action->setShortcut(KShortcut(QString("ALT+CTRL+%1").arg(colorId)));
        action->setShortcutConfigurable(false);
        action->forgetGlobalShortcut();
        action->setData(colorId);

        connect(action, SIGNAL(triggered()),
                this, SLOT(slotAssignFromShortcut()));

        return true;
    }

    return false;
}

bool TagsActionMngr::createTagActionShortcut(int tagId)
{
    if (!tagId)
    {
        return false;
    }

    TagInfo tinfo = DatabaseAccess().db()->getTagInfo(tagId);

    if (tinfo.isNull())
    {
        return false;
    }

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
        KAction* action = ac->addAction(QString("%1-%2").arg(d->tagShortcutPrefix).arg(tinfo.id));
        action->setText(i18n("Assign Tag \"%1\"", tinfo.name));
        action->setParent(this);
        action->setShortcut(ks);
        action->setShortcutConfigurable(true);
        action->forgetGlobalShortcut();
        action->setIcon(icon);
        action->setData(tinfo.id);

        connect(action, SIGNAL(triggered()),
                this, SLOT(slotAssignFromShortcut()));

        connect(action, SIGNAL(changed()),
                this, SLOT(slotTagActionChanged()));

        d->tagsActionMap.insert(tinfo.id, action);
    }
}

void TagsActionMngr::slotTagActionChanged()
{
    KAction* action = dynamic_cast<KAction*>(sender());

    if (!action)
    {
        return;
    }

    int tagId       = action->data().toInt();
    QKeySequence ks = action->shortcut().primary();
    updateTagShortcut(tagId, ks);
}

void TagsActionMngr::updateTagShortcut(int tagId, const QKeySequence& ks)
{
    if (!tagId)
    {
        return;
    }

    kDebug() << "Tag Shortcut " << tagId << "Changed to " << ks;

    TagProperties tprop(tagId);
    tprop.setProperty(TagPropertyName::tagKeyboardShortcut(), ks.toString());
}

void TagsActionMngr::slotAlbumDeleted(Album* album)
{
    TAlbum* talbum = dynamic_cast<TAlbum*>(album);

    if (!talbum)
    {
        return;
    }

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
        {
            delete d->tagsActionMap.take(tagId);
        }

        kDebug() << "Delete Shortcut assigned to tag " << tagId;
    }
}

void TagsActionMngr::slotAssignFromShortcut()
{
    KAction* action = dynamic_cast<KAction*>(sender());

    if (!action)
    {
        return;
    }

    int val = action->data().toInt();
    kDebug() << "Shortcut value: " << val;

    QWidget* w      = kapp->activeWindow();
    DigikamApp* dkw = dynamic_cast<DigikamApp*>(w);

    if (dkw)
    {
        kDebug() << "Handling by DigikamApp";

        if (action->objectName().startsWith(d->ratingShortcutPrefix))
        {
            dkw->view()->slotAssignRating(val);
        }
        else if (action->objectName().startsWith(d->tagShortcutPrefix))
        {
            dkw->view()->toggleTag(val);
        }
        else if (action->objectName().startsWith(d->pickShortcutPrefix))
        {
            dkw->view()->slotAssignPickLabel(val);
        }
        else if (action->objectName().startsWith(d->colorShortcutPrefix))
        {
            dkw->view()->slotAssignColorLabel(val);
        }

        return;
    }

    ImageWindow* imw = dynamic_cast<ImageWindow*>(w);

    if (imw)
    {
        kDebug() << "Handling by ImageWindow";

        if (action->objectName().startsWith(d->ratingShortcutPrefix))
        {
            imw->slotAssignRating(val);
        }
        else if (action->objectName().startsWith(d->tagShortcutPrefix))
        {
            imw->toggleTag(val);
        }
        else if (action->objectName().startsWith(d->pickShortcutPrefix))
        {
            imw->slotAssignPickLabel(val);
        }
        else if (action->objectName().startsWith(d->colorShortcutPrefix))
        {
            imw->slotAssignColorLabel(val);
        }

        return;
    }

    LightTableWindow* ltw = dynamic_cast<LightTableWindow*>(w);

    if (ltw)
    {
        kDebug() << "Handling by LightTableWindow";

        if (action->objectName().startsWith(d->ratingShortcutPrefix))
        {
            ltw->slotAssignRating(val);
        }
        else if (action->objectName().startsWith(d->tagShortcutPrefix))
        {
            ltw->toggleTag(val);
        }
        else if (action->objectName().startsWith(d->pickShortcutPrefix))
        {
            ltw->slotAssignPickLabel(val);
        }
        else if (action->objectName().startsWith(d->colorShortcutPrefix))
        {
            ltw->slotAssignColorLabel(val);
        }

        return;
    }
}

} // namespace Digikam
