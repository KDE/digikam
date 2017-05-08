/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-29
 * Description : Intra-process file i/o lock
 *
 * Copyright (C) 2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * Parts of this file are based on qreadwritelock.cpp, LGPL,
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

#include "filereadwritelock.h"

// Qt includes

#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QWaitCondition>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class FileReadWriteLockPriv
{
public:

    explicit FileReadWriteLockPriv(const QString& filePath)
        : filePath(filePath),
          ref(0),
          waitingReaders(0),
          waitingWriters(0),
          accessCount(0),
          writer(0)
    {
    }

    bool isFree() const
    {
        return readers.isEmpty() &&
               !writer           &&
               !waitingReaders   &&
               !waitingWriters;
    }

public:

    QString                filePath;
    int                    ref;
    int                    waitingReaders;
    int                    waitingWriters;

    int                    accessCount;
    Qt::HANDLE             writer;
    QHash<Qt::HANDLE, int> readers;
};

typedef FileReadWriteLockPriv Entry;

class FileReadWriteLockStaticPrivate
{
public:

    QMutex                 mutex;
    QWaitCondition         readerWait;
    QWaitCondition         writerWait;

    QMutex                 tempFileMutex;

    QHash<QString, Entry*> entries;

public:

    Entry* entry(const QString& filePath);
    void   drop(Entry* entry);

    void   lockForRead(Entry* entry);
    void   lockForWrite(Entry* entry);
    bool   tryLockForRead(Entry* entry);
    bool   tryLockForRead(Entry* entry, int timeout);
    bool   tryLockForWrite(Entry* entry);
    bool   tryLockForWrite(Entry* entry, int timeout);
    void   unlock(Entry* entry);

    Entry* entryLockedForRead(const QString& filePath);
    Entry* entryLockedForWrite(const QString& filePath);
    void   unlockAndDrop(Entry* entry);

private:

    Entry* entry_locked(const QString& filePath);
    void   drop_locked(Entry* entry);
    bool   lockForRead_locked(Entry* entry, int mode, int timeout);
    bool   lockForWrite_locked(Entry* entry, int mode, int timeout);
    void   unlock_locked(Entry* entry);
};

// --- Entry allocation ---

Entry* FileReadWriteLockStaticPrivate::entry(const QString& filePath)
{
    QMutexLocker lock(&mutex);
    return entry_locked(filePath);
}

Entry* FileReadWriteLockStaticPrivate::entry_locked(const QString& filePath)
{
    QHash<QString, Entry*>::iterator it = entries.find(filePath);
    if (it == entries.end())
    {
        it = entries.insert(filePath, new Entry(filePath));
    }
    (*it)->ref++;
    return *it;
}

void FileReadWriteLockStaticPrivate::drop(Entry* entry)
{
    QMutexLocker lock(&mutex);
    drop_locked(entry);
}

void FileReadWriteLockStaticPrivate::drop_locked(Entry* entry)
{
    entry->ref--;
    if (entry->ref == 0 && entry->isFree())
    {
        entries.remove(entry->filePath);
        delete entry;
    }
}

// --- locking implementation ---

void FileReadWriteLockStaticPrivate::lockForRead(Entry* entry)
{
    QMutexLocker lock(&mutex);
    lockForRead_locked(entry, 0, 0);
}

bool FileReadWriteLockStaticPrivate::tryLockForRead(Entry* entry)
{
    QMutexLocker lock(&mutex);
    return lockForRead_locked(entry, 1, 0);
}

bool FileReadWriteLockStaticPrivate::tryLockForRead(Entry* entry, int timeout)
{
    QMutexLocker lock(&mutex);
    return lockForRead_locked(entry, 2, timeout);
}

bool FileReadWriteLockStaticPrivate::lockForRead_locked(Entry* entry, int mode, int timeout)
{
    Qt::HANDLE self = QThread::currentThreadId();

    // already recursively write-locked by this thread?
    if (entry->writer == self)
    {
        // If we already have the write lock recursively, just add another write lock instead of the read lock.
        // This situation is clean and all right.
        --entry->accessCount;
        return true;
    }
    // recursive read lock by this thread?
    QHash<Qt::HANDLE, int>::iterator it = entry->readers.find(self);
    if (it != entry->readers.end())
    {
        ++it.value();
        ++entry->accessCount;
        return true;
    }

    if (mode == 1)
    {
        // tryLock
        if (entry->accessCount < 0)
        {
            return false;
        }
    }
    else
    {
        while (entry->accessCount < 0 || entry->waitingWriters)
        {
            if (mode == 2)
            {
                // tryLock with timeout
                ++entry->waitingReaders;
                bool success = readerWait.wait(&mutex, timeout);
                --entry->waitingReaders;
                if (!success)
                {
                    return false;
                }
            }
            else
            {
                // lock
                ++entry->waitingReaders;
                readerWait.wait(&mutex);
                --entry->waitingReaders;
            }
        }
    }

    entry->readers.insert(self, 1);
    ++entry->accessCount;
    return true;
}

void FileReadWriteLockStaticPrivate::lockForWrite(Entry* entry)
{
    QMutexLocker lock(&mutex);
    lockForWrite_locked(entry, 0, 0);
}

bool FileReadWriteLockStaticPrivate::tryLockForWrite(Entry* entry)
{
    QMutexLocker lock(&mutex);
    return lockForWrite_locked(entry, 1, 0);
}

bool FileReadWriteLockStaticPrivate::tryLockForWrite(Entry* entry, int timeout)
{
    QMutexLocker lock(&mutex);
    return lockForRead_locked(entry, 2, timeout);
}

bool FileReadWriteLockStaticPrivate::lockForWrite_locked(Entry* entry, int mode, int timeout)
{
    Qt::HANDLE self = QThread::currentThreadId();

    // recursive write-lock by this thread?
    if (entry->writer == self)
    {
        --entry->accessCount;
        return true;
    }

    // recursive read lock by this thread?
    QHash<Qt::HANDLE, int>::iterator it = entry->readers.find(self);
    int recursiveReadLockCount = 0;
    if (it != entry->readers.end())
    {
        // We could deadlock, or promote the read locks to write locks
        qCWarning(DIGIKAM_GENERAL_LOG) << "Locking for write, recursively locked for read: Promoting existing read locks to write locks! "
                   << "Avoid this situation.";
        // The lock was locked for read it.value() times by this thread recursively
        recursiveReadLockCount = it.value();
        entry->accessCount    -= it.value();
        entry->readers.erase(it);
    }

    while (entry->accessCount != 0)
    {
        if (mode == 1)
        {
            // tryLock
            return false;
        }
        else if (mode == 2)
        {
            // tryLock with timeout
            entry->waitingWriters++;
            bool success = writerWait.wait(&mutex, timeout);
            entry->waitingWriters--;
            if (!success)
            {
                return false;
            }
        }
        else
        {
            // lock
            entry->waitingWriters++;
            writerWait.wait(&mutex);
            entry->waitingWriters--;
        }
    }

    entry->writer = self;
    --entry->accessCount;
    // if we had recursive read locks, they are now promoted to write locks
    entry->accessCount -= recursiveReadLockCount;
    return true;
}

void FileReadWriteLockStaticPrivate::unlock(Entry* entry)
{
    QMutexLocker lock(&mutex);
    unlock_locked(entry);
}

void FileReadWriteLockStaticPrivate::unlock_locked(Entry* entry)
{
    bool unlocked = false;
    if (entry->accessCount > 0)
    {
        // releasing a read lock
        Qt::HANDLE self = QThread::currentThreadId();
        QHash<Qt::HANDLE, int>::iterator it = entry->readers.find(self);
        if (it != entry->readers.end())
        {
            if (--it.value() <= 0)
            {
                entry->readers.erase(it);
            }
        }

        unlocked = --entry->accessCount == 0;
    }
    else if (entry->accessCount < 0 && ++entry->accessCount == 0)
    {
        // released a write lock
        unlocked = true;
        entry->writer = 0;
    }

    if (unlocked)
    {
        if (entry->waitingWriters)
        {
            // we must wake all as it is one wait condition for all entries
            writerWait.wakeAll();
        }
        else if (entry->waitingReaders)
        {
            readerWait.wakeAll();
        }
    }

}

// --- Combination methods ---

Entry* FileReadWriteLockStaticPrivate::entryLockedForRead(const QString& filePath)
{
    QMutexLocker lock(&mutex);
    Entry* e = entry_locked(filePath);
    lockForRead_locked(e, 0, 0);
    return e;
}

Entry* FileReadWriteLockStaticPrivate::entryLockedForWrite(const QString& filePath)
{
    QMutexLocker lock(&mutex);
    Entry* e = entry_locked(filePath);
    lockForWrite_locked(e, 0, 0);
    return e;
}

void FileReadWriteLockStaticPrivate::unlockAndDrop(Entry* entry)
{
    QMutexLocker lock(&mutex);
    unlock_locked(entry);
    drop_locked(entry);
}

Q_GLOBAL_STATIC(FileReadWriteLockStaticPrivate, static_d)

// -------------------------------------------------------------------------

FileReadWriteLockKey::FileReadWriteLockKey(const QString& filePath)
    : d(static_d->entry(filePath))
{
}

FileReadWriteLockKey::~FileReadWriteLockKey()
{
    static_d->drop(d);
}

void FileReadWriteLockKey::lockForRead()
{
    static_d->lockForRead(d);
}

void FileReadWriteLockKey::lockForWrite()
{
    static_d->lockForWrite(d);
}

bool FileReadWriteLockKey::tryLockForRead()
{
    return static_d->tryLockForRead(d);
}

bool FileReadWriteLockKey::tryLockForRead(int timeout)
{
    return static_d->tryLockForRead(d, timeout);
}

bool FileReadWriteLockKey::tryLockForWrite()
{
    return static_d->tryLockForWrite(d);
}

bool FileReadWriteLockKey::tryLockForWrite(int timeout)
{
    return static_d->tryLockForWrite(d, timeout);
}

void FileReadWriteLockKey::unlock()
{
    return static_d->unlock(d);
}

// -------------------------------------------------------------------------

FileReadLocker::FileReadLocker(const QString& filePath)
    : d(static_d->entryLockedForRead(filePath))
{
}

FileReadLocker::~FileReadLocker()
{
    static_d->unlockAndDrop(d);
}

FileWriteLocker::FileWriteLocker(const QString& filePath)
    : d(static_d->entryLockedForWrite(filePath))
{
}

FileWriteLocker::~FileWriteLocker()
{
    static_d->unlockAndDrop(d);
}

// -------------------------------------------------------------------------

SafeTemporaryFile::SafeTemporaryFile()
{
}

SafeTemporaryFile::SafeTemporaryFile(const QString& templ)
    : QTemporaryFile(templ)
{
}

bool SafeTemporaryFile::open(QIODevice::OpenMode mode)
{
    QMutexLocker lock(&static_d->tempFileMutex);
    return QTemporaryFile::open(mode);
}

} // namespace Digikam
