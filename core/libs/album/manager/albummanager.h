/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface.
 *
 * Copyright (C) 2004      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAM_ALBUM_MANAGER_H
#define DIGIKAM_ALBUM_MANAGER_H

// Qt includes

#include <QList>
#include <QObject>
#include <QString>
#include <QMap>
#include <QUrl>
#include <QDate>
#include <QFileInfo>

// Local includes

#include "album.h"
#include "coredbalbuminfo.h"
#include "dbengineparameters.h"
#include "digikam_export.h"
#include "itemlisterrecord.h"

namespace Digikam
{

class FAlbum;
class CollectionLocation;
class AlbumChangeset;
class TagChangeset;
class SearchChangeset;
class CollectionImageChangeset;
class ImageTagChangeset;

/**
 * \class AlbumManager
 *
 * AlbumManager manages albums: does listing of albums and controls the lifetime of it.
 * For PAlbums and TAlbums, the listing is done by reading the db directly and
 * building the hierarchy of the albums. For DAlbums, since the listing takes
 * time, the work is delegated to a dbjob. Interested frontend entities can
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
     * A convenience function to get the instance of the AlbumManager
     */
    static AlbumManager* instance();

    /**
     * starts scanning the libraryPath and listing the albums. If the
     * libraryPath has not changed since the last scan, then nothing happens
     * @see setLibraryPath
     * @see refresh
     */
    void startScan();

    /**
     * Stop ongoing operations, prepare for application shutdown
     */
    void cleanUp();

    /**
     * This is similar to startScan, except that it assumes you have run
     * startScan at least once. It checks the database to see if any new albums
     * have been added and updates them accordingly. Use this when a change in the
     * filesystem is detected (but the album library path hasn't changed)
     * @see startScan
     */
    void refresh();

    /**
     * Ensures that valid item counts for physical and tag albums are available
     */
    void prepareItemCounts();

    bool isShowingOnlyAvailableAlbums() const;
    void setShowOnlyAvailableAlbums(bool onlyAvailable);

private Q_SLOTS:

    void slotImagesDeleted(const QList<qlonglong>& imageIds);

    // -----------------------------------------------------------------------------

    /** @name Operations with database
     */

    //@{

public:

    /**
     * Initialize. Informs the user about failures.
     * Returns true on success, false on failure.
     * A return value of false during startup indicates termination of the program
     * (user is informed)
     */
    bool setDatabase(const DbEngineParameters& params, bool priority,
                     const QString& suggestedAlbumRoot = QString());

    /**
     * Sets new database when chosen by the user in setup.
     * Handles user notification about problems.
     * Call this instead of setDatabase when digiKam is up and running.
     */
    void changeDatabase(const DbEngineParameters& params);

    /**
     * Checks if the given database path is equal to the current one
     */
    bool databaseEqual(const DbEngineParameters& parameters) const;

    void addFakeConnection();
    void removeFakeConnection();

    /**
     * Some checks for settings done in first run wizard in case of QSlite Database.
     */
    static void checkDatabaseDirsAfterFirstRun(const QString& dbPath, const QString& albumPath);

private:

    static bool moveToBackup(const QFileInfo& info);
    static bool copyToNewLocation(const QFileInfo& oldFile,
                                  const QFileInfo& newFile,
                                  const QString otherMessage = QString());

    //@}

    // -----------------------------------------------------------------------------

    /** @name Operations with collections
     */

    //@{

private:

    /** Returns true if it added or removed an album.
     */
    bool handleCollectionStatusChange(const CollectionLocation& location, int oldStatus);

    void addAlbumRoot(const CollectionLocation& location);
    void removeAlbumRoot(const CollectionLocation& location);

private Q_SLOTS:

    void slotCollectionLocationStatusChanged(const CollectionLocation&, int);
    void slotCollectionLocationPropertiesChanged(const CollectionLocation& location);
    void slotCollectionImageChange(const CollectionImageChangeset& changeset);

    //@}

    // -----------------------------------------------------------------------------

    /**
     * @name Operations on generic Album
     */

    //@{

public:

    /**
     * set current album to @p albums.
     */
    void setCurrentAlbums(const QList<Album*>& albums);

    /**
     * @returns current albums, previously set up by setCurrentAlbums
     */
    AlbumList currentAlbums() const;

    /**
     * clear current albums.
     */
    void clearCurrentAlbums();

    /**
     * @return a Album with the given globalID
     * @param gid the global id for the album
     */
    Album*    findAlbum(int gid) const;

    /**
     * @return a Album with the given type and id
     * @param id the id for the album (not the global id)
     */
    Album*    findAlbum(Album::Type type, int id) const;

    /**
     * @return A hash with the titles for all album IDs.
     */
    QHash<int, QString> albumTitles() const;

    /**
     * Returns if the given album is currently being moved, that is,
     * if this album is in between signalAlbumAboutToBeMoved and
     * signalAlbumMoved. In this case, you can preserve state of such an album
     * because the object is guaranteed not to be deleted, even if
     * signalAlbumAboutToBeDeleted is emitted.
     */
    bool isMovingAlbum(Album* album) const;

    /**
     * Returns the id of the item with the given filename in
     * the given Album.
     * @param album The album in which we search the item.
     * @param fileName The name of the item file.
     * @return The item id or -1 if not existent.
     */
    qlonglong getItemFromAlbum(Album* const album, const QString& fileName);

private:

    bool hasDirectChildAlbumWithTitle(Album* parent, const QString& title);
    void notifyAlbumDeletion(Album* album);
    void updateAlbumPathHash();
    void addGuardedPointer(Album* a, Album** pointer);
    void removeGuardedPointer(Album* a, Album** pointer);
    void changeGuardedPointer(Album* oldAlbum, Album* a, Album** pointer);
    void invalidateGuardedPointers(Album* album);

private Q_SLOTS:

    void slotAlbumsJobResult();
    void slotAlbumsJobData(const QMap<int, int>& albumsStatMap);
    void slotAlbumChange(const AlbumChangeset& changeset);
    void getAlbumItemsCount();

Q_SIGNALS:

    /** Emitted when an album is about to be added to the given parent (0 if album is root)
     *  after the item given by prev (prev is 0 if parent has no children yet).
     */
    void signalAlbumAboutToBeAdded(Album* album, Album* parent, Album* prev);

    /** Emitted when the album has been added.
     */
    void signalAlbumAdded(Album* album);

    /** Emitted when the album is about to be deleted, but is still fully valid.
     */
    void signalAlbumAboutToBeDeleted(Album* album);

    /** Emitted when the album is deleted, but the object can still be accessed.
     */
    void signalAlbumDeleted(Album* album);

    /** Emitted when the album is deleted, the object can no longer be accessed.
     *  For identification purposes, the former album pointer is passed.
     */
    void signalAlbumHasBeenDeleted(quintptr);

    void signalAlbumsCleared();
    void signalAlbumCurrentChanged(const QList<Album*>& albums);
    void signalAllAlbumsLoaded();

    void signalAlbumIconChanged(Album* album);
    void signalAlbumRenamed(Album* album);
    void signalAlbumNewPath(Album* album);

    /** Emittedd when an album is about to be moved. Signals for deleting and adding will be
     *  sent afterwards, but the album object is guaranteed not to be deleted until after signalAlbumMoved.
     */
    void signalAlbumAboutToBeMoved(Album* album);

    /** Emitted when the album is moved to its new parent. After signalAlbumAboutToBeMoved,
     *  all four signals for first deleting and then adding will have been sent.
     */
    void signalAlbumMoved(Album* album);
    void signalAlbumsUpdated(int type);

    /** Emitted when a change is done on available Albums.
     *  Please note that affected albums may appear or disappear after this signal has been emitted.
     */
    void signalShowOnlyAvailableAlbumsChanged(bool showsOnlyAvailableAlbums);

    //@}

    // -----------------------------------------------------------------------------

    /** @name Operations on Date Album
     */

    //@{

public:

    /**
     * @return a list of all DAlbums
     */
    AlbumList allDAlbums() const;

    /**
     * @return a DAlbum with given ID
     * @param id the id for the DAlbum
     */
    DAlbum*   findDAlbum(int id) const;

    /**
     * Returns the latest count for DAlbums as also emitted via
     * signalDAlbumsDirty.
     *
     * @return count map for DAlbums
     */
    QMap<YearMonth, int> getDAlbumsCount() const;

private Q_SLOTS:

    void slotDatesJobResult();
    void slotDatesJobData(const QMap<QDateTime, int>& datesStatMap);

   /**
     * Scan dates from the database and updates the DAlbums.
     */
    void scanDAlbumsScheduled();
    void scanDAlbums();

Q_SIGNALS:

    void signalDAlbumsDirty(const QMap<YearMonth, int>&);
    void signalDatesMapDirty(const QMap<QDateTime, int>&);
    void signalAllDAlbumsLoaded();

    //@}

    // -----------------------------------------------------------------------------

    /** @name Operations on Physical Album
     */

    //@{

public:

    /**
     * @return a list of all PAlbums
     */
    AlbumList allPAlbums() const;

    /**
     * @returns the current PAlbum or null if no one is selected
     */
    PAlbum* currentPAlbum() const;

    /**
     * Given a complete file url (kde url with file protocol), it will try to find
     * a PAlbum corresponding to it.
     * \warning This should not be used, unless really necessary
     * @return PAlbum corresponding to supplied @p url
     * @param url the url we need to check
     */
    PAlbum*   findPAlbum(const QUrl& url) const;

    /**
     * @return a PAlbum with given ID
     * @param id the id for the PAlbum
     */
    PAlbum*   findPAlbum(int id) const;

    /**
     * Returns the latest count for PAlbums as also emitted via
     * signalPAlbumsDirty.
     *
     * @return count map for PAlbums
     */
    QMap<int, int> getPAlbumsCount() const;

    void removeWatchedPAlbums(const PAlbum* const album);

    /**
     * Create a new PAlbum with supplied properties as a child of the parent
     * This is equivalent to creating a new folder on the disk with supplied
     * name in the parent's folder path. Also the supplied attributes are written
     * out to the database
     * \note the signalAlbumAdded will be fired before this function returns. Its
     * recommended to connect to that signal to get notification of new album added
     * @return the newly created PAlbum or 0 if it fails
     * @param parent  The parent album under which to create the new Album.
     *                Parent must not be root. Otherwise, use the other variants of this method.
     *                If parent is root, the albumRootPath must be supplied.
     * @param name    the name of the new album
     * @param caption the caption for the new album
     * @param date    the date for the new album
     * @param errMsg  this will contain the error message describing why the
     * operation failed
     */
    PAlbum* createPAlbum(PAlbum* parent, const QString& name,
                         const QString& caption, const QDate& date,
                         const QString& category,
                         QString& errMsg);

    /**
     * Overloaded method. Here you can supply an albumRootPath which must
     * correspond to an available collection location.
     */
    PAlbum* createPAlbum(const QString& albumRootPath, const QString& name,
                         const QString& caption, const QDate& date,
                         const QString& category,
                         QString& errMsg);

    /**
     * Overloaded method. Here you can supply a collection location (which
     * must be available).
     *
     * @param location the collection for the new album
     */
    PAlbum* createPAlbum(const CollectionLocation& location, const QString& name,
                         const QString& caption, const QDate& date,
                         const QString& category,
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
    bool updatePAlbumIcon(PAlbum* album, qlonglong iconID, QString& errMsg);

private:

    void insertPAlbum(PAlbum* album, PAlbum* parent);
    void removePAlbum(PAlbum* album);

private Q_SLOTS:

    /**
     * Scan albums directly from database and creates new PAlbums
     * It only creates those PAlbums which haven't already been
     * created.
     */
    void scanPAlbums();
    void updateChangedPAlbums();

Q_SIGNALS:

    void signalPAlbumsDirty(const QMap<int, int>&);

    //@}

    // -----------------------------------------------------------------------------

    /** @name Operations on Tag Album
     */

    //@{

public:

    /**
     * @return a list of all TAlbums
     */
    AlbumList allTAlbums() const;

    /**
     * @returns the current TAlbum or null if no one is selected
     */
    QList<TAlbum*> currentTAlbums() const;

    /**
     * @return a TAlbum with given ID
     * @param id the id for the TAlbum
     */
    TAlbum*   findTAlbum(int id) const;

    /**
     * @return a TAlbum with given tag path, or 0 if not found
     * @param tagPath the tag path ("People/Friend/John")
     */
    TAlbum*   findTAlbum(const QString& tagPath) const;

    /**
     * Returns the latest count for TAlbums as also emitted via
     * signalTAlbumsDirty.
     *
     * @return count map for TAlbums
     */
    QMap<int, int> getTAlbumsCount() const;

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
     * A list of tag paths is supplied.
     * If no corresponding TAlbum exists, a new one will be created.
     * @param tagPaths A list of tag paths
     * @returns A list of all TAlbums for the list (already existing or newly created)
     */
    AlbumList findOrCreateTAlbums(const QStringList& tagPaths);

    /**
     * Delete a TAlbum.
     * The tag is removed from the database
     * \note the signalAlbumDeleted will be fired before this function returns. Its
     * recommended to connect to that signal to get notification of album deletes
     * @return true if the operation succeeds or false otherwise
     * @param album   the TAlbum to delete
     * @param errMsg  this will contain the error message describing why the
     * @param askUser ask user to write metadata to images
     * operation failed
     */
    bool deleteTAlbum(TAlbum* album, QString& errMsg, bool askUser = true);

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
    bool moveTAlbum(TAlbum* album, TAlbum* newParent, QString& errMsg);

    /**
     * Merge a TAlbum to a TAlbum.
     * This updates the image tags in the database
     * @return true if the operation succeeds, false otherwise
     * @param album the Album which should be merged
     * @param destAlbum the Album to which album should be merged
     * @param dialog show dialog to ask the user if he wants to merge
     * @param errMsg this will contain the error message describing why the
     * operation failed
     */

    bool mergeTAlbum(TAlbum* album, TAlbum* destAlbum, bool dialog, QString& errMsg);

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
                          qlonglong iconID, QString& errMsg);

    /**
     * Get a list of recently assigned tags (only last 6 tags are listed)
     * @return the list of recently assigned TAlbums
     * @param includeInternal include internal tags in the returned list, or skip them
     */
    AlbumList getRecentlyAssignedTags(bool includeInternal = false) const;

    /**
     * @return A list with the tag paths for a list of tag IDs.
     * @param tagIDs list of tag album IDs
     * @param leadingSlash if <code>true</code> return tags with a leading slash
     * @param includeInternal include internal tags in the returned list, or skip them
     */
    QStringList tagPaths(const QList<int>& tagIDs, bool leadingSlash=true, bool includeInternal = false) const;

    /**
     * @return A list with the tag names for a list of tag IDs.
     * @param tagIDs list of tag album IDs
     */
    QStringList tagNames(const QList<int>& tagIDs, bool includeInternal = false) const;

    /**
     * @return A hash with the tag paths for all tag IDs.
     */
    QHash<int, QString> tagPaths(bool leadingSlash=true, bool includeInternal = false) const;

    /**
     * @return A hash with the tag names for all tag IDs.
     */
    QHash<int, QString> tagNames(bool includeInternal = false) const;

    /**
     * Returns a list of TAlbums which have the given property,
     * or the given property/value combination.
     */
    AlbumList findTagsWithProperty(const QString& property);
    AlbumList findTagsWithProperty(const QString& property, const QString& value);

    /**
     * TODO
     */
    QList<int> subTags(int tagId, bool recursive = false);

private:

    /**
     * Checks whether an Album has a direct child with the given name.
     *
     * @param parent album to check children for
     * @param title title to search for
     * @return <code>true</code> if there is a child with name, else
     *         <code>false</code>
     */
    void askUserForWriteChangedTAlbumToFiles(TAlbum* const album);
    void askUserForWriteChangedTAlbumToFiles(const QList<qlonglong>& imageIds);

    void insertTAlbum(TAlbum* album, TAlbum* parent);
    void removeTAlbum(TAlbum* album);

private Q_SLOTS:

    /**
     * Scan tags directly from database and creates new TAlbums
     * It only creates those TAlbums which haven't already been
     * created.
     */
    void scanTAlbums();

    void slotTagsJobResult();
    void slotTagsJobData(const QMap<int, int>& tagsStatMap);
    void slotTagChange(const TagChangeset& changeset);
    void slotImageTagChange(const ImageTagChangeset& changeset);

    void getTagItemsCount();
    void tagItemsCount();

Q_SIGNALS:

    void signalTAlbumsDirty(const QMap<int, int>&);
    void signalTagPropertiesChanged(TAlbum* album);

    //@}

    // -----------------------------------------------------------------------------

    /** @name Operations on Search Album
     */

    //@{

public:

    /**
     * @return a list of all SAlbums
     */
    AlbumList allSAlbums() const;

    /**
     * @return a SAlbum with given ID
     * @param id the id for the SAlbum
     */
    SAlbum*   findSAlbum(int id) const;

    /**
     * @return a SAlbum with given name, or 0 if not found
     * @param name the name of the search
     */
    SAlbum*   findSAlbum(const QString& name) const;

    /**
     * @return SAlbums with given type, empty list if not found
     * @param searchType the type of the search
     */
    QList<SAlbum*> findSAlbumsBySearchType(int searchType) const;

    /**
     * Create a new SAlbum with supplied url. If an existing SAlbum with same name
     * exists this function will return a pointer to that album, instead of creating
     * a new one. A newly created search album is added to the database. For an
     * existing SAlbum, the url is updated and written out to the database
     * \note the signalAlbumAdded will be fired before this function returns. Its
     * recommended to connect to that signal to get notification of new album added
     * @return the newly created SAlbum or an existing SAlbum with same name
     * @param name  name for the new search
     * @param type  the type of the search
     * @param query search query to use
     */
    SAlbum* createSAlbum(const QString& name, DatabaseSearch::Type type, const QString& query);

    /**
     * Update the url for a SAlbum
     * @return true if the operation succeeds, false otherwise
     * @param album the album to update
     * @param changedQuery the new query data of the album
     * @param changedName a new name, or null to keep the current name
     * @param type a new type, or UndefinedType to keep the current type
     */
    bool updateSAlbum(SAlbum* album, const QString& changedQuery,
                      const QString& changedName = QString(), DatabaseSearch::Type type = DatabaseSearch::UndefinedType);

    /**
     * Delete a SAlbum from the database
     * \note the signalAlbumDeleted will be fired before this function returns. Its
     * recommended to connect to that signal to get notification of album deletes
     * @return true if the operation succeeds, false otherwise
     * @param album the album to delete
     */
    bool deleteSAlbum(SAlbum* album);

private Q_SLOTS:

    /**
     * Scan searches directly from database and creates new SAlbums
     * It only creates those SAlbums which haven't already been
     * created.
     */
    void scanSAlbums();

    void slotSearchChange(const SearchChangeset& changeset);

Q_SIGNALS:

    void signalUpdateDuplicatesAlbums(const QList<SAlbum*>& modifiedAlbums,
                                      const QList<qlonglong>& deletedImages);
    void signalSearchUpdated(SAlbum* album);

    //@}

    // -----------------------------------------------------------------------------

    /** @name Operations on Face Album
     */

    //@{

public:

    /**
     * Returns the latest count for faces as also emitted via
     * signalFaceCountsDirty.
     *
     * @return count map for faces (confirmed and unconfirmed combined)
     */
    QMap<int, int> getFaceCount() const;

private Q_SLOTS:

    void slotPeopleJobResult();
    void slotPeopleJobData(const QMap<QString, QMap<int, int> >& facesStatMap);
    void personItemsCount();

Q_SIGNALS:

    void signalFaceCountsDirty(const QMap<int, int>&);

    //@}

private:

    friend class AlbumManagerCreator;
    AlbumManager();
    ~AlbumManager();

    static AlbumManager* internalInstance;

public:

    // Declared public because it used in ChangingDB class.
    class Private;

private:

    Private* const d;

    template <class T> friend class AlbumPointer;
    friend class Album;
};

} // namespace Digikam

#endif // DIGIKAM_ALBUM_MANAGER_H
