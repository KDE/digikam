/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-18
 * Description :database album interface.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

/** @file albumdb.h */

#ifndef ALBUMDB_H
#define ALBUMDB_H

// Qt includes.

#include <QString>
#include <QList>
#include <QStringList>
#include <QDateTime>
#include <QPair>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "albuminfo.h"
#include "databasefields.h"
#include "databaseaccess.h"
#include "digikam_export.h"

namespace Digikam
{

class DatabaseBackend;
class AlbumDBPriv;

class DIGIKAM_EXPORT AlbumDB
{
public:

    /**
     * This adds a keyword-value combination to the database Settings table
     * if the keyword already exists, the value will be replaced with the new
     * value.
     * @param keyword The keyword
     * @param value The value
     */
    void setSetting(const QString& keyword, const QString& value);

    /**
     * This function returns the value which is stored in the database
     * (table Settings).
     * @param keyword The keyword for which the value has to be returned.
     * @return The values which belongs to the keyword.
     */
    QString getSetting(const QString& keyword);

    /**
     * Get the settings for the file name filters of this database.
     * Returns a list with lowercase suffixes only, no wildcards added ("png", not "*.png")
     * Returned is a joint result of main and user settings.
     */
    void getFilterSettings(QStringList &imageFilter, QStringList &videoFilter, QStringList &audioFilter);

    /**
     * Returns the user-configurable filter settings.
     */
    void getUserFilterSettings(QString &imageFilterString, QString &videoFilterString, QString &audioFilterString);

    /**
     * Sets the main filter settings of the database. Should only be called at schema update.
     */
    void setFilterSettings(const QStringList &imageFilter, const QStringList &videoFilter, const QStringList &audioFilter);

    /**
     * Sets the user-configurable filter settings. The lists shall be as specified for getFilterSettings.
     * They may include entries starting with "-", which indicates that this format shall be removed from
     * the list, if it is included in the main settings list.
     */
    void setUserFilterSettings(const QStringList &imageFilter, const QStringList &videoFilter, const QStringList &audioFilter);

    /**
     * Sets the user-configurable filter settings. The strings shall be lists joined either by ";" or " ".
     * Extra whitespace, dots and wildcard characters (*.) are removed.
     */
    void setUserFilterSettings(const QString &imageFilterString, const QString &videoFilterString, const QString &audioFilterString);

    // ----------- AlbumRoot operations -----------

    /**
     * Returns all albums and their attributes in the database
     * @return a list of albums and their attributes
     */
    QList<AlbumRootInfo> getAlbumRoots();

    /**
     * Returns all albums with a specified status and their attributes in the database
     * @param status The status that the requested album roots shall have
     * @return a list of albums and their attributes
     */
    //QList<AlbumRootInfo> getAlbumRootsWithStatus(int status);

    /**
     * Add a new album to the database with the given attributes
     * @param type The type of the album root
     * @param absolutePath The last absolute path in the file system.
     *                     The usage of this parameter is up to the CollectionManager
     * @param uuid         The volume UUID of the volume this album root is contained on.
     * @param specificPath The path specific to volume
     * @returns the album root id of the newly created root
     */
    int addAlbumRoot(int type, const QString &identifier, const QString &specificPath);

    /**
     * Deletes an album  root from the database.
     * @param rootId the id of the album root
     */
    void deleteAlbumRoot(int rootId);

    /**
     * Returns the status of the album root with the given id,
     * or -1 if no such album root exists.
     */
    //int getAlbumRootStatus(int rootId);

    /**
     * Sets the status and absolutePath of the album root with given id.
     * @param status The new status
     * @param absolutePath The new absolute path. If this is null, it will not be changed.
     */
    //void setAlbumRootStatus(int rootId, int status, const QString &absolutePath = QString());

    // ----------- Album Listing operations -----------
    /**
     * Returns all albums and their attributes in the database
     * @return a list of albums and their attributes
     */
    AlbumInfo::List scanAlbums();

    /**
     * Returns all tags and their attributes in the database
     * @return a list of tags and their attributes
     */
    TagInfo::List scanTags();

    /**
     * Returns all searches from the database
     * @return a list of searches from the database
     */
    SearchInfo::List scanSearches();

    /**
     * Returns all albums in the database with their albumRoot and ID.
     */
    QList<AlbumShortInfo> getAlbumShortInfos();

    // ----------- Operations on PAlbums -----------

    /**
     * Add a new album to the database with the given attributes
     * @param albumRootId   id of the album root of the new album
     * @param relativePath  url of the album
     * @param caption       the album caption
     * @param date          the date for the album
     * @param collection    the album collection
     * @return the id of the album added or -1 if it failed
     */
    int addAlbum(int albumRootId, const QString& relativePath,
                 const QString& caption,
                 const QDate& date, const QString& collection);

    /*
    int addAlbum(const QString& albumRoot, const QString& relativePath,
                 const QString& caption,
                 const QDate& date, const QString& collection);
    */

    /**
     * Find out the album for a given folder.
     * @param albumRootId  id of the album root of the album
     * @param relativePath The relative path for which you want the albumID relative to the album root
     * @param create       If true, an album is newly created if it does not yet exist.
     *                     If false, -1 is returned if no album exists.
     * @return The albumID for that folder,
               or -1 if it does not exist and create is false.
     */
    int  getAlbumForPath(int albumRootId, const QString& relativePath, bool create = true);

    //int  getAlbumForPath(const QString &albumRoot, const QString& relativePath, bool create = true);

    /**
     * Find out the album ids for a given relative path, including the subalbums.
     * @param albumRootId  id of the album root of the album
     * @param relativePath The path for which you want the albumIDs relative to the album root
     * @return a list of album ids. The list is empty if now albums are found.
     */
    QList<int> getAlbumAndSubalbumsForPath(int albumRootId, const QString& relativePath);

    /* *
     * List the urls of all subalbums of the album specified by albumRoot and path.
     * @param onlyDirectSubalbums if this is true, only first-level subalbums are returned,
     *                            if false, all levels of children are returned (include subalbums of subalbums).
     */
    //QStringList getSubalbumsForPath(const QString &albumRoot, const QString& path, bool onlyDirectSubalbums = true);

    /**
     * Deletes an album from the database. This will not delete the
     * subalbums of the album.
     * @param albumID the id of the album
     */
    void deleteAlbum(int albumID);

    /**
     * Copy the properties of the given srcAlbum to the dstAlbum.
     * Both albums must exist.
     * @return true if the operations succeeds
     */
    bool copyAlbumProperties(int srcAlbumID, int dstAlbumID);

    /**
     * Set a new url for the album. This will not affect the url
     * of the subalbums.
     * @param albumID the id of the album
     * @param url     the new url for the album
     * @param renameSubalbums change the url of all subalbums as well
     */
    void renameAlbum(int albumID, const QString& url, bool renameSubalbums);

    /**
     * Set a caption for the album.
     * @param albumID the id of the album
     * @param caption the new caption for the album
     */
    void setAlbumCaption(int albumID, const QString& caption);

    /**
     * Set a collection for the album.
     * @param albumID    the id of the album
     * @param collection the new collection for the album
     */
    void setAlbumCollection(int albumID, const QString& collection);

    /**
     * Set a date for the album.
     * @param albumID  the id of the album
     * @param date     the date for the album
     */
    void setAlbumDate(int albumID, const QDate& date);

    /**
     * Set the icon for the album.
     * @param albumID the id of the album
     * @param iconID  the id of the icon file
     */
    void setAlbumIcon(int albumID, qlonglong iconID);

    /**
     * Get the image that is set as album icon
     * @param albumID the id of the album
     * @param iconAlbumRootId Returns the album root id of the image
     * @param iconAlbumRelativePath Returns the path below the album root of the image
     * @returns true if there is an icon set.
     */
    bool getAlbumIcon(int albumID, int *iconAlbumRootId, QString *iconAlbumRelativePath);

    /**
     * Given an albumid, this returns the album root id for that album
     * @param albumID the id of the albumdb
     * @return the id of the album root of this album
     */
    int getAlbumRootId(int albumID);

    /**
     * Given an albumid, this returns the path for that album
     * @param albumID the id of the album
     * @return the url of the album
     */
    //QString getAlbumPath(int albumID);

    /**
     * Given an albumid, this returns the relative path for that album
     * (the path below the album root, starting with a slash)
     * @param albumID the id of the album
     * @return the url of the album
     */
    QString getAlbumRelativePath(int albumID);

    /**
     * Returns the lowest/oldest date of all images for that album.
     * @param albumID the id of the album to calculate
     * @return the date.
     */
    QDate getAlbumLowestDate(int albumID);

    /**
     * Returns the highest/newest date of all images for that album.
     * @param albumID the id of the album to calculate
     * @return the date.
     */
    QDate getAlbumHighestDate(int albumID);

    /**
     * Returns the average date of all images for that album.
     * @param albumID the id of the album to calculate
     * @return the date.
     */
    QDate getAlbumAverageDate(int albumID);

    // ----------- Operations on TAlbums -----------

    /**
     * Adds a new tag to the database with given name, icon and parent id.
     * @param parentTagID the id of the tag which will become the new tags parent
     * @param name        the name of the tag
     * @param iconKDE     the name of the icon file (this is filename which kde
     * iconloader can load up)
     * @param iconID      the id of the icon file
     * Note: if the iconKDE parameter is empty, then the iconID parameter is used
     * @return the id of the tag added or -1 if it failed
     */
    int addTag(int parentTagID, const QString& name,
               const QString& iconKDE, qlonglong iconID);

    /**
     * Add tags for the item, create tags with the given paths if they do not yet exist
     * @param tagPaths a list of tag paths
     * @param create create new tags if necessary
     * @returns a list of albumIDs of the tags in tagPaths
     */
    QList<int> getTagsFromTagPaths(const QStringList &tagPaths, bool create);

    /**
     * Get a list of recently assigned tags (only last 6 tags are listed)
     * @return the list of recently assigned tags
     */
    //TODO move to other place (AlbumManager)
    QList<int> getRecentlyAssignedTags() const;

    /**
     * Deletes a tag from the database. This will not delete the
     * subtags of the tag.
     * @param tagID the id of the tag
     */
    void deleteTag(int tagID);

    /**
     * Set a new name for the tag. 
     * @param tagID the id of the tag
     * @param name  the new name for the tag
     */
    void setTagName(int tagID, const QString& name);

    /**
     * Set the icon for the tag.
     * @param tagID the id of the tag
     * @param iconKDE the filename for the kde icon file
     * @param iconID the id of the icon file
     * Note: Only one of the iconKDE or iconID parameters is used.
     * if the iconKDE parameter is empty, then the iconID parameter is used
     */
    void setTagIcon(int tagID, const QString& iconKDE, qlonglong iconID);

    /**
     * Get the icon for the tag.
     * This can be either an image of the collection, or a system icon name.
     * So either iconAlbumRelativePath or icon will be null.
     * @param tagID the id of the tag
     * @param iconAlbumRootId Returns the album root id of the image
     * @param iconAlbumRelativePath Returns the path below the album root of the image
     * @param icon an icon name that can be loaded by the system iconloader
     * @returns true if there is an icon set.
     */
    bool getTagIcon(int tagID, int *iconAlbumRootId, QString *iconAlbumRelativePath, QString *icon);

    /**
     * Set the parent tagid for the tag. This is equivalent to reparenting
     * the tag
     * @param tagID          the id of the tag
     * @param newParentTagID the new parentid for the tag
     */
    void setTagParentID(int tagID, int newParentTagID);

    // ----------- Operations on SAlbums -----------

    /**
     * Add a new search to the database with the given attributes
     * @param name       name of the search
     * @param url        url of the search
     * @return the id of the album added or -1 if it failed
     */
    int addSearch(const QString& name, const KUrl& url);

    /**
     * Updates Search with new attributes
     * @param searchID   the id of the search
     * @param name       name of the search
     * @param url        url of the search
     */
    void updateSearch(int searchID, const QString& name, const KUrl& url);

    /**
     * Delete a search from the database.
     * @param searchID the id of the search
     */
    void deleteSearch(int searchID);

    // ----------- Adding and deleting Items -----------
    /**
     * Put a new item in the database or replace an existing one.
     * @return the id of item added or -1 if it fails
     */
    qlonglong addItem(int albumID, const QString& name,
                      DatabaseItem::Status status,
                      DatabaseItem::Category category,
                      const QDateTime& modificationDate,
                      int fileSize,
                      const QString& uniqueHash);

    /**
     * Deletes an item from the database.
     * @param albumID The id of the album.
     * @param file The filename of the file to delete.
     */
    void deleteItem(int albumID, const QString& file);

    /**
     * Marks all items in the specified album as removed,
     * resets their dirids.
     * The album can be deleted afterwards without removing
     * the entries for the items, which
     * can later be removed by deleteRemovedItems().
     * @param albumID The id of the album
     */
    void removeItemsFromAlbum(int albumID);

    /**
     * Marks all items in the list as removed,
     * resets their dirids.
     * The items can later be removed by deleteRemovedItems().
     * @param itemIDs a list of item IDs to be marked
     */
    void removeItems(QList<qlonglong> itemIDs);

    /**
     * Delete all items from the database that are marked as removed.
     * Use with care!
     */
    void deleteRemovedItems();

    /**
     * Delete from the database all items
     * in the specified albums that are marked as removed.
     */
    void deleteRemovedItems(QList<int> albumIDs);

    // ----------- Finding items -----------

    /**
     * Get the imageId of the item
     * @param albumId the albumID of the item
     * @param name the name of the item
     * @return the ImageId for the item, or -1 if it does not exist
     */
    qlonglong getImageId(int albumID, const QString& name);

    enum ItemSortOrder
    {
        NoItemSorting,
        ByItemName,
        ByItemPath,
        ByItemDate,
        ByItemRating
    };

    /**
     * Returns all items for a given albumid. This is used to
     * verify if all items on disk are consistent with the database
     * in the scanlib class.
     * @param albumID The albumID for which you want all items.
     * @return It returns a QStringList with the filenames.
     */
    QStringList getItemNamesInAlbum(int albumID);

    /**
     * Returns an ItemScanInfo object for each item in the album
     * with the specified album id.
     */
    QList<ItemScanInfo> getItemScanInfos(int albumID);

    /**
     * Given a albumID, get a list of the url of all items in the album
     * NOTE: Uses the CollectionManager
     * @param  albumID the id of the album
     * @return a list of urls for the items in the album. The urls are the
     * absolute path of the items
     */
    QStringList getItemURLsInAlbum(int albumID, ItemSortOrder order = NoItemSorting);

     /**
      * Given a albumID, get a list of Ids of all items in the album
      * @param  albumID the id of the album
      * @return a list of Ids for the items in the album.
      */
    QList<qlonglong> getItemIDsInAlbum(int albumID);

    /**
     * Given a tagid, get a list of the url of all items in the tag
     * NOTE: Uses the CollectionManager
     * @param  tagID the id of the tag
     * @return a list of urls for the items in the tag. The urls are the
     * absolute path of the items
     */
    QStringList getItemURLsInTag(int tagID, bool recursive = false);

    /**
     * Given a tagID, get a list of Ids of all items in the tag
     * @param  tagID the id of the tag
     * @return a list of Ids for the items in the tag.
     */
    QList<qlonglong> getItemIDsInTag(int tagID, bool recursive = false);

    /**
     * Returns all creation dates found in the image metadata table
     */
    QList<QDateTime> getAllCreationDates();

    // ----------- Item properties -----------

    /**
     * Find the album of an item
     * @param imageID The ID of the item
     * @return The ID of the PAlbum of the item, or -1 if not found
    */
    int getItemAlbum(qlonglong imageID);

    /**
     * Retrieve the name of the item
     * @param imageID The ID of the item
     * @return The name of the item, or a null string if not found
     */
    QString getItemName(qlonglong imageID);

    /**
     * Get item and album info from the image ID
     */
    ItemShortInfo getItemShortInfo(qlonglong imageID);

    /**
     * Get scan info from the image ID
     */
    ItemScanInfo getItemScanInfo(qlonglong imageID);

    /**
     * Update the fields of the Images table that have changed when
     * the file has been modified on disk.
     * @param imageID the image that has been modified
     */
    void updateItem(qlonglong imageID,
                    DatabaseItem::Category category,
                    const QDateTime& modificationDate,
                    int fileSize,
                    const QString& uniqueHash);

    /**
     * Returns the requested fields from the Images table.
     * Choose the fields with the mask.
     * The fields will be returned in the following order and type:
     * 0) Int       Album
     * 1) String    Name
     * 2) Int       Status
     * 3) Int       Category
     * 4) DateTime  ModificationDate
     * 5) int       FileSize
     * 6) String    uniqueHash
     */
    QVariantList getImagesFields(qlonglong imageID, DatabaseFields::Images imagesFields);

    /**
     * Add (or replace) the ImageInformation of the specified item.
     * If there is already an entry, it will be discarded.
     * The QVariantList shall have 9 entries, of types in this order:
     * 0) Int       rating
     * 1) DateTime* creationDate
     * 2) DateTime* digitizationDate
     * 3) Int       orientation
     * 4) Int       width
     * 5) Int       height
     * 6) String    format
     * 7) Int       colorDepth
     * 8) Int       colorModel
     * ( (*) You can provide the date also as a string in the format Qt::IsoDate)
     * You can leave out entries from this list, which will then be filled with null values.
     * Indicate the values that you have passed in the ImageInformation flag in the third parameters.
     */
    void addImageInformation(qlonglong imageID, const QVariantList &infos,
                             DatabaseFields::ImageInformation fields = DatabaseFields::ImageInformationAll);

    /**
     * Change the indicated fields of the image information for the specified item.
     * Fields not indicated by the fields parameter will not be touched.
     * This method does nothing if the item does not yet have an entry in the ImageInformation table.
     * The parameters are as for the method above.
     */
    void changeImageInformation(qlonglong imageID, const QVariantList &infos,
                                DatabaseFields::ImageInformation fields = DatabaseFields::ImageInformationAll);

    /**
     * Read image information. Parameters as above.
     */
    QVariantList getImageInformation(qlonglong imageID,
                                     DatabaseFields::ImageInformation infoFields = DatabaseFields::ImageInformationAll);

    /**
     * Add (or replace) the ImageMetadata of the specified item.
     * If there is already an entry, it will be discarded.
     * The QVariantList shall at most 15 entries, of types as defined
     * in the DBSCHEMA and in metadatainfo.h, in this order:
     *  0) String    make
     *  1) String    model
     *  2) Double    aperture
     *  3) Double    focalLength
     *  4) Double    focalLength35
     *  5) Double    exposureTime
     *  6) Int       exposureProgram
     *  7) Int       exposureMode
     *  8) Int       sensitivity
     *  9) Int       flash
     * 10) Int       WhiteBalance
     * 11) Int       WhiteBalanceColorTemperature
     * 12) Int       meteringMode
     * 13) Double    subjectDistance
     * 14) Double    subjectDistanceCategory
     * You can leave out entries from this list. Indicate the values that you have
     * passed in the ImageMetadata flag in the third parameters.
     */
    void addImageMetadata(qlonglong imageID, const QVariantList &infos,
                           DatabaseFields::ImageMetadata fields = DatabaseFields::ImageMetadataAll);

    /**
     * Change the indicated fields of the image information for the specified item.
     * This method does nothing if the item does not yet have an entry in the ImageInformation table.
     * The parameters are as for the method above.
     */
    void changeImageMetadata(qlonglong imageID, const QVariantList &infos,
                             DatabaseFields::ImageMetadata fields = DatabaseFields::ImageMetadataAll);

    /**
     * Read image metadata. Parameters as above.
     */
    QVariantList getImageMetadata(qlonglong imageID,
                                  DatabaseFields::ImageMetadata metadataFields = DatabaseFields::ImageMetadataAll);

    /**
     * Add (or replace) the ImagePosition of the specified item.
     * If there is already an entry, it will be discarded.
     * The QVariantList shall have at most 9 entries, of types in this order:
     * 0) String    Latitude
     * 1) Double    LatitudeNumber
     * 2) String    Longitude
     * 3) Double    LongitudeNumber
     * 4) Double    Altitude
     * 5) Double    Orientation
     * 6) Double    Tilt
     * 7) Double    Roll
     * 8) String    Description
     * You can leave out entries from this list. Indicate the values that you have
     * passed in the ImageInfo flag in the third parameters.
     */
    void addImagePosition(qlonglong imageID, const QVariantList &infos,
                           DatabaseFields::ImagePositions fields = DatabaseFields::ImagePositionsAll);

    /**
     * Change the indicated fields of the image information for the specified item.
     * This method does nothing if the item does not yet have an entry in the ImageInformation table.
     * The parameters are as for the method above.
     */
    void changeImagePosition(qlonglong imageID, const QVariantList &infos,
                             DatabaseFields::ImagePositions fields = DatabaseFields::ImagePositionsAll);

    /**
     * Read image metadata. Parameters as above.
     */
    QVariantList getImagePosition(qlonglong imageID,
                                  DatabaseFields::ImagePositions positionFields = DatabaseFields::ImagePositionsAll);

    /**
     * Retrieves all available comments for the specified item.
     */
    QList<CommentInfo> getImageComments(qlonglong imageID);

    /**
     * Sets the comments for the image. A comment for the image with the same
     * source, language and author will be overwritten.
     * @param imageID  The imageID of the image
     * @param comment  The comment string
     * @param source   The source of the comment
     * @param language Information about the language of the comment. A null string shall be used
     *                 if language information is not available from the source, or if
     *                 the comment is in the default language.
     * @param author   Optional information about the author who wrote the comment.
     *                 If not supported by the source, pass a null string.
     * @param date     Optional information about the date when the comment was written
     *                 If not supported by the source, pass a null string.
     * @returns the comment ID of the comment
     */
    int setImageComment(qlonglong imageID, const QString &comment, DatabaseComment::Type type,
                        const QString &language = QString(), const QString &author = QString(),
                        const QDateTime &date = QDateTime());

    /**
     * Changes the properties of a comment.
     * The QVariantList shall have at most 5 entries, of types in this order:
     * 0) Int       Type
     * 1) String    Language
     * 2) String    Author
     * 3) DateTime  Date
     * 4) String    Comment
     */
    void changeImageComment(int commentId, const QVariantList &infos,
                            DatabaseFields::ImageComments fields = DatabaseFields::ImageCommentsAll);


    /**
     * Get the datetime for the item
     * @param imageID the ID of the item
     * @return the datetime for the item
     */
    //QDateTime getItemDate(qlonglong imageID);

    /**
     * Get the item rating
     * @param imageID the ID of the item
     * @return the rating for the item
     */
    //int getItemRating(qlonglong imageID);

    /**
     * Update the date of a item to supplied date
     * @param imageID The ID of the item
     * @param datetime The datetime to be stored. Should try to let that be
     * the exif-datetime, but if not available the modification date.
     * @return It will always return true. Maybe that will change.
     */
    //bool setItemDate(qlonglong imageID, const QDateTime& datetime);

    /**
     * Update the rating of a item to supplied value
     * @param imageID The ID of the item
     * @param rating The rating value to be stored.
     */
    //void setItemRating(qlonglong imageID, int rating);


    /**
     * Get the datetime for the item
     * @param albumID the albumID of the item
     * @param name    the name of the item
     * @return the datetime for the item
     */
    //QDateTime getItemDate(int albumID, const QString& name);


    /**
     * Update the date of a item to supplied date
     * @param albumID The albumID where the file is located.
     * @param name The filename
     * @param datetime The datetime to be stored. Should try to let that be
     * the exif-datetime, but if not available the modification date.
     * @return It will always return true. Maybe that will change.
     */
    //bool setItemDate(int albumID, const QString& name,
      //               const QDateTime& datetime);

    /**
     * Get the caption for the item
     * @param imageID the id  of the item
     * @return the caption for the item
     */
    //QString getItemCaption(qlonglong imageID);

    /**
     * Get the caption for the item
     * @param albumID the albumID of the item
     * @param name    the name of the item
     * @return the caption for the item
     */
    //QString getItemCaption(int albumID, const QString& name);

    /**
     * Set the caption for the item
     * @param imageID the id of the item
     * @param caption the caption for the item
     */
    //void setItemCaption(qlonglong imageID, const QString& caption);

    /**
     * Set the caption for the item
     * @param albumID the albumID of the item
     * @param name    the name of the item
     * @param caption the caption for the item
     */
    //void setItemCaption(int albumID, const QString& name, const QString& caption);


    // ----------- Items and their tags -----------

    /**
     * Add a tag for the item
     * @param imageID the ID of the item
     * @param tagID   the tagID for the tag
     */
    void addItemTag(qlonglong imageID, int tagID);

    /**
     * Add a tag for the item
     * @param albumID the albumID of the item
     * @param name    the name of the item
     * @param tagID   the tagID for the tag
     */
    void addItemTag(int albumID, const QString& name, int tagID);

    /**
     * Add each tag of a list of tags
     * to each member of a list of items.
     */
    void addTagsToItems(QList<qlonglong> imageIDs, QList<int> tagIDs);

    /**
     * Remove a specific tag for the item
     * @param imageID the ID of the item
     * @param tagID   the tagID for the tag
     */
    void removeItemTag(qlonglong imageID, int tagID);

    /**
     * Remove all tags for the item
     * @param imageID the ID of the item
     */
    void removeItemAllTags(qlonglong imageID);

    /**
     * Remove each tag from a list of tags
     * from a each member of a list of items.
     */
    void removeTagsFromItems(QList<qlonglong> imageIDs, QList<int> tagIDs);

    /**
     * Get a list of names of all the tags for the item
     * @param imageID the ID of the item
     * @return the list of names of all tags for the item
     */
    QStringList getItemTagNames(qlonglong imageID);

    /**
     * Get a list of IDs of all the tags for the item
     * @param imageID the ID of the item
     * @return the list of IDs of all tags for the item
     */
    QList<int> getItemTagIDs(qlonglong imageID);

    /**
     * Given a set of items (identified by their IDs),
     * this will see if any of the items has a tag.
     * @param imageIDList a list of IDs of the items
     * @return true if at least one of the items has a tag
     */
    bool hasTags(const QList<qlonglong>& imageIDList);

    /**
     * Given a set of items (identified by their IDs),
     * get a list of ID of all common tags
     * @param imageIDList a list of IDs of the items
     * @return the list of common IDs of the given items
     */
    QList<int> getItemCommonTagIDs(const QList<qlonglong>& imageIDList);

    // ----------- Moving and Copying Items -----------

    /**
     * Move the attributes of an item to a different item. Useful when
     * say a file is renamed
     * @param  srcAlbumID the id of the source album
     * @param  dstAlbumID the id of the destination album
     * @param  srcName    the name of the source file
     * @param  dstName    the name of the destination file
     */
    void moveItem(int srcAlbumID, const QString& srcName,
                  int dstAlbumID, const QString& dstName);

    /**
     * Copy the attributes of an item to a different item. Useful when
     * say a file is copied.
     * The operation fails (returns -1) of src and dest are identical.
     * @param  srcAlbumID the id of the source album
     * @param  dstAlbumID the id of the destination album
     * @param  srcName    the name of the source file
     * @param  dstName    the name of the destination file
     * @return the id of item added or -1 if it fails
     */
    int copyItem(int srcAlbumID, const QString& srcName,
                 int dstAlbumID, const QString& dstName);


    // ----------- Download history methods -----------

    /**
     * Search for the specified fingerprint in the download history table.
     * Returns the id of the entry, or -1 if not found.
     */
    int findInDownloadHistory(const QString &identifier, const QString &name, int fileSize, const QDateTime &date);

    /**
     * Add the specified fingerprint to the download history table.
     * Returns the id of the entry.
     */
    int addToDownloadHistory(const QString &identifier, const QString &name, int fileSize, const QDateTime &date);

    // ----------- Static helper methods for constructing SQL queries -----------

    static QStringList imagesFieldList(DatabaseFields::Images fields);
    static QStringList imageInformationFieldList(DatabaseFields::ImageInformation fields);
    static QStringList imageMetadataFieldList(DatabaseFields::ImageMetadata fields);
    static QStringList imagePositionsFieldList(DatabaseFields::ImagePositions fields);
    static QStringList imageCommentsFieldList(DatabaseFields::ImageComments fields);
    static void addBoundValuePlaceholders(QString &query, int count);

private:

    friend class Digikam::DatabaseAccess;

    /**
     * Constructor
     */
    AlbumDB(DatabaseBackend *backend);

    /**
     * Destructor
     */
    ~AlbumDB();

private:

    AlbumDBPriv* d;
};

}  // namespace Digikam

#endif /* ALBUMDB_H */
