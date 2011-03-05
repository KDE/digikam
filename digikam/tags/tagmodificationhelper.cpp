/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : helper class used to modify tag albums in views
 *
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2010-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "tageditdlg.h"

namespace Digikam
{

class TagModificationHelper::TagModificationHelperPriv
{
public:

    TagModificationHelperPriv()
    {
        parentTag    = 0;
        dialogParent = 0;
    }

    AlbumPointer<TAlbum>  parentTag;
    QWidget*              dialogParent;
};

TagModificationHelper::TagModificationHelper(QObject* parent, QWidget* dialogParent)
    : QObject(parent), d(new TagModificationHelperPriv)
{
    d->dialogParent = dialogParent;
}

TagModificationHelper::~TagModificationHelper()
{
    delete d;
}

void TagModificationHelper::setParentTag(TAlbum* parent)
{
    d->parentTag = parent;
}

TAlbum* TagModificationHelper::slotTagNew(TAlbum* parent, const QString& title, const QString& iconName)
{
    // ensure that there is a parent
    TAlbum* p = parent;

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
        if (!doCreate)
        {
            return 0;
        }
    }

    QMap<QString, QString> errMap;
    AlbumList tList = TagEditDlg::createTAlbum(p, editTitle, editIconName, ks, errMap);
    TagEditDlg::showtagsListCreationError(d->dialogParent, errMap);

    if (errMap.isEmpty() && !tList.isEmpty())
    {
        TAlbum* tag = static_cast<TAlbum*>(tList.last());
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
    if (d->parentTag)
    {
        return slotTagNew(d->parentTag);
    }
    else
    {
        kWarning() << "Tried to create a new tag but no parent tag was given";
        return 0;
    }
}

void TagModificationHelper::slotTagEdit(TAlbum* tag)
{
    if (!tag)
    {
        return;
    }

    QString      title, icon;
    QKeySequence ks;

    bool doEdit = TagEditDlg::tagEdit(d->dialogParent, tag, title, icon, ks);
    if (!doEdit)
    {
        return;
    }

    if (tag->title() != title)
    {
        QString errMsg;

        if (!AlbumManager::instance()->renameTAlbum(tag, title, errMsg))
        {
            KMessageBox::error(0, errMsg);
        }
    }

    if (tag->icon() != icon)
    {
        QString errMsg;

        if (!AlbumManager::instance()->updateTAlbumIcon(tag, icon, 0, errMsg))
        {
            KMessageBox::error(0, errMsg);
        }
    }

    if (tag->property(TagPropertyName::tagKeyboardShortcut()) != ks.toString())
    {
        TagsActionMngr::defaultManager()->updateTagShortcut(tag->id(), ks);
    }

    emit tagEdited(tag);
}

void TagModificationHelper::slotTagEdit()
{
    if (d->parentTag)
    {
        slotTagEdit(d->parentTag);
    }
}

void TagModificationHelper::slotTagDelete(TAlbum* tag)
{
    if (!tag || tag->isRoot())
    {
        return;
    }

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

        if (result != KMessageBox::Continue)
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

    if (result == KMessageBox::Continue)
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
    if (d->parentTag)
    {
        slotTagDelete(d->parentTag);
    }
}

} // namespace Digikam
