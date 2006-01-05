/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-15
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju
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

/** @file albummanager.h */

#ifndef ALBUMMANAGER_H
#define ALBUMMANAGER_H

// Qt includes.

#include <qobject.h>
#include <qstring.h>
#include <qvaluelist.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "digikam_export.h"

class QDate;

namespace KIO
{
class Job;
}

namespace Digikam
{

class Album;
class PAlbum;
class TAlbum;
class DAlbum;
class SAlbum;
class AlbumDB;
class AlbumItemHandler;
class AlbumManagerPriv;

typedef QValueList<Album*> AlbumList;

/**
 * \class AlbumManager
 *
 * There are two primary managers which manage the listing and 
 * lifetime of Album and ImageInfo: AlbumManager and AlbumLister
 * 
 * AlbumManager manages albums: does listing of albums and controls the lifetime of it. 
 * For PAlbums and TAlbums, the listing is done by reading the db directly and
 * building the hierarchy of the albums. For DAlbums, since the listing takes
 * time, the work is delegated to a kioslave. Interested frontend entities can
 * connect to the albummanager to receive notifications of new Albums, when
 * Albums are deleted and when the current album is changed.
 * 
 * Additional operations are provided for: creating/deleting/rename Albums, 
 * updating icons and moving Albums.
 *
 */
class DIGIKAM_EXPORT AlbumManager : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor
     */
    AlbumManager();

    /**
     * Destructor
     */
    ~AlbumManager();

    /**
     * A convenience function to get the instance of the AlbumManager
     */
    static AlbumManager* instance();

    /**
     * returns a pointer to the current AlbumDB
     */
    AlbumDB*   albumDB();

    /** @name Library path And Scanning
     */
    //@{
    /**
    * Set the @p libraryPath to the given path
    *
    * If the libraryPath is the same as the current path, nothing happens.
    * Otherwise the currently listed albums are cleared. The albums in the new
    * library path are not listed till you call startScan()
    * @param path the new libraryPath
    * @see startScan
    */
    void       setLibraryPath(const QString& path);

    /**
    * @return the current libraryPath
    */
    QString    getLibraryPath() const;

    /**
    * starts scanning the libraryPath and listing the albums. If the
    * libraryPath has not changed since the lastscan, then nothing happens
    * @see setLibraryPath
    * @see refresh
    */
    void       startScan();

    /**
    * This is similar to startScan, except that it assumes you have run
    * startScan at least once. It checks the database to see if any new albums
    * have been added and updates them accordingly. Use this when a change in the
    * filesystem is detected (but the album library path hasn't changed)
    * @see startScan
    */
    void       refresh();
    //@}

    /** @name List of Albums and current Album
     */
    //@{
    /**
     * @return a list of all PAlbums
     */
    AlbumList allPAlbums() const;

    /**
     * @return a list of all TAlbums
     */
    AlbumList allTAlbums() const;

    /**
     * @return a list of all SAlbums
     */
    AlbumList allSAlbums() const;

    /**
     * @return a list of all SAlbums
     */
    AlbumList allDAlbums() const;

    /**
    * set the current album to @p album. Call this from views which show
    * listing of albums. This also causes it to fire the signal
    * signalAlbumCurrentChanged()
    */
    void      setCurrentAlbum(Album *album);
 
    /**
    * @returns the current albumm
    */
    Album*    currentAlbum() const;
    //@}

    /** @name Finding Albums
     */
    //@{
    /**
     * Given a complete file url (kde url with file protocol), it will try to find
     * a PAlbum corresponding to it.
     * \warning This should not be used, unless really necessary
     * @return PAlbum correspoding to supplied @p url
     * @param url the url we need to check
     */
    PAlbum*   findPAlbum(const KURL& url) const;

    /**
     * @return a PAlbum with given ID
     * @param id the id for the PAlbum
     */
    PAlbum*   findPAlbum(int id) const;

    /**
     * @return a TAlbum with given ID
     * @param id the id for the TAlbum
     */
    TAlbum*   findTAlbum(int id) const;

    /**
     * @return a SAlbum with given ID
     * @param id the id for the SAlbum
     */
    SAlbum*   findSAlbum(int id) const;

    /**
     * @return a DAlbum with given ID
     * @param id the id for the DAlbum
     */
    DAlbum*   findDAlbum(int id) const;
    
    /**
     * @return a Album with the given globalID
     * @param gid the global id for the album
     */
    Album*    findAlbum(int gid) const;
    //@}

    /** @name Operations on PAlbum
     */
    //@{
    /**
     * Create a new PAlbum with supplied properties as a child of the parent
     * This is equivalent to creating a new folder on the disk with supplied
     * name in the parent's folder path. Also the supplied attributes are written
     * out to the database
     * \note the signalAlbumAdded will be fired before this function returns. Its
     * recommended to connect to that signal to get notification of new album added
     * @return the newly created PAlbum or 0 if it fails
     * @param parent  the parent album under which to create the new Album
     * @param name    the name of the new album
     * @param caption the caption for the new album
     * @param date    the date for the new album
     * @param collection the collection for the new album
     * @param errMsg  this will contain the error message describing why the
     * operation failed
     */
    PAlbum* createPAlbum(PAlbum* parent, const QString& name,
                         const QString& caption, const QDate& date,
                         const QString& collection, 
                         QString& errMsg);
    
    /**
     * Renames a PAlbum. This is equivalent to actually renaming the corresponding
     * folder on the disk.
     * @return true if the operation succeeds, false otherwise
     * @param album the Album which should be renamed
     * @param newName the new name for the album
     * @param errMsg this will contain the error message describing why the
     * operation failed
     */ 
    bool renamePAlbum(PAlbum* album, const QString& newName,
                      QString& errMsg);

    /**
     * Update the icon for an album. The @p icon is the name (and not full path)
     * of the file in the album
     * @return true if the operation succeeds, false otherwise
     * @param album  the album for which icon should be changed
     * @param iconID the filename of the new icon
     * @param errMsg if the operation fails, this will contain the error message
     * describing why the operation failed
     */
    bool updatePAlbumIcon(PAlbum *album, Q_LLONG iconID, QString& errMsg);
    //@}
    
    /** @name Operations on TAlbum
     */
    //@{
    /**
     * Create a new TAlbum with supplied properties as a child of the parent
     * The tag is added to the database
     * \note the signalAlbumAdded will be fired before this function returns. Its
     * recommended to connect to that signal to get notification of new album added
     * @return the newly created TAlbum or 0 if it fails
     * @param parent  the parent album under which to create the new Album
     * @param name    the name of the new album
     * @param iconkde the iconkde for the new album (this is a filename which
     * kde iconloader can load up
     * @param errMsg  this will contain the error message describing why the
     * operation failed
     */
    TAlbum* createTAlbum(TAlbum* parent, const QString& name, 
                         const QString& iconkde, QString& errMsg);

    
    /**
     * Delete a TAlbum.
     * The tag is removed from the database
     * \note the signalAlbumDeleted will be fired before this function returns. Its
     * recommended to connect to that signal to get notification of album deletes
     * @return true if the operation succeeds or false otherwise
     * @param album   the TAlbum to delete
     * @param errMsg  this will contain the error message describing why the
     * operation failed
     */
    bool deleteTAlbum(TAlbum* album, QString& errMsg);

    /**
     * Renames a TAlbum.
     * This updates the tag name in the database
     * @return true if the operation succeeds, false otherwise
     * @param album the Album which should be renamed
     * @param name the new name for the album
     * @param errMsg this will contain the error message describing why the
     * operation failed
     */ 
    bool renameTAlbum(TAlbum* album, const QString& name, QString& errMsg);

    /**
     * Move a TAlbum to a new parent.
     * This updates the tag parent ID in the database
     * @return true if the operation succeeds, false otherwise
     * @param album the Album which should be moved
     * @param newParent the Parent Album to which album should be moved
     * @param errMsg this will contain the error message describing why the
     * operation failed
     */ 
    bool moveTAlbum(TAlbum* album, TAlbum *newParent, QString &errMsg);
    
    /**
     * Update the icon for a TAlbum. 
     * @return true if the operation succeeds, false otherwise
     * @param album the album for which icon should be changed
     * @param iconKDE  a simple filename which can be loaded by KIconLoader
     * @param iconID   id of the icon image file
     * @param errMsg this will contain the error message describing why the
     * operation failed
     * \note if iconKDE is not empty then iconID is used. So if you want to set
     * the icon to a file which can be loaded by KIconLoader, pass it in as
     * iconKDE. otherwise pass a null QString to iconKDE and set iconID
     */
    bool updateTAlbumIcon(TAlbum* album, const QString& iconKDE,
                          Q_LLONG iconID, QString& errMsg);
    //@}
    

    /** @name Operations on SAlbum
     */
    //@{
    /**
     * Create a new SAlbum with supplied url. If an existing SAlbum with same name
     * exists this function will return a pointer to that album, instead of creating
     * a new one. A newly created search album is added to the database. For an
     * existing SAlbum, the url is updated and written out to the database
     * \note the signalAlbumAdded will be fired before this function returns. Its
     * recommended to connect to that signal to get notification of new album added
     * @return the newly created SAlbum or an existing SAlbum with same name
     * @param url    the url of the album
     * @param simple indicates whether the Search album is of simple type or
     * extended type
     */
    SAlbum* createSAlbum(const KURL& url, bool simple);
    
    /**
     * Update the url for a SAlbum
     * @return true if the operation succeeds, false otherwise
     * @param album the album to update
     * @param newURL the new url of the album
     */
    bool updateSAlbum(SAlbum* album, const KURL& newURL);

    /**
     * Delete a SAlbum from the database
     * \note the signalAlbumDeleted will be fired before this function returns. Its
     * recommended to connect to that signal to get notification of album deletes
     * @return true if the operation succeeds, false otherwise
     * @param album the album to delete
     */
    bool deleteSAlbum(SAlbum* album);
    //@}
    
    void setItemHandler(AlbumItemHandler *handler);
    AlbumItemHandler* getItemHandler();
    void refreshItemHandler(const KURL::List& itemList=KURL::List());
    void emitAlbumItemsSelected(bool val);

private:

    static AlbumManager* m_instance;
    AlbumManagerPriv*    d;

    void insertPAlbum(PAlbum *album);
    void removePAlbum(PAlbum *album);
    void insertTAlbum(TAlbum *album);
    void removeTAlbum(TAlbum *album);

    /**
     * Scan albums directly from database and creates new PAlbums
     * It only create those PAlbums which haven't already been
     * created
     */
    void scanPAlbums();

    /**
     * Scan tags directly from database and creates new TAlbums
     * It only create those TAlbums which haven't already been
     * created
     */
    void scanTAlbums();
    
    /**
     * Scan searches directly from database and creates new SAlbums
     * It only create those SAlbums which haven't already been
     * created
     */
    void scanSAlbums();

    /**
     * Makes use of a KIO::Job to list dates from the database
     * @see slotResult
     * @see slotData
     */
    void scanDAlbums();

private slots:

    void slotResult(KIO::Job* job);
    void slotData(KIO::Job* job, const QByteArray& data);
    void slotDirty(const QString& path);

signals:

    void signalAlbumAdded(Album* album);
    void signalAlbumDeleted(Album* album);
    void signalAlbumItemsSelected(bool selected);
    void signalAlbumsCleared();
    void signalAlbumCurrentChanged(Album* album);
    void signalAllAlbumsLoaded();
    void signalAllDAlbumsLoaded();    
    void signalAlbumIconChanged(Album* album);
    void signalAlbumRenamed(Album* album);
    void signalTAlbumMoved(TAlbum* album, TAlbum* newParent);
};

}  // namespace Digikam

#endif /* ALBUMMANAGER_H */
