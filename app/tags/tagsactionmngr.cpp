/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-01-24
 * Description : Tags Action Manager
 *
 * Copyright (C) 2011-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QShortcut>

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
#include "databasewatch.h"
#include "databaseinfocontainers.h"
#include "digikamapp.h"
#include "dxmlguiwindow.h"
#include "digikamview.h"
#include "imagewindow.h"
#include "lighttablewindow.h"
#include "picklabelwidget.h"
#include "colorlabelwidget.h"
#include "tagscache.h"
#include "tagproperties.h"
#include "ratingwidget.h"
#include "slideshow.h"
#include "syncjob.h"

namespace Digikam
{

TagsActionMngr* TagsActionMngr::m_defaultManager = 0;

TagsActionMngr* TagsActionMngr::defaultManager()
{
    return m_defaultManager;
}

class TagsActionMngr::Private
{
public:

    Private()
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

TagsActionMngr::TagsActionMngr(QWidget* const parent)
    : QObject(parent), d(new Private)
{
    if (!m_defaultManager)
    {
        m_defaultManager = this;
    }

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(imageTagChange(ImageTagChangeset)),
            this, SLOT(slotImageTagChanged(ImageTagChangeset)));
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

void TagsActionMngr::registerTagsActionCollections()
{
    d->actionCollectionList.append(DigikamApp::instance()->actionCollection());
    d->actionCollectionList.append(ImageWindow::imageWindow()->actionCollection());
    d->actionCollectionList.append(LightTableWindow::lightTableWindow()->actionCollection());

    // Create Tags shortcuts.

    QList<int> tagIds = TagsCache::instance()->tagsWithProperty(TagPropertyName::tagKeyboardShortcut());

    foreach(int tagId, tagIds)
    {
        createTagActionShortcut(tagId);
    }
}

QList<KActionCollection*> TagsActionMngr::actionCollections() const
{
    return d->actionCollectionList;
}

void TagsActionMngr::registerLabelsActions(KActionCollection* const ac)
{
    // Create Rating shortcuts.

    for (int i = RatingMin ; i <= RatingMax ; ++i)
    {
        createRatingActionShortcut(ac, i);
    }

    // Create Color Label shortcuts.

    for (int i = NoColorLabel ; i <= WhiteLabel ; ++i)
    {
        createColorLabelActionShortcut(ac, i);
    }

    // Create Pick Label shortcuts.

    for (int i = NoPickLabel ; i <= AcceptedLabel ; ++i)
    {
        createPickLabelActionShortcut(ac, i);
    }
}

void TagsActionMngr::registerActionsToWidget(QWidget* const wdg)
{
    DXmlGuiWindow* const win = dynamic_cast<DXmlGuiWindow*>(kapp->activeWindow());

    if (win)
    {
        foreach(QAction* const ac, win->actionCollection()->actions())
        {
            if (ac->objectName().startsWith(d->ratingShortcutPrefix) ||
                ac->objectName().startsWith(d->tagShortcutPrefix)    ||
                ac->objectName().startsWith(d->pickShortcutPrefix)   ||
                ac->objectName().startsWith(d->colorShortcutPrefix))
            {
                wdg->addAction(ac);
            }
        }
    }
}

bool TagsActionMngr::createRatingActionShortcut(KActionCollection* const ac, int rating)
{
    if (ac)
    {
        KAction* const action = ac->addAction(QString("%1-%2").arg(d->ratingShortcutPrefix).arg(rating));
        action->setText(i18n("Assign Rating \"%1 Star\"", rating));
        action->setShortcut(KShortcut(QString("CTRL+%1").arg(rating)));
        action->setShortcutConfigurable(true);
        action->forgetGlobalShortcut();
        action->setIcon(RatingWidget::buildIcon(rating, KIconLoader::SizeSmallMedium));
        action->setData(rating);

        connect(action, SIGNAL(triggered()),
                this, SLOT(slotAssignFromShortcut()));

        return true;
    }

    return false;
}

bool TagsActionMngr::createPickLabelActionShortcut(KActionCollection* const ac, int pickId)
{
    if (ac)
    {
        KAction* const action = ac->addAction(QString("%1-%2").arg(d->pickShortcutPrefix).arg(pickId));
        action->setText(i18n("Assign Pick Label \"%1\"", PickLabelWidget::labelPickName((PickLabel)pickId)));
        action->setShortcut(KShortcut(QString("ALT+%1").arg(pickId)));
        action->setShortcutConfigurable(true);
        action->forgetGlobalShortcut();
        action->setIcon(PickLabelWidget::buildIcon((PickLabel)pickId, KIconLoader::SizeSmallMedium));
        action->setData(pickId);

        connect(action, SIGNAL(triggered()),
                this, SLOT(slotAssignFromShortcut()));

        return true;
    }

    return false;
}

bool TagsActionMngr::createColorLabelActionShortcut(KActionCollection* const ac, int colorId)
{
    if (ac)
    {
        KAction* const action = ac->addAction(QString("%1-%2").arg(d->colorShortcutPrefix).arg(colorId));
        action->setText(i18n("Assign Color Label \"%1\"", ColorLabelWidget::labelColorName((ColorLabel)colorId)));
        action->setShortcut(KShortcut(QString("ALT+CTRL+%1").arg(colorId)));
        action->setShortcutConfigurable(true);
        action->forgetGlobalShortcut();
        action->setIcon(ColorLabelWidget::buildIcon((ColorLabel)colorId, KIconLoader::SizeSmallMedium));
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

    TAlbum* const talbum = AlbumManager::instance()->findTAlbum(tagId);

    if (!talbum)
    {
        return false;
    }

    QString value = TagsCache::instance()->propertyValue(tagId, TagPropertyName::tagKeyboardShortcut());

    if (value.isEmpty())
    {
        return false;
    }

    KShortcut ks(value);
    // FIXME: tag icons can be files on disk, or system icon names. Only the latter will work here.
    KIcon     icon(SyncJob::getTagThumbnail(talbum));

    kDebug() << "Create Shortcut " << ks.toString()
             << " to Tag " << talbum->title() << " (" << tagId << ")";

    foreach(KActionCollection* const ac, d->actionCollectionList)
    {
        KAction* const action = ac->addAction(QString("%1-%2").arg(d->tagShortcutPrefix).arg(tagId));
        action->setText(i18n("Assign Tag \"%1\"", talbum->title()));
        action->setParent(this);
        action->setShortcut(ks);
        action->setShortcutConfigurable(true);
        action->forgetGlobalShortcut();
        action->setIcon(icon);
        action->setData(tagId);

        connect(action, SIGNAL(triggered()),
                this, SLOT(slotAssignFromShortcut()));

        connect(action, SIGNAL(changed()),
                this, SLOT(slotTagActionChanged()));

        d->tagsActionMap.insert(tagId, action);
    }

    return true;
}

void TagsActionMngr::slotTagActionChanged()
{
    KAction* const action = dynamic_cast<KAction*>(sender());

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

    QString value = TagsCache::instance()->propertyValue(tagId, TagPropertyName::tagKeyboardShortcut());

    if (value == ks.toString())
    {
        return;
    }

    TagProperties tprop(tagId);

    if (ks.isEmpty())
    {
        removeTagActionShortcut(tagId);
        tprop.removeProperties(TagPropertyName::tagKeyboardShortcut());
    }
    else
    {
        removeTagActionShortcut(tagId);
        tprop.setProperty(TagPropertyName::tagKeyboardShortcut(), ks.toString());
        createTagActionShortcut(tagId);
    }
}

void TagsActionMngr::slotAlbumDeleted(Album* album)
{
    TAlbum* const talbum = dynamic_cast<TAlbum*>(album);

    if (!talbum)
    {
        return;
    }

    removeTagActionShortcut(talbum->id());
    kDebug() << "Delete Shortcut assigned to tag " << album->id();
}

bool TagsActionMngr::removeTagActionShortcut(int tagId)
{
    if (!d->tagsActionMap.contains(tagId))
    {
        return false;
    }

    foreach(KAction* const act, d->tagsActionMap.values(tagId))
    {
        if (act)
        {
            KActionCollection* const ac = dynamic_cast<KActionCollection*>(act->parent());

            if (ac)
            {
                ac->takeAction(act);
            }

            delete act;
        }
    }

    d->tagsActionMap.remove(tagId);

    return true;
}

void TagsActionMngr::slotAssignFromShortcut()
{
    KAction* const action = dynamic_cast<KAction*>(sender());

    if (!action)
    {
        return;
    }

    int val = action->data().toInt();
    kDebug() << "Shortcut value: " << val;

    QWidget* const w      = kapp->activeWindow();
    DigikamApp* const dkw = dynamic_cast<DigikamApp*>(w);

    if (dkw)
    {
        //kDebug() << "Handling by DigikamApp";

        if (action->objectName().startsWith(d->ratingShortcutPrefix))
        {
            dkw->view()->slotAssignRating(val);
        }
        else if (action->objectName().startsWith(d->pickShortcutPrefix))
        {
            dkw->view()->slotAssignPickLabel(val);
        }
        else if (action->objectName().startsWith(d->colorShortcutPrefix))
        {
            dkw->view()->slotAssignColorLabel(val);
        }
        else if (action->objectName().startsWith(d->tagShortcutPrefix))
        {
            dkw->view()->toggleTag(val);
        }

        return;
    }

    ImageWindow* const imw = dynamic_cast<ImageWindow*>(w);

    if (imw)
    {
        //kDebug() << "Handling by ImageWindow";

        if (action->objectName().startsWith(d->ratingShortcutPrefix))
        {
            imw->slotAssignRating(val);
        }
        else if (action->objectName().startsWith(d->pickShortcutPrefix))
        {
            imw->slotAssignPickLabel(val);
        }
        else if (action->objectName().startsWith(d->colorShortcutPrefix))
        {
            imw->slotAssignColorLabel(val);
        }
        else if (action->objectName().startsWith(d->tagShortcutPrefix))
        {
            imw->toggleTag(val);
        }

        return;
    }

    LightTableWindow* const ltw = dynamic_cast<LightTableWindow*>(w);

    if (ltw)
    {
        //kDebug() << "Handling by LightTableWindow";

        if (action->objectName().startsWith(d->ratingShortcutPrefix))
        {
            ltw->slotAssignRating(val);
        }
        else if (action->objectName().startsWith(d->pickShortcutPrefix))
        {
            ltw->slotAssignPickLabel(val);
        }
        else if (action->objectName().startsWith(d->colorShortcutPrefix))
        {
            ltw->slotAssignColorLabel(val);
        }
        else if (action->objectName().startsWith(d->tagShortcutPrefix))
        {
            ltw->toggleTag(val);
        }

        return;
    }

    SlideShow* const sld = dynamic_cast<SlideShow*>(w);

    if (sld)
    {
        //kDebug() << "Handling by SlideShow";

        if (action->objectName().startsWith(d->ratingShortcutPrefix))
        {
            sld->slotAssignRating(val);
        }
        else if (action->objectName().startsWith(d->pickShortcutPrefix))
        {
            sld->slotAssignPickLabel(val);
        }
        else if (action->objectName().startsWith(d->colorShortcutPrefix))
        {
            sld->slotAssignColorLabel(val);
        }
        else if (action->objectName().startsWith(d->tagShortcutPrefix))
        {
            sld->toggleTag(val);
        }

        return;
    }
}

// Special case with Slideshow which do not depand of database.

void TagsActionMngr::slotImageTagChanged(const ImageTagChangeset&)
{
    QWidget* const w     = kapp->activeWindow();
    SlideShow* const sld = dynamic_cast<SlideShow*>(w);

    if (sld)
    {
        KUrl url = sld->currentItem();
        ImageInfo info = ImageInfo::fromUrl(url);
        sld->updateTags(url, AlbumManager::instance()->tagNames(info.tagIds()));
    }
}

} // namespace Digikam
