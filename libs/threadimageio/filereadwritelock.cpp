/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-29
 * Description : Intra-process file i/o lock
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

#include "filereadwritelock.h"

// Qt includes

#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

// KDE includes

#include <kglobal.h>

// Local includes

#include "digikam_export.h"


namespace Digikam
{

class FileReadWriteLockPriv
{
public:
    FileReadWriteLockPriv(const QString& filePath)
        : filePath(filePath),
          ref(0), waitingReaders(0), waitingWriters(0), readers(0), writer(false)
    {
    }

    QString filePath;
    int     ref;
    int     waitingReaders;
    int     waitingWriters;
    int     readers;
    bool    writer;

    bool isFree() const
    {
        return !readers && !writer && !waitingReaders && !waitingWriters;
    }
};

typedef FileReadWriteLockPriv Entry;

class FileReadWriteLockStaticPrivate
{
public:

    QMutex         mutex;
    QWaitCondition readerWait;
    QWaitCondition writerWait;
    
    QHash<QString, Entry*> entries;

    Entry *entry(const QString& filePath);
    void drop(Entry* entry);

    void lockForRead(Entry* entry);
    void lockForWrite(Entry* entry);
    bool tryLockForRead(Entry* entry);
    bool tryLockForRead(Entry* entry, int timeout);
    bool tryLockForWrite(Entry* entry);
    bool tryLockForWrite(Entry* entry, int timeout);
    void unlock(Entry* entry);

    Entry *entryLockedForRead(const QString& filePath);
    Entry *entryLockedForWrite(const QString& filePath);
    void unlockAndDrop(Entry* entry);

private:

    Entry *entry_locked(const QString& filePath);
    void drop_locked(Entry* entry);
    void lockForRead_locked(Entry* entry);
    void lockForWrite_locked(Entry* entry);
    void unlock_locked(Entry* entry);
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
    lockForRead_locked(entry);
}

void FileReadWriteLockStaticPrivate::lockForRead_locked(Entry* entry)
{
    while (entry->writer)
    {
        entry->waitingReaders++;
        readerWait.wait(&mutex);
        entry->waitingReaders--;
    }
    entry->readers++;
}

void FileReadWriteLockStaticPrivate::lockForWrite(Entry* entry)
{
    QMutexLocker lock(&mutex);
    lockForWrite_locked(entry);
}

void FileReadWriteLockStaticPrivate::lockForWrite_locked(Entry* entry)
{
    while (entry->writer || entry->readers)
    {
        entry->waitingWriters++;
        writerWait.wait(&mutex);
        entry->waitingWriters--;
    }
    entry->writer = true;
}

bool FileReadWriteLockStaticPrivate::tryLockForRead(Entry* entry)
{
    QMutexLocker lock(&mutex);
    if (entry->writer)
    {
        return false;
    }
    entry->readers++;
    return true;
}

bool FileReadWriteLockStaticPrivate::tryLockForRead(Entry* entry, int timeout)
{
    QMutexLocker lock(&mutex);
    if (entry->writer)
    {
        entry->waitingReaders++;
        readerWait.wait(&mutex, timeout);
        entry->waitingReaders--;
    }
    if (entry->writer)
    {
        return false;
    }
    entry->readers++;
    return true;
}

bool FileReadWriteLockStaticPrivate::tryLockForWrite(Entry* entry)
{
    QMutexLocker lock(&mutex);
    if (entry->writer || entry->readers)
    {
        return false;
    }
    entry->writer = true;
    return true;
}

bool FileReadWriteLockStaticPrivate::tryLockForWrite(Entry* entry, int timeout)
{
    QMutexLocker lock(&mutex);
    if (entry->writer || entry->readers)
    {
        entry->waitingWriters++;
        writerWait.wait(&mutex, timeout);
        entry->waitingWriters--;
    }
    if (entry->writer || entry->readers)
    {
        return false;
    }
    entry->writer = true;
    return true;
}

void FileReadWriteLockStaticPrivate::unlock(Entry* entry)
{
    QMutexLocker lock(&mutex);
    unlock_locked(entry);
}

void FileReadWriteLockStaticPrivate::unlock_locked(Entry* entry)
{
    if (entry->writer)
    {
        entry->writer = false;
    }
    else
    {
        entry->readers--;
    }

    if (entry->waitingWriters)
    {
        if (!entry->readers)
        {
            // we must wake all as it is one wait condition for all entries
            writerWait.wakeAll();
        }
    }
    else if (entry->waitingReaders)
    {
        // we must wake all as it is one wait condition for all entries
        readerWait.wakeAll();
    }
}

// --- Combination methods ---

Entry *FileReadWriteLockStaticPrivate::entryLockedForRead(const QString& filePath)
{
    QMutexLocker lock(&mutex);
    Entry* e = entry_locked(filePath);
    lockForRead_locked(e);
    return e;
}

Entry *FileReadWriteLockStaticPrivate::entryLockedForWrite(const QString& filePath)
{
    QMutexLocker lock(&mutex);
    Entry* e = entry_locked(filePath);
    lockForWrite_locked(e);
    return e;
}

void FileReadWriteLockStaticPrivate::unlockAndDrop(Entry* entry)
{
    QMutexLocker lock(&mutex);
    unlock_locked(entry);
    drop_locked(entry);
}

K_GLOBAL_STATIC(FileReadWriteLockStaticPrivate, static_d)


// -------------------------------------


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
    static_d->lockForWrite(d);
}

FileWriteLocker::~FileWriteLocker()
{
    static_d->unlockAndDrop(d);
}

}

