/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-05
 * Description : file action manager task list
 *
 * Copyright (C) 2012 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

// KDE includes

#include <kdebug.h>

// Local includes

#include "fileactionimageinfolist.h"

namespace Digikam
{

void TwoProgressItemsContainer::createProgressItem(QAtomicPointer<ProgressItem>& ptr,
                                                   const QString& action, FileActionProgressItemCreator* creator)
{
    if (ptr)
    {
        return;
    }
    ProgressItem* item = creator->createProgressItem(action);
    
    if (!ptr.testAndSetOrdered(0, item))
    {
        delete item;
        return;
    }

    creator->addProgressItem(item);
}

void TwoProgressItemsContainer::checkFinish(QAtomicPointer<ProgressItem>& ptr)
{
    kDebug() << ptr->totalItems() << ptr->completedItems();
    if (ptr->totalCompleted())
    {
        ProgressItem* item = ptr;
        if (item && ptr.testAndSetOrdered(item, 0))
        {
            kDebug() << "Setting item complete";
            item->setComplete();
        }
    }
}

void FileActionProgressItemContainer::schedulingForDB(int numberOfInfos, const QString& action, FileActionProgressItemCreator* creator)
{
    kDebug() << numberOfInfos << action;
    createFirstItem(action, creator);
    firstItem->incTotalItems(numberOfInfos);
}

void FileActionProgressItemContainer::dbProcessed(int numberOfInfos)
{
    firstItem->advance(numberOfInfos);
}

void FileActionProgressItemContainer::dbFinished()
{
    checkFinish(firstItem);
    kDebug() << "checked db";
}

void FileActionProgressItemContainer::schedulingForWrite(int numberOfInfos, const QString& action, FileActionProgressItemCreator* creator)
{
    kDebug() << numberOfInfos << action;
    createSecondItem(action, creator);
    secondItem->incTotalItems(numberOfInfos);
}

void FileActionProgressItemContainer::written(int numberOfInfos)
{
    secondItem->advance(numberOfInfos);
}

void FileActionProgressItemContainer::finishedWriting()
{
    checkFinish(secondItem);
    kDebug() << "checked writing";
}

FileActionImageInfoList FileActionImageInfoList::create(const QList<ImageInfo>& infos)
{
    FileActionImageInfoList list;
    list = infos;
    list.container = new FileActionProgressItemContainer;
    return list;
}

FileActionImageInfoList FileActionImageInfoList::continueTask(const QList<ImageInfo>& infos, FileActionProgressItemContainer* container)
{
    FileActionImageInfoList list;
    list = infos;
    list.container = container;
    return list;
}


} // namespace Digikam

