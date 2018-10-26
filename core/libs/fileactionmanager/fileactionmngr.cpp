/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-05
 * Description : file action manager
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "fileactionmngr.h"
#include "fileactionmngr_p.h"

// Qt includes

#include <QPointer>
#include <QProgressDialog>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "applicationsettings.h"
#include "iteminfotasksplitter.h"
#include "loadingcacheinterface.h"
#include "metadatahub.h"
#include "metaenginesettings.h"
#include "thumbnailloadthread.h"
#include "disjointmetadata.h"

namespace Digikam
{

class Q_DECL_HIDDEN FileActionMngrCreator
{
public:

    FileActionMngr object;
};

Q_GLOBAL_STATIC(FileActionMngrCreator, metadataManagercreator)

FileActionMngr* FileActionMngr::instance()
{
    return &metadataManagercreator->object;
}

FileActionMngr::FileActionMngr()
    : d(new Private(this))
{

    connect(d->fileWorker, SIGNAL(imageChangeFailed(QString,QStringList)),
            this, SIGNAL(signalImageChangeFailed(QString,QStringList)));
}

FileActionMngr::~FileActionMngr()
{
    shutDown();
    delete d;
}

bool FileActionMngr::requestShutDown()
{
    if (!isActive())
    {
        shutDown();
        return true;
    }

    QPointer<QProgressDialog> dialog = new QProgressDialog;
    dialog->setMinimum(0);
    dialog->setMaximum(0);
    dialog->setMinimumDuration(100);
    dialog->setLabelText(i18nc("@label", "Finishing tasks"));

    connect(d, SIGNAL(signalTasksFinished()),
            dialog, SLOT(accept()));

    dialog->exec();
    // Either, we finished and all is fine, or the user cancelled and we kill
    shutDown();
    return true;
}

void FileActionMngr::shutDown()
{
    d->dbWorker->deactivate();
    d->fileWorker->deactivate();
    d->dbWorker->wait();
    d->fileWorker->wait();
}

bool FileActionMngr::isActive()
{
    return d->isActive();
}

void FileActionMngr::assignTags(const QList<qlonglong>& ids, const QList<int>& tagIDs)
{
    assignTags(ItemInfoList(ids), tagIDs);
}

void FileActionMngr::assignTag(const ItemInfo& info, int tagID)
{
    assignTags(QList<ItemInfo>() << info, QList<int>() << tagID);
}

void FileActionMngr::assignTag(const QList<ItemInfo>& infos, int tagID)
{
    assignTags(infos, QList<int>() << tagID);
}

void FileActionMngr::assignTags(const ItemInfo& info, const QList<int>& tagIDs)
{
    assignTags(QList<ItemInfo>() << info, tagIDs);
}

void FileActionMngr::assignTags(const QList<ItemInfo>& infos, const QList<int>& tagIDs)
{
    FileActionItemInfoList taskList = FileActionItemInfoList::create(infos);
    taskList.schedulingForDB(i18n("Assigning image tags"), d->dbProgressCreator());
    d->assignTags(taskList, tagIDs);
}

void FileActionMngr::removeTag(const ItemInfo& info, int tagID)
{
    removeTags(QList<ItemInfo>() << info, QList<int>() << tagID);
}

void FileActionMngr::removeTag(const QList<ItemInfo>& infos, int tagID)
{
    removeTags(infos, QList<int>() << tagID);
}

void FileActionMngr::removeTags(const ItemInfo& info, const QList<int>& tagIDs)
{
    removeTags(QList<ItemInfo>() << info, tagIDs);
}

void FileActionMngr::removeTags(const QList<ItemInfo>& infos, const QList<int>& tagIDs)
{
    FileActionItemInfoList taskList = FileActionItemInfoList::create(infos);
    taskList.schedulingForDB(i18n("Removing image tags"), d->dbProgressCreator());
    d->removeTags(taskList, tagIDs);
}

void FileActionMngr::assignPickLabel(const ItemInfo& info, int pickId)
{
    assignPickLabel(QList<ItemInfo>() << info, pickId);
}

void FileActionMngr::assignColorLabel(const ItemInfo& info, int colorId)
{
    assignColorLabel(QList<ItemInfo>() << info, colorId);
}

void FileActionMngr::assignPickLabel(const QList<ItemInfo>& infos, int pickId)
{
    FileActionItemInfoList taskList = FileActionItemInfoList::create(infos);
    taskList.schedulingForDB(i18n("Assigning image pick label"), d->dbProgressCreator());
    d->assignPickLabel(taskList, pickId);
}

void FileActionMngr::assignColorLabel(const QList<ItemInfo>& infos, int colorId)
{
    FileActionItemInfoList taskList = FileActionItemInfoList::create(infos);
    taskList.schedulingForDB(i18n("Assigning image color label"), d->dbProgressCreator());
    d->assignColorLabel(taskList, colorId);
}

void FileActionMngr::assignRating(const ItemInfo& info, int rating)
{
    assignRating(QList<ItemInfo>() << info, rating);
}

void FileActionMngr::assignRating(const QList<ItemInfo>& infos, int rating)
{
    FileActionItemInfoList taskList = FileActionItemInfoList::create(infos);
    taskList.schedulingForDB(i18n("Assigning image ratings"), d->dbProgressCreator());
    d->assignRating(taskList, rating);
}

void FileActionMngr::addToGroup(const ItemInfo& pick, const QList<ItemInfo>& infos)
{
    FileActionItemInfoList taskList = FileActionItemInfoList::create(infos);
    taskList.schedulingForDB(i18n("Editing group"), d->dbProgressCreator());
    d->editGroup(AddToGroup, pick, taskList);
}

void FileActionMngr::removeFromGroup(const ItemInfo& info)
{
    removeFromGroup(QList<ItemInfo>() << info);
}

void FileActionMngr::removeFromGroup(const QList<ItemInfo>& infos)
{
    FileActionItemInfoList taskList = FileActionItemInfoList::create(infos);
    taskList.schedulingForDB(i18n("Editing group"), d->dbProgressCreator());
    d->editGroup(RemoveFromGroup, ItemInfo(), taskList);
}

void FileActionMngr::ungroup(const ItemInfo& info)
{
    ungroup(QList<ItemInfo>() << info);
}

void FileActionMngr::ungroup(const QList<ItemInfo>& infos)
{
    FileActionItemInfoList taskList = FileActionItemInfoList::create(infos);
    taskList.schedulingForDB(i18n("Editing group"), d->dbProgressCreator());
    d->editGroup(Ungroup, ItemInfo(), taskList);
}

void FileActionMngr::setExifOrientation(const QList<ItemInfo>& infos, int orientation)
{
    FileActionItemInfoList taskList = FileActionItemInfoList::create(infos);
    taskList.schedulingForDB(i18n("Updating orientation in database"), d->dbProgressCreator());
    d->setExifOrientation(taskList, orientation);
}

//void FileActionMngr::applyMetadata(const QList<ItemInfo>& infos, const MetadataHub& hub)
//{
//    FileActionItemInfoList taskList = FileActionItemInfoList::create(infos);
//    taskList.schedulingForDB(i18n("Applying metadata"), d->dbProgressCreator());
//    // create hub parent-less, we will need to clone() it in a thread,
//    // and that does not work with parent in a different thread
////    d->applyMetadata(taskList, new MetadataHubOnTheRoad(hub));
//}

void FileActionMngr::applyMetadata(const QList<ItemInfo>& infos, const DisjointMetadata &hub)
{
    FileActionItemInfoList taskList = FileActionItemInfoList::create(infos);
    taskList.schedulingForDB(i18n("Applying metadata"), d->dbProgressCreator());
    d->applyMetadata(taskList, new DisjointMetadata(hub));
}

void FileActionMngr::applyMetadata(const QList<ItemInfo>& infos, DisjointMetadata* hub)
{
//    if (hub->parent())
//    {
//        qCDebug(DIGIKAM_GENERAL_LOG) << "MetadataHubOnTheRoad object must not have a QObject parent";
//        delete hub;
//        return;
//    }

    FileActionItemInfoList taskList = FileActionItemInfoList::create(infos);
    taskList.schedulingForDB(i18n("Applying metadata"), d->dbProgressCreator());
    d->applyMetadata(taskList, hub);
}

void FileActionMngr::transform(const QList<ItemInfo>& infos, MetaEngineRotation::TransformationAction action)
{
    FileActionItemInfoList taskList = FileActionItemInfoList::create(infos);
    taskList.schedulingForWrite(i18n("Rotating images"), d->fileProgressCreator());

    for (ItemInfoTaskSplitter splitter(taskList); splitter.hasNext();)
    {
        d->transform(splitter.next(), action);
    }
}

void FileActionMngr::copyAttributes(const ItemInfo& source, const QString& derivedPath)
{
    copyAttributes(source, QStringList() << derivedPath);
}

void FileActionMngr::copyAttributes(const ItemInfo& source, const QStringList& derivedPaths)
{
    FileActionItemInfoList taskList = FileActionItemInfoList::create(QList<ItemInfo>() << source);
    taskList.schedulingForDB(i18n("Copying attributes"), d->dbProgressCreator());
    d->copyAttributes(taskList, derivedPaths);
}

} // namespace Digikam
