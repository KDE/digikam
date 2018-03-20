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

#ifndef FILEACTIONIMAGEINFOLIST_H
#define FILEACTIONIMAGEINFOLIST_H

// Qt includes

#include <QAtomicPointer>
#include <QList>
#include <QExplicitlySharedDataPointer>

// Local includes

#include "imageinfo.h"
#include "progressmanager.h"

#include <QDebug>


namespace Digikam
{

class FileActionProgressItemCreator
{
public:

    virtual ~FileActionProgressItemCreator() {}
    virtual ProgressItem* createProgressItem(const QString& action) const = 0;
    virtual void addProgressItem(ProgressItem* const item) = 0;
};

// -------------------------------------------------------------------------------------------------------------------

class TwoProgressItemsContainer :  public QSharedData
{
protected:

    QAtomicPointer<ProgressItem> firstItem;
    QAtomicPointer<ProgressItem> secondItem;

    // Note: It is currently not safe to schedule after the framework had a chance to
    // advance all already scheduled items. For this, we'd need to add a mechanism (flag to block completion?)

    void scheduleOnProgressItem(QAtomicPointer<ProgressItem>& ptr, int total,
                                const QString& action, FileActionProgressItemCreator* const creator);
    void advance(QAtomicPointer<ProgressItem>& ptr, int n);
};

// -------------------------------------------------------------------------------------------------------------------

class FileActionProgressItemContainer :public QObject, public TwoProgressItemsContainer
{
    Q_OBJECT
public:
    FileActionProgressItemContainer();
    void schedulingForDB(int numberOfInfos, const QString& action, FileActionProgressItemCreator* const creator);
    void dbProcessed(int numberOfInfos);
    void dbFinished();
    void schedulingForWrite(int numberOfInfos, const QString& action, FileActionProgressItemCreator* const creator);
    void written(int numberOfInfos);
    void finishedWriting();

Q_SIGNALS:
    void signalWrittingDone();

};

// -------------------------------------------------------------------------------------------------------------------

class FileActionImageInfoList : public QList<ImageInfo>
{
public:

    FileActionImageInfoList() {}

    FileActionImageInfoList(const FileActionImageInfoList& copy) : QList(copy)
    {
        this->container = copy.container;
    }

    ~FileActionImageInfoList() {}

public:

    static FileActionImageInfoList create(const QList<ImageInfo>& list);
    static FileActionImageInfoList continueTask(const QList<ImageInfo>& list, FileActionProgressItemContainer* const container);

    FileActionProgressItemContainer* progress() const { return container.data(); }

    /// before sending to db worker
    void schedulingForDB(int numberOfInfos, const QString& action, FileActionProgressItemCreator* const creator)
        { progress()->schedulingForDB(numberOfInfos, action, creator); }
    void schedulingForDB(const QString& action, FileActionProgressItemCreator* const creator)
        { schedulingForDB(size(), action, creator); }

    /// db worker progress info
    void dbProcessedOne()               { dbProcessed(1);                         }
    void dbProcessed(int numberOfInfos) { progress()->dbProcessed(numberOfInfos); }
    void dbFinished()                   { progress()->dbFinished();               }

    /// db worker calls this before sending to file worker
    void schedulingForWrite(int numberOfInfos, const QString& action, FileActionProgressItemCreator* const creator)
        { progress()->schedulingForWrite(numberOfInfos, action, creator); }
    void schedulingForWrite(const QString& action, FileActionProgressItemCreator* const creator)
        { schedulingForWrite(size(), action, creator); }

    /// file worker calls this when finished
    void writtenToOne()             { written(1);                         }
    void written(int numberOfInfos) { progress()->written(numberOfInfos); }
    void finishedWriting()          { progress()->finishedWriting();      }

    QExplicitlySharedDataPointer<FileActionProgressItemContainer> container;
private:

    FileActionImageInfoList(const QList<ImageInfo>& list) : QList<ImageInfo>(list) {}

};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::FileActionImageInfoList)

#endif // FILEACTIONIMAGEINFOLIST_H
