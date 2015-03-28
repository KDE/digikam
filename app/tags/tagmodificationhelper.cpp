/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : helper class used to modify tag albums in views
 *
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2010-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "tagmodificationhelper.moc"

// Qt includes

#include <QAction>
#include <QDeclarativeContext>

// KDE includes

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "databasetransaction.h"
#include "imageinfo.h"
#include "metadatahub.h"
#include "scancontroller.h"
#include "statusprogressbar.h"
#include "tagsactionmngr.h"
#include "tagproperties.h"
#include "tageditdlg.h"
#include "facetags.h"

namespace Digikam
{

class TagModificationHelper::Private
{
public:

    Private()
    {
        parentTag    = 0;
        dialogParent = 0;
    }

    AlbumPointer<TAlbum>  parentTag;
    QWidget*              dialogParent;
};

TagModificationHelper::TagModificationHelper(QObject* const parent, QWidget* const dialogParent)
    : QObject(parent), d(new Private)
{
    d->dialogParent = dialogParent;
}

TagModificationHelper::~TagModificationHelper()
{
    delete d;
}

void TagModificationHelper::bindTag(QAction* action, TAlbum* album) const
{
    action->setData(QVariant::fromValue(AlbumPointer<TAlbum>(album)));
}

TAlbum* TagModificationHelper::boundTag(QObject* sender) const
{
    QAction* action = 0;

    if ( (action = qobject_cast<QAction*>(sender)) )
    {
        return action->data().value<AlbumPointer<TAlbum> >();
    }

    return 0;
}
void TagModificationHelper::bindMultipleTags(QAction* action, QList<TAlbum*> tags)
{
    action->setData(QVariant::fromValue(tags));
}

QList<TAlbum*> TagModificationHelper::boundMultipleTags(QObject* sender)
{
    QAction* action = 0;

    if ( (action = qobject_cast<QAction*>(sender)) )
    {
        return (action->data().value<QList<TAlbum*> >());
    }

    return QList<TAlbum*>();
}

TAlbum* TagModificationHelper::slotTagNew(TAlbum* parent, const QString& title, const QString& iconName)
{
    // ensure that there is a parent
    AlbumPointer<TAlbum> p(parent);

    if (!p)
    {
        p = AlbumManager::instance()->findTAlbum(0);

        if (!p)
        {
            kError() << "Could not find root tag album";
            return 0;
        }
    }

    QString      editTitle    = title;
    QString      editIconName = iconName;
    QKeySequence ks;

    if (title.isEmpty())
    {
        bool doCreate = TagEditDlg::tagCreate(d->dialogParent, p, editTitle, editIconName, ks);

        if (!doCreate || !p)
        {
            return 0;
        }
    }

    QMap<QString, QString> errMap;
    AlbumList tList = TagEditDlg::createTAlbum(p, editTitle, editIconName, ks, errMap);
    TagEditDlg::showtagsListCreationError(d->dialogParent, errMap);

    if (errMap.isEmpty() && !tList.isEmpty())
    {
        TAlbum* const tag = static_cast<TAlbum*>(tList.last());
        emit tagCreated(tag);
        return tag;
    }
    else
    {
        return 0;
    }
}

TAlbum* TagModificationHelper::slotTagNew()
{
    return slotTagNew(boundTag(sender()));
}

void TagModificationHelper::slotTagEdit(TAlbum* t)
{
    if (!t)
    {
        return;
    }

    AlbumPointer<TAlbum> tag(t);

    QString      title, icon;
    QKeySequence ks;

    bool doEdit = TagEditDlg::tagEdit(d->dialogParent, tag, title, icon, ks);

    if (!doEdit || !tag)
    {
        return;
    }

    if (tag && tag->title() != title)
    {
        QString errMsg;

        if (AlbumManager::instance()->renameTAlbum(tag, title, errMsg))
        {
            // TODO: make an option to edit the full name of a face tag
            if (FaceTags::isPerson(tag->id()))
            {
                TagProperties props(tag->id());
                props.setProperty(TagPropertyName::person(), title);
                props.setProperty(TagPropertyName::kfaceName(), title);
            }
        }
        else
        {
            KMessageBox::error(0, errMsg);
        }
    }

    if (tag && tag->icon() != icon)
    {
        QString errMsg;

        if (!AlbumManager::instance()->updateTAlbumIcon(tag, icon, 0, errMsg))
        {
            KMessageBox::error(0, errMsg);
        }
    }

    if (tag && tag->property(TagPropertyName::tagKeyboardShortcut()) != ks.toString())
    {
        TagsActionMngr::defaultManager()->updateTagShortcut(tag->id(), ks);
    }

    emit tagEdited(tag);
}

void TagModificationHelper::slotTagEdit()
{
    slotTagEdit(boundTag(sender()));
}

void TagModificationHelper::slotTagDelete(TAlbum* t)
{
    if (!t || t->isRoot())
    {
        return;
    }

    AlbumPointer<TAlbum> tag(t);

    // find number of subtags
    int children = 0;
    AlbumIterator iter(tag);

    while (iter.current())
    {
        ++children;
        ++iter;
    }

    // ask for deletion of children
    if (children)
    {
        int result = KMessageBox::warningContinueCancel(d->dialogParent,
                                                        i18np("Tag '%2' has one subtag. "
                                                              "Deleting this will also delete "
                                                              "the subtag. "
                                                              "Do you want to continue?",
                                                              "Tag '%2' has %1 subtags. "
                                                              "Deleting this will also delete "
                                                              "the subtags. "
                                                              "Do you want to continue?",
                                                              children,
                                                              tag->title()));

        if (result != KMessageBox::Continue || !tag)
        {
            return;
        }
    }

    QString message;
    QList<qlonglong> assignedItems = DatabaseAccess().db()->getItemIDsInTag(tag->id());

    if (!assignedItems.isEmpty())
    {
        message = i18np("Tag '%2' is assigned to one item. "
                        "Do you want to continue?",
                        "Tag '%2' is assigned to %1 items. "
                        "Do you want to continue?",
                        assignedItems.count(), tag->title());
    }
    else
    {
        message = i18n("Delete '%1' tag?", tag->title());
    }

    int result = KMessageBox::warningContinueCancel(0, message,
                                                    i18n("Delete Tag"),
                                                    KGuiItem(i18n("Delete"),
                                                             "edit-delete"));

    if (result == KMessageBox::Continue && tag)
    {
        emit aboutToDeleteTag(tag);
        QString errMsg;

        if (!AlbumManager::instance()->deleteTAlbum(tag, errMsg))
        {
            KMessageBox::error(0, errMsg);
        }
    }
}

void TagModificationHelper::slotTagDelete()
{
    slotTagDelete(boundTag(sender()));
}

void TagModificationHelper::slotMultipleTagDel(QList<TAlbum* >& tags)
{
    QString tagWithChildrens;
    QString tagWithImages;
    QMultiMap<int, TAlbum*> sortedTags;

    foreach(TAlbum* const t, tags)
    {

        if (!t || t->isRoot())
        {
            continue;
        }

        AlbumPointer<TAlbum> tag(t);

        // find number of subtags
        int children = 0;
        AlbumIterator iter(tag);

        while (iter.current())
        {
            ++children;
            ++iter;
        }

        if(children)
            tagWithChildrens.append(tag->title() + QString(" "));

        QList<qlonglong> assignedItems = DatabaseAccess().db()->getItemIDsInTag(tag->id());

        if(!assignedItems.isEmpty())
            tagWithImages.append(tag->title() + QString(" "));

        /**
         * Tags must be deleted from children to parents, if we don't want
         * to step on invalid index. Use QMultiMap to order them by distance
         * to root tag
         */

        Album* parent = t;
        int depth = 0;

        while(!parent->isRoot())
        {
            parent = parent->parent();
            depth++;
        }

        sortedTags.insert(depth,tag);

    }

    // ask for deletion of children

    if (!tagWithChildrens.isEmpty())
    {
        int result = KMessageBox::warningContinueCancel(0,
                                                        i18n("Tags '%1' have one or more subtags. "
                                                             "Deleting them will also delete "
                                                             "the subtags. "
                                                             "Do you want to continue?",
                                                             tagWithChildrens));

        if (result != KMessageBox::Continue)
        {
            return;
        }
    }

    QString message;

    if (!tagWithImages.isEmpty())
    {
        message = i18n("Tags '%1' are assigned to one or more items. "
                        "Do you want to continue?",
                        tagWithImages);
    }
    else
    {
        message = i18n("Delete '%1' tag(s)?", tagWithImages);
    }

    int result = KMessageBox::warningContinueCancel(0, message,
                                                    i18n("Delete Tag"),
                                                    KGuiItem(i18n("Delete"),
                                                            "edit-delete"));

    if (result == KMessageBox::Continue)
    {
        QMultiMap<int, TAlbum*>::iterator it;
        /**
         * QMultimap doesn't provide reverse iterator, -1 is required
         * because end() points after the last element
         */
        for(it = sortedTags.end()-1; it != sortedTags.begin()-1; --it)
        {
            emit aboutToDeleteTag(it.value());
            QString errMsg;

            if (!AlbumManager::instance()->deleteTAlbum(it.value(), errMsg))
            {
                KMessageBox::error(0, errMsg);
            }
        }
    }
}

void TagModificationHelper::slotMultipleTagDel()
{
    QList<TAlbum*> lst = boundMultipleTags(sender());
    kDebug() << lst.size();
    slotMultipleTagDel(lst);
}

} // namespace Digikam
