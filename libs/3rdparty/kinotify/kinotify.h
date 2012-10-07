/* This file is part of the KDE libraries
   Copyright (C) 2007-2010 Sebastian Trueg <trueg@kde.org>
   Copyright 2011 Marcel Wiesweg <marcel dot wiesweg at gmx dot de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _KINOTIFY_H_
#define _KINOTIFY_H_

#include <QtCore/QObject>
#include <QtCore/QFlags>

#include "digikam_export.h"

namespace Digikam
{

/**
 * A simple wrapper around inotify which only allows
 * to add folders recursively.
 *
 * Warning: moving of top-level folders is not supported and
 * results in undefined behaviour.
 */
class DIGIKAM_EXPORT KInotify : public QObject
{
    Q_OBJECT

public:

    explicit KInotify( QObject* parent = 0 );
    virtual ~KInotify();

    /**
     * Inotify events that can occur. Use with addWatch
     * to define the events that should be watched.
     *
     * These flags correspond to the native Linux inotify flags.
     */
    enum WatchEvent
    {
        EventAccess          = 0x00000001, /**< File was accessed (read, compare inotify's IN_ACCESS) */
        EventAttributeChange = 0x00000004, /**< Metadata changed (permissions, timestamps, extended attributes, etc., compare inotify's IN_ATTRIB) */
        EventCloseWrite      = 0x00000008, /**< File opened for writing was closed (compare inotify's IN_CLOSE_WRITE) */
        EventCloseRead       = 0x00000010, /**< File not opened for writing was closed (compare inotify's IN_CLOSE_NOWRITE) */
        EventCreate          = 0x00000100, /** File/directory created in watched directory (compare inotify's IN_CREATE) */
        EventDelete          = 0x00000200, /**< File/directory deleted from watched directory (compare inotify's IN_DELETE) */
        EventDeleteSelf      = 0x00000400, /**< Watched file/directory was itself deleted (compare inotify's IN_DELETE_SELF) */
        EventModify          = 0x00000002, /**< File was modified (compare inotify's IN_MODIFY) */
        EventMoveSelf        = 0x00000800, /**< Watched file/directory was itself moved (compare inotify's IN_MOVE_SELF) */
        EventMoveFrom        = 0x00000040, /**< File moved out of watched directory (compare inotify's IN_MOVED_FROM) */
        EventMoveTo          = 0x00000080, /**< File moved into watched directory (compare inotify's IN_MOVED_TO) */
        EventOpen            = 0x00000020, /**< File was opened (compare inotify's IN_OPEN) */
        EventUnmount         = 0x00002000, /**< Backing fs was unmounted (compare inotify's IN_UNMOUNT) */
        EventQueueOverflow   = 0x00004000, /**< Event queued overflowed (compare inotify's IN_Q_OVERFLOW) */
        EventIgnored         = 0x00008000, /**< File was ignored (compare inotify's IN_IGNORED) */
        EventMove            = ( EventMoveFrom|EventMoveTo),
        EventAll             = ( EventAccess|
                                 EventAttributeChange|
                                 EventCloseWrite|
                                 EventCloseRead|
                                 EventCreate|
                                 EventDelete|
                                 EventDeleteSelf|
                                 EventModify|
                                 EventMoveSelf|
                                 EventMoveFrom|
                                 EventMoveTo|
                                 EventOpen )
    };
    Q_DECLARE_FLAGS(WatchEvents, WatchEvent)

    /**
     * Watch flags
     *
     * These flags correspond to the native Linux inotify flags.
     */
    enum WatchFlag
    {
        FlagOnlyDir     = 0x01000000, /**< Only watch the path if it is a directory (IN_ONLYDIR) */
        FlagDoNotFollow = 0x02000000, /**< Don't follow a sym link (IN_DONT_FOLLOW) */
        FlagOneShot     = 0x80000000  /**< Only send event once (IN_ONESHOT) */
    };
    Q_DECLARE_FLAGS(WatchFlags, WatchFlag)

    /**
     * \return \p true if inotify is available and usable.
     */
    static bool available();

    bool watchingPath( const QString& path ) const;

protected:

    /**
     * Called for every folder that is being watched.
     * Returns true if the watch should be add or false if it should NOT be added.
     */
    virtual bool filterWatch( const QString & path, WatchEvents & modes, WatchFlags & flags );

public Q_SLOTS:

    bool removeWatch( const QString& path );
    bool removeAllWatches();

    bool watchDirectory(const QString& path);
    bool watchDirectoryAndSubdirs(const QString& path);
    bool removeDirectory( const QString& path );

    virtual bool addWatch( const QString& path, WatchEvents modes, WatchFlags flags = WatchFlags() );

Q_SIGNALS:

    /**
     * Emitted if a file is accessed (KInotify::EventAccess)
     */
    void accessed( const QString& file );

    /**
     * Emitted if file attributes are changed (KInotify::EventAttributeChange)
     */
    void attributeChanged( const QString& file );

    /**
     * Emitted if FIXME (KInotify::EventCloseWrite)
     */
    void closedWrite( const QString& file );

    /**
     * Emitted if FIXME (KInotify::EventCloseRead)
     */
    void closedRead( const QString& file );

    /**
     * Emitted if a new file has been created in one of the watched
     * folders (KInotify::EventCreate)
     */
    void created( const QString& file, bool isDir );

    /**
     * Emitted if a watched file or folder has been deleted.
     * This includes files in watched foldes (KInotify::EventDelete and KInotify::EventDeleteSelf)
     */
    void deleted( const QString& file, bool isDir );

    /**
     * Emitted if a watched file is modified (KInotify::EventModify)
     */
    void modified( const QString& file );

    /**
     * Emitted if a file or folder has been moved or renamed.
     *
     * \warning The moved signal will only be emitted if both the source and target folder
     * are being watched.
     */
    void moved( const QString& oldName, const QString& newName );

    /**
     * Emitted if a file is moved from a given place, or to a given place.
     *
     * Note: moved() gives you more information in that it pairs movedFrom with the corresponding movedTo,
     * but is only emitted if both source and target are being watched.
     *
     * Note: Order of emitted signals is
     *  1) movedFrom
     *  2) moved
     *  3) movedTo
     */
    void movedFrom(const QString& oldName);
    void movedTo(const QString& newName);

    /**
     * Emitted if a file is opened (KInotify::EventOpen)
     */
    void opened( const QString& file );

    /**
     * Emitted if a watched path has been unmounted (KInotify::EventUnmount)
     */
    void unmounted( const QString& file );

    /**
     * Emitted if during updating the internal watch structures (recursive watches)
     * the inotify user watch limit was reached.
     *
     * This means that not all requested paths can be watched until the user watch
     * limit is increased.
     *
     * This signal will only be emitted once.
     */
    void watchUserLimitReached();

private Q_SLOTS:

    void slotEvent( int );

private:

    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void _k_addWatches() )
};

} // namespace Digikam

#endif // _KINOTIFY_H_
