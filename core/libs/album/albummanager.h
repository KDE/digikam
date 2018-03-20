/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface.
 *
 * Copyright (C) 2004      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

/** @file albummanager.h */

#ifndef ALBUMMANAGER_H
#define ALBUMMANAGER_H

// Qt includes

#include <QList>
#include <QObject>
#include <QString>
#include <QMap>
#include <QUrl>

// Local includes

#include "album.h"
#include "coredbalbuminfo.h"
#include "dbengineparameters.h"
#include "digikam_export.h"
#include "imagelisterrecord.h"

class QDate;

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

    /** @name Library path And Scanning
     */
    //@{
    /**
     * Initialize. Informs the user about failures.
     * Returns true on success, false on failure.
     * A return value of false during startup indicates termination of the program
     * (user is informed)
     */
    bool setDatabase(const DbEngineParameters& params, bool priority, const QString& suggestedAlbumRoot = QString());

    /**
     * Some checks for settings done in first run wizard in case of QSlite Database.
     */
    static void checkDatabaseDirsAfterFirstRun(const QString& dbPath, const QString& albumPath);

    /**
     * Sets new database when chosen by the user in setup.
     * Handles user notification about problems.
     * Call this instead of setDatabase when digiKam is up and running.
     */
    void changeDatabase(const DbEngineParameters& params);

    /**
     * Stop ongoing operations, prepare for application shutdown
     */
    void cleanUp();

    /**
     * Checks if the given database path is equal to the current one
     */
    bool databaseEqual(const DbEngineParameters& parameters) const;

    /**
     * starts scanning the libraryPath and listing the albums. If the
     * libraryPath has not changed since the last scan, then nothing happens
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

    /**
     * Ensures that valid item counts for physical and tag albums are available
     */
    void       prepareItemCounts();
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
     * @return a list of all DAlbums
     */
    AlbumList allDAlbums() const;

    /**
     * @return a list of all FAlbums
     */
    AlbumList allFAlbums() const;

    /**
     * set current album to @p albums. It's similar to setCurrentAlbum,
     * but supports multiple selected albums
     */
    void setCurrentAlbums(QList<Album*> albums);

    /**
     * @returns current albums, previously set up by setCurrentAlbums
     */
    AlbumList currentAlbums() const;

    /**
     * @returns the current PAlbum or null if no one is selected
     */
    PAlbum* currentPAlbum() const;

    /**
     * @returns the current TAlbum or null if no one is selected
     */
    QList<TAlbum*> currentTAlbums() const;

    /**
     * @returns the current FAlbum or null if no one is selected
     */
    FAlbum* currentFAlbum() const;
    //@}

    /** @name Finding Albums
     */
    //@{
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
     * @return a FAlbum with given name
     * @param name the name for the FAlbum (name of the person which the FAlbum corresponds to
     */
    FAlbum*   findFAlbum(const QString& name) const;

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
     * @return a TAlbum with given tag path, or 0 if not found
     * @param tagPath the tag path ("People/Friend/John")
     */
    TAlbum*   findTAlbum(const QString& tagPath) const;

    /**
     * @return a SAlbum with given name, or 0 if not found
     * @param name the name of the search
     */
    SAlbum*   findSAlbum(const QString& name) const;

    /**
     * @return SAlbums with given type, empty list if not found
     * @param searchType the type of the search
     */
    QList< SAlbum* > findSAlbumsBySearchType(int searchType) const;

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

    /**
     * Returns the id of the item with the given filename in
     * the given PAlbum.
     * @param album The albumId in which we search the item.
     * @param fileName The name of the item file.
     * @return The item id or -1 if not existent.
     */
    qlonglong getItemFromAlbum(PAlbum* album, const QString& fileName);
    //@}

    /**
     * @return A hash with the titles for all album IDs.
     */
    QHash<int, QString> albumTitles() const;

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
    bool moveTAlbum(TAlbum* album, TAlbum* newParent, QString& errMsg);

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
    //@}

    /** @name Operations on TAlbum
     */
    //@{
    /**
     * Create a new FAlbum with supplied properties as a child of the parent
     * The person is added to the database
     * \note the signalAlbumAdded will be fired before this function returns. Its
     * recommended to connect to that signal to get notification of new album added
     * @return the newly created FAlbum or 0 if it fails
     * @param parent  the parent album under which to create the new FAlbum
     * @param name    the name of the new album
     * @param iconkde the iconkde for the new album (this is a filename which
     * kde iconloader can load up
     * @param errMsg  this will contain the error message describing why the
     * operation failed
     */
    FAlbum* createFAlbum(FAlbum* parent, const QString& name,
                         const QString& iconkde, QString& errMsg);

    /**
     * Delete a FAlbum.
     * The person is removed from the database
     * \note the signalAlbumDeleted will be fired before this function returns. Its
     * recommended to connect to that signal to get notification of album deletes
     * @return true if the operation succeeds or false otherwise
     * @param album   the FAlbum to delete
     * @param errMsg  this will contain the error message describing why the
     * operation failed
     */
    bool deleteFAlbum(FAlbum* album, QString& errMsg);

    /**
     * Renames a FAlbum.
     * This updates the person name in the database
     * @return true if the operation succeeds, false otherwise
     * @param album the Album which should be renamed
     * @param name the new name for the album
     * @param errMsg this will contain the error message describing why the
     * operation failed
     */
    bool renameFAlbum(FAlbum* album, const QString& name, QString& errMsg);

    /**
     * Update the icon for a FAlbum.
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
    bool updateFAlbumIcon(FAlbum* album, const QString& iconKDE,
                          qlonglong iconID, QString& errMsg);

    /**
     * @return A list with the name paths for a list of tag names.
     * @param names list of tag album names
     * @param leadingSlash if <code>true</code> return name paths with a leading slash
     */
    QStringList namePaths(const QList<QString>& tagNames, bool leadingSlash=true) const;

    //@}

    /** @name Accessors to counting maps
     */
    //@{

    /**
     * Returns the latest count for PAlbums as also emitted via
     * signalPAlbumsDirty.
     *
     * @return count map for PAlbums
     */
    QMap<int, int> getPAlbumsCount() const;

    /**
     * Returns the latest count for TAlbums as also emitted via
     * signalTAlbumsDirty.
     *
     * @return count map for TAlbums
     */
    QMap<int, int> getTAlbumsCount() const;

    /**
     * Returns the latest count for DAlbums as also emitted via
     * signalDAlbumsDirty.
     *
     * @return count map for DAlbums
     */
    QMap<YearMonth, int> getDAlbumsCount() const;

    /**
     * Returns the latest count for faces as also emitted via
     * signalFaceCountsDirty.
     *
     * @return count map for faces (confirmed and unconfirmed combined)
     */
    QMap<int, int> getFaceCount() const;

    /**
     * Returns if the given album is currently being moved, that is,
     * if this album is in between signalAlbumAboutToBeMoved and
     * signalAlbumMoved. In this case, you can preserve state of such an album
     * because the object is guaranteed not to be deleted, even if
     * signalAlbumAboutToBeDeleted is emitted.
     */
    bool isMovingAlbum(Album* album) const;

    bool isShowingOnlyAvailableAlbums() const;
    void setShowOnlyAvailableAlbums(bool onlyAvailable);

    void removeWatchedPAlbums(const PAlbum* const album);

    void addFakeConnection();
    void removeFakeConnection();

    //@}

Q_SIGNALS:

    /// An album is about to be added to the given parent (0 if album is root)
    /// after the item given by prev (prev is 0 if parent has no children yet)
    void signalAlbumAboutToBeAdded(Album* album, Album* parent, Album* prev);
    /// The album has been added.
    void signalAlbumAdded(Album* album);
    /// The album is about to be deleted, but is still fully valid.
    void signalAlbumAboutToBeDeleted(Album* album);
    /// The album is deleted, but the object can still be accessed.
    void signalAlbumDeleted(Album* album);
    /// The album is deleted, the object can no longer be accessed.
    /// For identification purposes, the former album pointer is passed.
    void signalAlbumHasBeenDeleted(quintptr);
    void signalAlbumsCleared();
    void signalAlbumCurrentChanged(QList<Album*> album);
    void signalAllAlbumsLoaded();
    void signalAllDAlbumsLoaded();
    void signalAlbumIconChanged(Album* album);
    void signalAlbumRenamed(Album* album);
    void signalAlbumNewPath(Album* album);
    void signalSearchUpdated(SAlbum* album);
    /// Indicates that an album is about to be moved. Signals for deleting and adding will be
    /// sent afterwards, but the album object is guaranteed not to be deleted until after signalAlbumMoved.
    void signalAlbumAboutToBeMoved(Album* album);
    /// Emitted when the album is moved to its new parent. After signalAlbumAboutToBeMoved,
    /// all four signals for first deleting and then adding will have been sent.
    void signalAlbumMoved(Album* album);
    void signalPAlbumsDirty(const QMap<int, int>&);
    void signalTAlbumsDirty(const QMap<int, int>&);
    void signalDAlbumsDirty(const QMap<YearMonth, int>&);
    void signalFaceCountsDirty(const QMap<int, int>&);
    void signalDatesMapDirty(const QMap<QDateTime, int>&);
    void signalTagPropertiesChanged(TAlbum* album);
    void signalAlbumsUpdated(int type);
    void signalUpdateDuplicatesAlbums(const QList<SAlbum*>& modifiedAlbums, const QList<qlonglong>& deletedImages);
    // Signals a change in this property. Please note that affected albums may appear or disappear after this signal has been emitted.
    void signalShowOnlyAvailableAlbumsChanged(bool showsOnlyAvailableAlbums);

private Q_SLOTS:

    void slotDatesJobResult();
    void slotDatesJobData(const QMap<QDateTime, int>& datesStatMap);
    void slotAlbumsJobResult();
    void slotAlbumsJobData(const QMap<int,int>& albumsStatMap);
    void slotTagsJobResult();
    void slotTagsJobData(const QMap<int,int>& tagsStatMap);
    void slotPeopleJobResult();
    void slotPeopleJobData(const QMap<QString,QMap<int,int> >& facesStatMap);

    void slotCollectionLocationStatusChanged(const CollectionLocation&, int);
    void slotCollectionLocationPropertiesChanged(const CollectionLocation& location);
    void slotAlbumChange(const AlbumChangeset& changeset);
    void slotTagChange(const TagChangeset& changeset);
    void slotSearchChange(const SearchChangeset& changeset);
    void slotCollectionImageChange(const CollectionImageChangeset& changeset);
    void slotImageTagChange(const ImageTagChangeset& changeset);
    void slotImagesDeleted(const QList<qlonglong>& imageIds);

    /**
     * Scan albums directly from database and creates new PAlbums
     * It only creates those PAlbums which haven't already been
     * created.
     */
    void scanPAlbums();
    void updateChangedPAlbums();

    /**
     * Scan tags directly from database and creates new TAlbums
     * It only creates those TAlbums which haven't already been
     * created.
     */
    void scanTAlbums();
    /**
     * Scan searches directly from database and creates new SAlbums
     * It only creates those SAlbums which haven't already been
     * created.
     */
    void scanSAlbums();
    /**
     * Scan dates from the database (via IOSlave) and
     * updates the DAlbums.
     */
    void scanDAlbumsScheduled();
    void scanDAlbums();

    void getAlbumItemsCount();
    void getTagItemsCount();
    void tagItemsCount();
    void personItemsCount();

private:

    friend class AlbumManagerCreator;
    AlbumManager();
    ~AlbumManager();

    /**
     * Checks whether an Album has a direct child with the given name.
     *
     * @param parent album to check children for
     * @param title title to search for
     * @return <code>true</code> if there is a child with name, else
     *         <code>false</code>
     */
    bool hasDirectChildAlbumWithTitle(Album* parent, const QString& title);

    bool handleCollectionStatusChange(const CollectionLocation& location, int oldStatus);
    void insertPAlbum(PAlbum* album, PAlbum* parent);
    void removePAlbum(PAlbum* album);
    void insertTAlbum(TAlbum* album, TAlbum* parent);
    void removeTAlbum(TAlbum* album);
    void updateAlbumPathHash();

    void notifyAlbumDeletion(Album* album);

    void addAlbumRoot(const CollectionLocation& location);
    void removeAlbumRoot(const CollectionLocation& location);

    void handleKioNotification(const QUrl& url);

    void addGuardedPointer(Album* a, Album** pointer);
    void removeGuardedPointer(Album* a, Album** pointer);
    void changeGuardedPointer(Album* oldAlbum, Album* a, Album** pointer);
    void invalidateGuardedPointers(Album* album);

    static AlbumManager* internalInstance;

public:

    // Declared public because it used in ChangingDB class.
    class Private;

private:

    Private* const d;

    template <class T> friend class AlbumPointer;
    friend class Album;
};

// ------------------------------------------------------------------------------------

/**
 * You can use AlbumPointer to store a guarded pointer to Album
 * or one of the subclasses (use template parameter).
 * The pointer will be set to 0 when the album object is deleted.
 */
template <class T = Album>

class AlbumPointer
{
public:

    AlbumPointer()
        : album(0)
    {
    }

    AlbumPointer(T* a)
        : album(a)
    {
        AlbumManager::instance()->addGuardedPointer(album, &album);
    }

    AlbumPointer(const AlbumPointer<T>& p)
        : album(p.album)
    {
        AlbumManager::instance()->addGuardedPointer(album, &album);
    }

    ~AlbumPointer()
    {
        AlbumManager::instance()->removeGuardedPointer(album, &album);
    }

    AlbumPointer<T>& operator=(T* a)
    {
        Album* const oldAlbum = album;
        album                 = a;
        AlbumManager::instance()->changeGuardedPointer(oldAlbum, album, &album);
        return *this;
    }

    AlbumPointer<T>& operator=(const AlbumPointer<T>& p)
    {
        Album* const oldAlbum = album;
        album                 = p.album;
        AlbumManager::instance()->changeGuardedPointer(oldAlbum, album, &album);
        return *this;
    }

    T* operator->() const
    {
        return static_cast<T*>(const_cast<Album*>(album));
    }

    T& operator*() const
    {
        return *static_cast<T*>(const_cast<Album*>(album));
    }

    operator T* () const
    {
        return static_cast<T*>(const_cast<Album*>(album));
    }

    bool operator!() const
    {
        return !album;
    }

private:

    friend class AlbumManager;
    Album* album;
};

// ------------------------------------------------------------------------------------

template <class T = Album>

class AlbumPointerList : public QList<AlbumPointer<T> >
{
public:

    AlbumPointerList()
    {
    }

    AlbumPointerList(const AlbumPointerList<T>& list)
        : QList<AlbumPointer<T> >(list)
    {
    }

    explicit AlbumPointerList(const QList<T*>& list)
    {
        operator=(list);
    }

    AlbumPointerList<T>& operator=(const AlbumPointerList<T>& list)
    {
        return QList<AlbumPointer<T> >::operator=(list);
    }

    AlbumPointerList<T>& operator=(const QList<T*>& list)
    {
        foreach(T* const t, list)
        {
            this->append(AlbumPointer<T>(t));
        }

        return *this;
    }
};

}  // namespace Digikam

Q_DECLARE_METATYPE(Digikam::AlbumPointer<>)
Q_DECLARE_METATYPE(Digikam::AlbumPointer<Digikam::PAlbum>)
Q_DECLARE_METATYPE(Digikam::AlbumPointer<Digikam::TAlbum>)
Q_DECLARE_METATYPE(Digikam::AlbumPointer<Digikam::SAlbum>)
Q_DECLARE_METATYPE(Digikam::AlbumPointer<Digikam::DAlbum>)
Q_DECLARE_METATYPE(QList<Digikam::TAlbum*>)

#endif /* ALBUMMANAGER_H */
