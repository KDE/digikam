/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-05
 * Description : file action manager task list
 *
 * Copyright (C) 2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Local includes

#include "fileactionimageinfolist.h"
#include "digikam_debug.h"
#include "progressmanager.h"

namespace Digikam
{

void TwoProgressItemsContainer::scheduleOnProgressItem(QAtomicPointer<ProgressItem>& ptr, int total,
                                                       const QString& action, FileActionProgressItemCreator* const creator)
{
    if (total <= 0)
    {
        return;
    }

    if (!ptr)
    {
        ProgressItem* const item = creator->createProgressItem(action);

        if (ptr.testAndSetOrdered(0, item))
        {
            creator->addProgressItem(item);
        }
        else
        {
            delete item;
        }
    }

    ptr.load()->incTotalItems(total);
}

void TwoProgressItemsContainer::advance(QAtomicPointer<ProgressItem>& ptr, int n)
{
    if (ptr.load()->advance(n))
    {
        ProgressItem* const item = ptr;

        if (item && ptr.testAndSetOrdered(item, 0))
        {
            item->setComplete();
        }
    }
}

FileActionProgressItemContainer::FileActionProgressItemContainer()
{

}

void FileActionProgressItemContainer::schedulingForDB(int numberOfInfos, const QString& action,
                                                      FileActionProgressItemCreator* const creator)
{
    scheduleOnProgressItem(firstItem, numberOfInfos, action, creator);
}

void FileActionProgressItemContainer::dbProcessed(int numberOfInfos)
{
    advance(firstItem, numberOfInfos);
}

void FileActionProgressItemContainer::dbFinished()
{
    //checkFinish(firstItem);
}

void FileActionProgressItemContainer::schedulingForWrite(int numberOfInfos, const QString& action,
                                                         FileActionProgressItemCreator* const creator)
{
    scheduleOnProgressItem(secondItem, numberOfInfos, action, creator);
    connect(secondItem, SIGNAL(progressItemCompleted(ProgressItem*)),
            this, SIGNAL(signalWrittingDone()));
}

void FileActionProgressItemContainer::written(int numberOfInfos)
{
    advance(secondItem, numberOfInfos);
}

void FileActionProgressItemContainer::finishedWriting()
{
//    checkFinish(secondItem);
}

FileActionImageInfoList FileActionImageInfoList::create(const QList<ImageInfo>& infos)
{
    FileActionImageInfoList list;
    list           = infos;
    list.container = new FileActionProgressItemContainer;
    return list;
}

FileActionImageInfoList FileActionImageInfoList::continueTask(const QList<ImageInfo>& infos, FileActionProgressItemContainer* const container)
{
    FileActionImageInfoList list;
    list           = infos;
    list.container = container;
    return list;
}

} // namespace Digikam
