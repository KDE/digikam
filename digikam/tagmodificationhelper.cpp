/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : helper class used to modify physical albums in views
 *
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

#include "tagmodificationhelper.moc"

// KDE includes
#include <kmessagebox.h>
#include <klocale.h>
#include <kapplication.h>

// Local includes
#include "album.h"
#include "albumdb.h"
#include "tageditdlg.h"
#include "statusprogressbar.h"
#include "albumlister.h"
#include "scancontroller.h"
#include "databasetransaction.h"
#include "metadatahub.h"
#include "imageinfo.h"

namespace Digikam
{

class TagModificationHelperPriv
{
public:
    QWidget *dialogParent;
};

TagModificationHelper::TagModificationHelper(QObject *parent, QWidget *dialogParent) :
    QObject(parent), d(new TagModificationHelperPriv)
{
    d->dialogParent = dialogParent;
}

TagModificationHelper::~TagModificationHelper()
{
    delete d;
}

void TagModificationHelper::slotTagNew(TAlbum *parent, const QString &title, const QString &iconName)
{

    QString editTitle = title;
    QString editIconName = iconName;

    if (title.isEmpty())
    {
        bool doCreate = TagEditDlg::tagCreate(d->dialogParent, parent, editTitle, editIconName);
        if(!doCreate)
        {
            return;
        }
    }

    QMap<QString, QString> errMap;
    AlbumList tList = TagEditDlg::createTAlbum(parent, editTitle, editIconName, errMap);
    TagEditDlg::showtagsListCreationError(d->dialogParent, errMap);

}

void TagModificationHelper::slotTagEdit(TAlbum *tag)
{

    if(!tag)
    {
        return;
    }

    QString title, icon;
    bool doEdit = TagEditDlg::tagEdit(d->dialogParent, tag, title, icon);
    if(!doEdit)
    {
        return;
    }

    if(tag->title() != title)
    {
        QString errMsg;
        if(!AlbumManager::instance()->renameTAlbum(tag, title, errMsg))
        {
            KMessageBox::error(0, errMsg);
        }
    }

    if(tag->icon() != icon)
    {
        QString errMsg;
        if (!AlbumManager::instance()->updateTAlbumIcon(tag, icon, 0, errMsg))
        {
            KMessageBox::error(0, errMsg);
        }
    }

}

void TagModificationHelper::slotTagDelete(TAlbum *tag)
{

    if (!tag || tag->isRoot())
    {
        return;
    }

    // find number of subtags
    int children = 0;
    AlbumIterator iter(tag);
    while(iter.current())
    {
        ++children;
        ++iter;
    }

    // ask for deletion of children
    if(children)
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

        if(result != KMessageBox::Continue)
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

    if(result == KMessageBox::Continue)
    {
        QString errMsg;
        if (!AlbumManager::instance()->deleteTAlbum(tag, errMsg))
        {
            KMessageBox::error(0, errMsg);
        }
    }
}

void TagModificationHelper::slotAssignTags(int tagId, const QList<int>& imageIDs)
{
    TAlbum *destAlbum = AlbumManager::instance()->findTAlbum(tagId);
    if (!destAlbum)
    {
        return;
    }

    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode,
                               i18n("Assigning image tags. Please wait..."));

    // TODO this code reaaaaaally looks like it should be encapsulated in a
    // method that is part of a model or something like that
    AlbumLister::instance()->blockSignals(true);
    ScanController::instance()->suspendCollectionScan();
    DatabaseTransaction transaction;
    MetadataHub         hub;
    int i=0;

    for (QList<int>::const_iterator it = imageIDs.constBegin(); it != imageIDs.constEnd(); ++it)
    {
        // create temporary ImageInfo object
        ImageInfo info(*it);

        hub.load(info);
        hub.setTag(destAlbum, true);

        QString filePath = info.filePath();
        hub.write(info, MetadataHub::PartialWrite);
        bool fileChanged = hub.write(filePath, MetadataHub::FullWriteIfChanged);
        if (fileChanged)
        {
            ScanController::instance()->scanFileDirectly(filePath);
        }

        emit signalProgressValue((int)((i++/(float)imageIDs.count())*100.0));
        kapp->processEvents();
    }
    ScanController::instance()->resumeCollectionScan();
    AlbumLister::instance()->blockSignals(false);

    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());

}

}
